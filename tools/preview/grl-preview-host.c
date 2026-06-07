/* grl-preview-host.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Live-reload preview host for graylib sketches.
 *
 * Usage:
 *   grl-preview-host <sketch.c>
 *   grl-preview-host <sketch.c> --export out.gif [--frames N] [--fps F]
 *
 * Linux-only: uses inotify for file watching and dlopen/dlsym for
 * hot-reload. macOS and Windows are future work.
 *
 * Sketch ABI (see sketch-template.c for the full contract):
 *
 *   REQUIRED:
 *     void sketch_draw (GrlImage *canvas, double t);
 *
 *   OPTIONAL:
 *     void sketch_init (void);
 *     void *sketch_state (void);
 *     void sketch_reload (void *state);
 *
 * In-process crash warning:
 *   The sketch .so runs inside this process. A fatal signal (SIGSEGV, etc.)
 *   from sketch code will crash the host. Process isolation is future work.
 *
 * Building the host:
 *   Run from the graylib root:  make BUILD_PREVIEW=1 preview-tools
 *   or:  cd tools/preview && make
 *   The host must be able to find libgraylib.so at runtime:
 *     LD_LIBRARY_PATH=../../build/lib:$LD_LIBRARY_PATH ./grl-preview-host sketch.c
 */

/* _GNU_SOURCE for non-blocking inotify, CLOCK_MONOTONIC, etc. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* GRAYLIB_INSIDE / GRAYLIB_COMPILATION are provided by the build (-D flags) so
 * the umbrella header's direct-include guard is satisfied. */
#include <graylib.h>

/* ============================================================================
 * Constants / tunables
 * ============================================================================ */

#define CANVAS_WIDTH        800
#define CANVAS_HEIGHT       600
#define DEFAULT_FPS         60
#define DEFAULT_FRAMES      120
#define DEBOUNCE_MS         150   /* ms quiet period before rebuild triggers */
#define INOTIFY_EVENTS      (IN_CLOSE_WRITE | IN_MOVED_TO)

/* ============================================================================
 * Sketch function-pointer types
 * ============================================================================ */

typedef void  (*SketchDrawFn)   (GrlImage *canvas, double t);
typedef void  (*SketchInitFn)   (void);
typedef void *(*SketchStateFn)  (void);
typedef void  (*SketchReloadFn) (void *state);

/* ============================================================================
 * Loaded sketch handle
 * ============================================================================ */

typedef struct
{
    void           *dl_handle;   /* dlopen handle              */
    SketchDrawFn    draw;        /* required                   */
    SketchInitFn    init;        /* optional                   */
    SketchStateFn   state;       /* optional                   */
    SketchReloadFn  reload;      /* optional                   */
} SketchHandle;

static void
sketch_handle_close (SketchHandle *h)
{
    if (h == NULL || h->dl_handle == NULL)
        return;
    dlclose (h->dl_handle);
    h->dl_handle = NULL;
    h->draw      = NULL;
    h->init      = NULL;
    h->state     = NULL;
    h->reload    = NULL;
}

/* ============================================================================
 * Time helpers
 * ============================================================================ */

/*
 * Returns monotonic time in milliseconds.
 */
static long long
now_ms (void)
{
    struct timespec ts;
    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + (long long)(ts.tv_nsec / 1000000LL);
}

/*
 * Returns elapsed seconds since @start_ms.
 */
static double
elapsed_sec (long long start_ms)
{
    return (double)(now_ms () - start_ms) / 1000.0;
}

/* ============================================================================
 * Compiler invocation
 *
 * The sketch source is compiled into a temp .so using the same include paths
 * and link flags that the examples/Makefile uses.  The host must be invoked
 * from a location where the build/ directory is at ../../build/ relative to
 * this source file, or the user must override with GRAYLIB_ROOT env var.
 *
 * Flags mirror examples/Makefile:
 *
 *   CFLAGS: $(GLIB_CFLAGS) -I$(PROJROOT) -I$(PROJROOT)/src -DGRAYLIB_COMPILATION
 *   LDFLAGS: -L$(LIBOUTDIR) -lgraylib $(GLIB_LIBS) $(RAYLIB_LIBDIR)/libraylib.a
 *            $(PLATFORM_LIBS)
 *
 * We resolve the graylib root from the executable's own path at runtime
 * (by reading /proc/self/exe), then fall back to the GRAYLIB_ROOT env var,
 * then finally fall back to a compile-time default.
 * ============================================================================ */

/*
 * Resolve the absolute path of the graylib repo root.
 * Strategy: walk up from /proc/self/exe looking for "config.mk".
 * Falls back to GRAYLIB_ROOT env var, then to GRAYLIB_ROOT_DEFAULT.
 */
#ifndef GRAYLIB_ROOT_DEFAULT
#define GRAYLIB_ROOT_DEFAULT "."
#endif

static void
resolve_graylib_root (char *out, size_t out_size)
{
    char   exe_path[PATH_MAX];
    char  *env_root;
    ssize_t n;
    char  *slash;
    char   probe[PATH_MAX];
    int    found;
    int    depth;

    /* Try GRAYLIB_ROOT env var first */
    env_root = getenv ("GRAYLIB_ROOT");
    if (env_root != NULL && env_root[0] != '\0')
    {
        snprintf (out, out_size, "%s", env_root);
        return;
    }

    /* Try /proc/self/exe walk */
    n = readlink ("/proc/self/exe", exe_path, sizeof (exe_path) - 1);
    if (n > 0)
    {
        exe_path[n] = '\0';

        /* Walk up at most 8 levels looking for config.mk */
        found = 0;
        for (depth = 0; depth < 8 && !found; depth++)
        {
            slash = strrchr (exe_path, '/');
            if (slash == NULL)
                break;
            *slash = '\0';

            snprintf (probe, sizeof (probe), "%s/config.mk", exe_path);
            if (access (probe, F_OK) == 0)
            {
                snprintf (out, out_size, "%s", exe_path);
                found = 1;
            }
        }
        if (found)
            return;
    }

    /* Final fallback */
    snprintf (out, out_size, "%s", GRAYLIB_ROOT_DEFAULT);
}

/*
 * Build a sketch source file into a shared object.
 *
 * @sketch_src   - absolute path to sketch .c
 * @out_so       - path to write the output .so
 * @graylib_root - path to graylib repo root
 *
 * Returns 0 on success, non-zero on failure.
 * Compiler stderr is printed to stderr.
 */
static int
compile_sketch (const char *sketch_src,
                const char *out_so,
                const char *graylib_root)
{
    char  cmd[8192];
    char  glib_cflags[2048];
    char  glib_libs[2048];
    FILE *fp;
    int   rc;

    /* Ask pkg-config for glib flags at runtime */
    fp = popen ("pkg-config --cflags glib-2.0 gobject-2.0 gio-2.0 2>/dev/null",
                "r");
    if (fp == NULL)
    {
        glib_cflags[0] = '\0';
    }
    else
    {
        if (fgets (glib_cflags, sizeof (glib_cflags), fp) == NULL)
            glib_cflags[0] = '\0';
        pclose (fp);
        /* Strip trailing newline */
        {
            size_t len = strlen (glib_cflags);
            if (len > 0 && glib_cflags[len - 1] == '\n')
                glib_cflags[len - 1] = '\0';
        }
    }

    fp = popen ("pkg-config --libs glib-2.0 gobject-2.0 gio-2.0 2>/dev/null",
                "r");
    if (fp == NULL)
    {
        glib_libs[0] = '\0';
    }
    else
    {
        if (fgets (glib_libs, sizeof (glib_libs), fp) == NULL)
            glib_libs[0] = '\0';
        pclose (fp);
        {
            size_t len = strlen (glib_libs);
            if (len > 0 && glib_libs[len - 1] == '\n')
                glib_libs[len - 1] = '\0';
        }
    }

    /*
     * Compose compiler command.
     *
     * Include paths:
     *   -I<root>           (for the graylib.pc / top-level include)
     *   -I<root>/src       (for src/graylib.h and sub-headers)
     *
     * Link flags mirror examples/Makefile:
     *   -L<root>/build/lib -lgraylib
     *   <root>/deps/raylib/src/libraylib.a
     *   $(PLATFORM_LIBS): -lGL -lm -lpthread -ldl -lrt -lX11
     *
     * -Wl,-rpath is set so the sketch .so can find libgraylib.so when
     * dlopen'd, without requiring LD_LIBRARY_PATH.  The host itself still
     * needs LD_LIBRARY_PATH (or an installed graylib) to run at all.
     */
    snprintf (cmd, sizeof (cmd),
              "cc -shared -fPIC -O2"
              " -DGRAYLIB_COMPILATION"           /* graylib.h defines GRAYLIB_INSIDE itself */
              " %s"                              /* glib cflags */
              " -I%s -I%s/src"                  /* graylib includes */
              " '%s'"                            /* sketch source */
              " -o '%s'"                         /* output .so */
              " -L%s/build/lib -lgraylib"        /* graylib shared lib */
              " %s"                              /* glib libs */
              " %s/deps/raylib/src/libraylib.a"  /* embedded raylib */
              " -lGL -lm -lpthread -ldl -lrt -lX11" /* platform libs */
              " -Wl,-rpath,%s/build/lib"         /* runtime rpath */
              " 2>&1",                           /* merge stderr->stdout */
              glib_cflags,
              graylib_root, graylib_root,
              sketch_src,
              out_so,
              graylib_root,
              glib_libs,
              graylib_root,
              graylib_root);

    fprintf (stderr, "[preview] Compiling: %s\n", sketch_src);

    fp = popen (cmd, "r");
    if (fp == NULL)
    {
        fprintf (stderr, "[preview] Failed to invoke compiler\n");
        return -1;
    }

    /* Print compiler output */
    {
        char line[512];
        while (fgets (line, sizeof (line), fp) != NULL)
            fputs (line, stderr);
    }

    rc = pclose (fp);

    if (WIFEXITED (rc) && WEXITSTATUS (rc) == 0)
    {
        fprintf (stderr, "[preview] Compile OK -> %s\n", out_so);
        return 0;
    }

    fprintf (stderr, "[preview] Compile FAILED (exit %d)\n",
             WIFEXITED (rc) ? WEXITSTATUS (rc) : -1);
    return -1;
}

/* ============================================================================
 * dlopen / validate a compiled sketch .so
 *
 * Returns a heap-allocated SketchHandle on success (caller must free + dlclose),
 * or NULL on failure (logs reason to stderr).
 * ============================================================================ */

static SketchHandle *
load_sketch_so (const char *so_path)
{
    SketchHandle *h;
    void         *dl;
    SketchDrawFn  draw_fn;

    dl = dlopen (so_path, RTLD_NOW | RTLD_LOCAL);
    if (dl == NULL)
    {
        fprintf (stderr, "[preview] dlopen failed: %s\n", dlerror ());
        return NULL;
    }

    /* sketch_draw is required */
    draw_fn = (SketchDrawFn)(uintptr_t)dlsym (dl, "sketch_draw");
    if (draw_fn == NULL)
    {
        fprintf (stderr, "[preview] sketch_draw not found in .so: %s\n",
                 dlerror ());
        dlclose (dl);
        return NULL;
    }

    h = (SketchHandle *)calloc (1, sizeof (SketchHandle));
    if (h == NULL)
    {
        dlclose (dl);
        return NULL;
    }

    h->dl_handle = dl;
    h->draw      = draw_fn;
    h->init      = (SketchInitFn)(uintptr_t)dlsym (dl, "sketch_init");
    h->state     = (SketchStateFn)(uintptr_t)dlsym (dl, "sketch_state");
    h->reload    = (SketchReloadFn)(uintptr_t)dlsym (dl, "sketch_reload");

    return h;
}

/* ============================================================================
 * Hot-reload: snapshot + swap
 * ============================================================================ */

/*
 * Attempt to compile @sketch_src and atomically swap into @current_out.
 * On success, *current_out is replaced (old handle is dlclosed after state
 * snapshot).  On failure, *current_out is left unchanged.
 *
 * @sketch_src    - absolute path to sketch source
 * @tmp_so        - path for the freshly-compiled .so
 * @graylib_root  - repo root
 * @current_out   - [in/out] pointer to the currently-loaded handle (may be NULL)
 */
static void
try_reload (const char   *sketch_src,
            const char   *tmp_so,
            const char   *graylib_root,
            SketchHandle **current_out)
{
    SketchHandle *new_h;
    SketchHandle *old_h;
    void         *old_state;

    if (compile_sketch (sketch_src, tmp_so, graylib_root) != 0)
    {
        fprintf (stderr,
                 "[preview] Keeping last-good sketch (compile failed)\n");
        return;
    }

    new_h = load_sketch_so (tmp_so);
    if (new_h == NULL)
    {
        fprintf (stderr,
                 "[preview] Keeping last-good sketch (load failed)\n");
        return;
    }

    /* Snapshot state from old handle before dlclose */
    old_h     = *current_out;
    old_state = NULL;

    if (old_h != NULL && old_h->state != NULL)
        old_state = old_h->state ();

    /* dlclose the old handle only after we have the new one */
    if (old_h != NULL)
    {
        sketch_handle_close (old_h);
        free (old_h);
    }

    *current_out = new_h;

    /* Initialise the new handle */
    if (old_state != NULL && new_h->reload != NULL)
    {
        new_h->reload (old_state);
    }
    else
    {
        if (new_h->init != NULL)
            new_h->init ();
    }

    fprintf (stderr, "[preview] Hot-reload complete\n");
}

/* ============================================================================
 * inotify watch setup
 * ============================================================================ */

/*
 * Set up an inotify watch on the directory containing @sketch_path.
 * Returns the inotify fd (non-blocking) or -1 on error.
 * @watch_desc_out receives the watch descriptor.
 */
static int
setup_inotify (const char *sketch_path,
               int        *watch_desc_out)
{
    char  dir_buf[PATH_MAX];
    char *dir;
    int   ifd;
    int   wd;

    strncpy (dir_buf, sketch_path, sizeof (dir_buf) - 1);
    dir_buf[sizeof (dir_buf) - 1] = '\0';
    dir = dirname (dir_buf);

    ifd = inotify_init1 (IN_NONBLOCK);
    if (ifd < 0)
    {
        perror ("[preview] inotify_init1");
        return -1;
    }

    wd = inotify_add_watch (ifd, dir, INOTIFY_EVENTS);
    if (wd < 0)
    {
        perror ("[preview] inotify_add_watch");
        close (ifd);
        return -1;
    }

    *watch_desc_out = wd;
    return ifd;
}

/*
 * Drain the inotify fd and return 1 if an event matching @sketch_basename
 * was seen, 0 otherwise.  Non-blocking (poll with 0 timeout).
 */
static int
inotify_poll (int        ifd,
              int        wd,
              const char *sketch_basename)
{
    struct pollfd pfd;
    int           ready;
    char          buf[4096];
    ssize_t       n;
    int           matched;
    size_t        offset;

    pfd.fd     = ifd;
    pfd.events = POLLIN;
    ready = poll (&pfd, 1, 0); /* non-blocking */
    if (ready <= 0)
        return 0;

    n = read (ifd, buf, sizeof (buf));
    if (n <= 0)
        return 0;

    matched = 0;
    offset  = 0;

    while (offset < (size_t)n)
    {
        struct inotify_event *ev;

        ev = (struct inotify_event *)(buf + offset);
        offset += sizeof (struct inotify_event) + ev->len;

        if (ev->wd != wd)
            continue;
        if (!(ev->mask & INOTIFY_EVENTS))
            continue;
        if (ev->len == 0)
        {
            /* No name attached — match unconditionally */
            matched = 1;
            continue;
        }
        if (strcmp (ev->name, sketch_basename) == 0)
            matched = 1;
    }

    return matched;
}

/* ============================================================================
 * Placeholder drawn when no valid sketch is loaded
 * ============================================================================ */

static void
draw_placeholder (GrlImage *canvas,
                  double    t)
{
    int w;
    int h;
    int phase;

    w = grl_image_get_width  (canvas);
    h = grl_image_get_height (canvas);
    phase = (int)(t * 2.0) % 2;

    {
        g_autoptr (GrlColor) bg = (phase == 0)
            ? grl_color_new (40, 10, 10, 255)
            : grl_color_new (30, 10, 10, 255);
        grl_image_clear_background (canvas, bg);
    }

    {
        g_autoptr (GrlColor) fg = grl_color_new (200, 100, 100, 255);
        grl_image_draw_text_bitmap (canvas,
                                    "No valid sketch loaded.",
                                    8, h / 2 - 16, 10, fg);
        grl_image_draw_text_bitmap (canvas,
                                    "Fix compile errors and save.",
                                    8, h / 2,      10, fg);
    }

    /* Suppress unused-parameter warning for w */
    (void)w;
}

/* ============================================================================
 * Headless export mode
 * ============================================================================ */

static int
run_export (SketchHandle *sketch,
            GrlImage     *canvas,
            const char   *out_path,
            int           frame_count,
            int           fps)
{
    GrlGifWriter *gif;
    GError       *error;
    int           delay_cs;
    int           i;
    double        t;

    delay_cs = (fps > 0) ? (100 / fps) : 3; /* centiseconds per frame */
    if (delay_cs < 1) delay_cs = 1;

    error = NULL;
    gif = grl_gif_writer_new (out_path,
                              grl_image_get_width  (canvas),
                              grl_image_get_height (canvas),
                              0,      /* loop forever */
                              &error);
    if (gif == NULL)
    {
        fprintf (stderr, "[preview] Failed to open GIF for writing: %s\n",
                 error ? error->message : "(unknown)");
        if (error) g_error_free (error);
        return 1;
    }

    fprintf (stderr, "[preview] Exporting %d frames at %d fps -> %s\n",
             frame_count, fps, out_path);

    for (i = 0; i < frame_count; i++)
    {
        t = (double)i / (double)fps;
        sketch->draw (canvas, t);

        if (!grl_gif_writer_add_frame (gif, canvas, delay_cs, &error))
        {
            fprintf (stderr, "[preview] GIF write error at frame %d: %s\n",
                     i, error ? error->message : "(unknown)");
            if (error) g_error_free (error);
            g_object_unref (gif);
            return 1;
        }
    }

    if (!grl_gif_writer_close (gif, &error))
    {
        fprintf (stderr, "[preview] GIF close error: %s\n",
                 error ? error->message : "(unknown)");
        if (error) g_error_free (error);
        g_object_unref (gif);
        return 1;
    }

    g_object_unref (gif);
    fprintf (stderr, "[preview] Export complete: %s\n", out_path);
    return 0;
}

/* ============================================================================
 * Windowed (live-reload) main loop
 * ============================================================================ */

static int
run_windowed (SketchHandle **sketch_out,
              GrlImage      *canvas,
              const char    *sketch_src_abs,
              const char    *tmp_so,
              const char    *graylib_root)
{
    GrlWindow   *window;
    GrlTexture  *texture;
    int          ifd;
    int          watch_desc;
    char         basename_buf[PATH_MAX];
    const char  *sketch_basename;
    long long    last_event_ms;
    long long    start_ms;

    /* Open window */
    window = grl_window_new (CANVAS_WIDTH, CANVAS_HEIGHT,
                             "grl-preview-host — live reload");
    if (!grl_window_is_ready (window))
    {
        fprintf (stderr, "[preview] Failed to open window\n");
        g_object_unref (window);
        return 1;
    }
    grl_window_set_target_fps (window, DEFAULT_FPS);

    /* Create GPU texture from the canvas (same dimensions) */
    texture = grl_texture_new_from_image (canvas);

    /* inotify setup */
    strncpy (basename_buf, sketch_src_abs, sizeof (basename_buf) - 1);
    basename_buf[sizeof (basename_buf) - 1] = '\0';
    sketch_basename = basename (basename_buf);

    ifd        = setup_inotify (sketch_src_abs, &watch_desc);
    last_event_ms = 0;
    start_ms   = now_ms ();

    while (!grl_window_should_close (window))
    {
        long long    now;
        double       t;

        now = now_ms ();
        t   = elapsed_sec (start_ms);

        /* -------------------------------------------------------------------
         * Poll inotify (non-blocking) for sketch file changes
         * ------------------------------------------------------------------- */
        if (ifd >= 0 &&
            inotify_poll (ifd, watch_desc, sketch_basename))
        {
            last_event_ms = now;
        }

        /* Debounce: rebuild only after a quiet period */
        if (last_event_ms > 0 &&
            (now - last_event_ms) >= DEBOUNCE_MS)
        {
            last_event_ms = 0; /* reset */
            try_reload (sketch_src_abs, tmp_so, graylib_root, sketch_out);
        }

        /* -------------------------------------------------------------------
         * Render frame into CPU canvas
         * ------------------------------------------------------------------- */
        if (*sketch_out != NULL)
            (*sketch_out)->draw (canvas, t);
        else
            draw_placeholder (canvas, t);

        /* -------------------------------------------------------------------
         * Upload canvas to GPU texture and present
         * ------------------------------------------------------------------- */
        grl_texture_update (texture, canvas);

        grl_window_begin_drawing (window);
        {
            g_autoptr (GrlColor) bg = grl_color_new (0, 0, 0, 255);
            grl_window_clear_background (window, bg);
        }
        {
            g_autoptr (GrlColor) tint = grl_color_new_white ();
            grl_draw_texture (texture, 0, 0, tint);
        }
        grl_window_end_drawing (window);
    }

    /* Cleanup */
    if (ifd >= 0)
        close (ifd);

    g_object_unref (texture);
    g_object_unref (window);

    return 0;
}

/* ============================================================================
 * Argument parsing helpers
 * ============================================================================ */

static void
print_usage (const char *prog)
{
    fprintf (stderr,
             "Usage:\n"
             "  %s <sketch.c>\n"
             "  %s <sketch.c> --export out.gif [--frames N] [--fps F]\n"
             "\n"
             "Options:\n"
             "  --export <file>   Write frames to an animated GIF (headless)\n"
             "  --frames N        Number of frames to export (default: %d)\n"
             "  --fps F           Frames per second for export (default: %d)\n"
             "\n"
             "Environment:\n"
             "  GRAYLIB_ROOT      Override graylib repo root auto-detection\n"
             "  LD_LIBRARY_PATH   Must include the graylib build/lib directory\n"
             "\n"
             "Notes:\n"
             "  - Windowed mode uses inotify to watch the sketch source file.\n"
             "    The host recompiles and hot-reloads on every save.\n"
             "  - Compile failures keep the last successfully loaded sketch.\n"
             "  - This tool is Linux-only (inotify + dlopen).\n"
             "  - The sketch runs in-process; crashes in sketch code crash the host.\n",
             prog, prog, DEFAULT_FRAMES, DEFAULT_FPS);
}

/* ============================================================================
 * main
 * ============================================================================ */

int
main (int   argc,
      char *argv[])
{
    const char  *sketch_src;
    const char  *export_path;
    int          export_frames;
    int          export_fps;
    int          i;
    char         sketch_src_abs[PATH_MAX];
    char         tmp_so[PATH_MAX];
    char         graylib_root[PATH_MAX];
    GrlImage    *canvas;
    SketchHandle *sketch;
    int          ret;

    /* -----------------------------------------------------------------------
     * Parse arguments
     * ----------------------------------------------------------------------- */
    if (argc < 2)
    {
        print_usage (argv[0]);
        return 1;
    }

    sketch_src    = argv[1];
    export_path   = NULL;
    export_frames = DEFAULT_FRAMES;
    export_fps    = DEFAULT_FPS;

    for (i = 2; i < argc; i++)
    {
        if (strcmp (argv[i], "--export") == 0 && i + 1 < argc)
        {
            export_path = argv[++i];
        }
        else if (strcmp (argv[i], "--frames") == 0 && i + 1 < argc)
        {
            export_frames = atoi (argv[++i]);
            if (export_frames <= 0) export_frames = DEFAULT_FRAMES;
        }
        else if (strcmp (argv[i], "--fps") == 0 && i + 1 < argc)
        {
            export_fps = atoi (argv[++i]);
            if (export_fps <= 0) export_fps = DEFAULT_FPS;
        }
        else if (strcmp (argv[i], "--help") == 0 ||
                 strcmp (argv[i], "-h") == 0)
        {
            print_usage (argv[0]);
            return 0;
        }
        else
        {
            fprintf (stderr, "[preview] Unknown option: %s\n", argv[i]);
            print_usage (argv[0]);
            return 1;
        }
    }

    /* -----------------------------------------------------------------------
     * Resolve absolute paths
     * ----------------------------------------------------------------------- */
    if (realpath (sketch_src, sketch_src_abs) == NULL)
    {
        /*
         * realpath fails if the file doesn't exist yet or the path is
         * relative without a resolvable base.  Construct the absolute path
         * manually by prepending cwd.
         */
        char cwd[PATH_MAX];
        if (getcwd (cwd, sizeof (cwd)) != NULL)
            snprintf (sketch_src_abs, sizeof (sketch_src_abs),
                      "%s/%s", cwd, sketch_src);
        else
            snprintf (sketch_src_abs, sizeof (sketch_src_abs),
                      "%s", sketch_src);
    }

    /* Temp .so lives in /tmp to avoid cluttering the source tree */
    snprintf (tmp_so, sizeof (tmp_so),
              "/tmp/grl-preview-%d.so", (int)getpid ());

    resolve_graylib_root (graylib_root, sizeof (graylib_root));
    fprintf (stderr, "[preview] graylib root: %s\n", graylib_root);
    fprintf (stderr, "[preview] sketch:       %s\n", sketch_src_abs);

    /* -----------------------------------------------------------------------
     * Create the CPU-side canvas (RGBA8, kept alive for the full session)
     * ----------------------------------------------------------------------- */
    {
        g_autoptr (GrlColor) fill = grl_color_new (0, 0, 0, 255);
        canvas = grl_image_new_color (CANVAS_WIDTH, CANVAS_HEIGHT, fill);
    }

    if (canvas == NULL)
    {
        fprintf (stderr, "[preview] Failed to create canvas image\n");
        return 1;
    }

    /* -----------------------------------------------------------------------
     * Initial compile + load
     * ----------------------------------------------------------------------- */
    sketch = NULL;
    try_reload (sketch_src_abs, tmp_so, graylib_root, &sketch);

    if (sketch == NULL)
    {
        if (export_path != NULL)
        {
            fprintf (stderr,
                     "[preview] Initial compile failed; cannot export.\n");
            g_object_unref (canvas);
            return 1;
        }
        fprintf (stderr,
                 "[preview] Initial compile failed; starting with placeholder.\n");
    }

    /* -----------------------------------------------------------------------
     * Run
     * ----------------------------------------------------------------------- */
    if (export_path != NULL)
    {
        ret = run_export (sketch, canvas, export_path, export_frames, export_fps);
    }
    else
    {
        ret = run_windowed (&sketch, canvas, sketch_src_abs, tmp_so, graylib_root);
    }

    /* -----------------------------------------------------------------------
     * Cleanup
     * ----------------------------------------------------------------------- */
    if (sketch != NULL)
    {
        sketch_handle_close (sketch);
        free (sketch);
    }

    g_object_unref (canvas);
    unlink (tmp_so);

    return ret;
}
