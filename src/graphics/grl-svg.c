/* grl-svg.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * SVG import / export backed by GrlPath.
 *
 * Architecture
 * ============
 * The file is split into four sections:
 *
 *   1. Internal helpers — colour parsing, transform composition,
 *      SVG-path-d tokeniser and builder.
 *   2. Minimal tolerant XML scanner — scans tags and attributes
 *      without a full DOM; handles comments, PI, DOCTYPE, CDATA,
 *      self-closing tags, and unknown elements.
 *   3. SVG builder — walks the scanned tag stream maintaining a
 *      paint/transform inheritance stack and emits GrlVectorShape
 *      objects into a GPtrArray.
 *   4. Public API — load from file / memory, save, to_string,
 *      path-to-SVG-string, image-draw-shapes.
 */

#include "config.h"
#include "grl-svg.h"

#include "../math/grl-matrix.h"

#include <glib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* =========================================================================
 * Error quark
 * ========================================================================= */

G_DEFINE_QUARK (grl-svg-error-quark, grl_svg_error)

/* =========================================================================
 * GrlVectorShape GBoxed
 * ========================================================================= */

/**
 * grl_vector_shape_new:
 *
 * Allocates and zero-initialises a new #GrlVectorShape backed by an empty
 * #GrlPath.
 *
 * Returns: (transfer full): A newly allocated #GrlVectorShape.
 */
GrlVectorShape *
grl_vector_shape_new (void)
{
    GrlVectorShape *shape;

    shape = g_new0 (GrlVectorShape, 1);
    shape->path = grl_path_new ();
    shape->fill_rule = GRL_FILL_RULE_NONZERO;

    return shape;
}

/**
 * grl_vector_shape_copy:
 * @self: (nullable): A #GrlVectorShape.
 *
 * Creates a deep copy of @self.
 *
 * Returns: (transfer full) (nullable): A new independent copy.
 */
GrlVectorShape *
grl_vector_shape_copy (const GrlVectorShape *self)
{
    GrlVectorShape *copy;

    if (self == NULL)
        return NULL;

    copy = g_new (GrlVectorShape, 1);
    copy->path        = grl_path_copy (self->path);
    copy->has_fill    = self->has_fill;
    copy->fill        = self->fill;
    copy->has_stroke  = self->has_stroke;
    copy->stroke      = self->stroke;
    copy->stroke_width = self->stroke_width;
    copy->fill_rule   = self->fill_rule;

    return copy;
}

/**
 * grl_vector_shape_free:
 * @self: (nullable): A #GrlVectorShape.
 *
 * Frees @self and unrefs its embedded path.
 */
void
grl_vector_shape_free (GrlVectorShape *self)
{
    if (self == NULL)
        return;

    g_clear_object (&self->path);
    g_free (self);
}

/**
 * grl_vector_shape_get_path:
 * @self: A #GrlVectorShape.
 *
 * Returns: (transfer none): The #GrlPath owned by this shape.
 */
GrlPath *
grl_vector_shape_get_path (GrlVectorShape *self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return self->path;
}

G_DEFINE_BOXED_TYPE (GrlVectorShape, grl_vector_shape,
                     grl_vector_shape_copy, grl_vector_shape_free)


/* =========================================================================
 * Section 1: Internal helpers
 * ========================================================================= */

/* -------------------------------------------------------------------------
 * Colour parsing
 * ------------------------------------------------------------------------- */

typedef struct
{
    const gchar *name;
    guint8       r, g, b;
} NamedColor;

static const NamedColor SVG_NAMED_COLORS[] = {
    { "black",       0,   0,   0 },
    { "white",     255, 255, 255 },
    { "red",       255,   0,   0 },
    { "green",       0, 128,   0 },
    { "lime",        0, 255,   0 },
    { "blue",        0,   0, 255 },
    { "yellow",    255, 255,   0 },
    { "cyan",        0, 255, 255 },
    { "aqua",        0, 255, 255 },
    { "magenta",   255,   0, 255 },
    { "fuchsia",   255,   0, 255 },
    { "gray",      128, 128, 128 },
    { "grey",      128, 128, 128 },
    { "silver",    192, 192, 192 },
    { "navy",        0,   0, 128 },
    { "maroon",    128,   0,   0 },
    { "purple",    128,   0, 128 },
    { "teal",        0, 128, 128 },
    { "olive",     128, 128,   0 },
    { "orange",    255, 165,   0 },
    { "coral",     255, 127,  80 },
    { "salmon",    250, 128, 114 },
    { "pink",      255, 192, 203 },
    { "brown",     165,  42,  42 },
    { "transparent", 0,   0,   0 }, /* alpha handled separately */
    { NULL, 0, 0, 0 }
};

/*
 * parse_svg_color:
 * Parses a CSS/SVG colour value into *r,*g,*b,*a.
 * Returns TRUE on success; FALSE if the value is "none" or unparseable.
 * "transparent" maps to r=g=b=0, a=0, returns TRUE.
 * "*a" is NOT modified for named colours other than "transparent" — the
 * caller must initialise *a before calling.
 */
static gboolean
parse_svg_color (const gchar *s,
                 guint8      *r,
                 guint8      *g,
                 guint8      *b,
                 guint8      *a)
{
    gchar  lower[64];
    gsize  len;
    guint  i;

    if (s == NULL)
        return FALSE;

    /* Skip leading whitespace */
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
        s++;

    if (g_ascii_strcasecmp (s, "none") == 0)
        return FALSE;

    /* #rgb */
    if (s[0] == '#')
    {
        const gchar *h = s + 1;
        len = strlen (h);

        if (len == 3)
        {
            gint rv, gv, bv;
            rv = g_ascii_xdigit_value (h[0]);
            gv = g_ascii_xdigit_value (h[1]);
            bv = g_ascii_xdigit_value (h[2]);
            if (rv < 0 || gv < 0 || bv < 0)
                return FALSE;
            *r = (guint8)(rv * 17);
            *g = (guint8)(gv * 17);
            *b = (guint8)(bv * 17);
            return TRUE;
        }
        if (len >= 6)
        {
            gint hi;
            guint32 val = 0;
            gint k;
            for (k = 0; k < 6; k++)
            {
                hi = g_ascii_xdigit_value (h[k]);
                if (hi < 0)
                    return FALSE;
                val = (val << 4) | (guint32)hi;
            }
            *r = (guint8)((val >> 16) & 0xFF);
            *g = (guint8)((val >>  8) & 0xFF);
            *b = (guint8)(val & 0xFF);
            return TRUE;
        }
        return FALSE;
    }

    /* rgb(r,g,b) */
    if (g_ascii_strncasecmp (s, "rgb(", 4) == 0)
    {
        gint rv, gv, bv;
        if (sscanf (s + 4, "%d , %d , %d", &rv, &gv, &bv) == 3 ||
            sscanf (s + 4, "%d,%d,%d",     &rv, &gv, &bv) == 3)
        {
            *r = (guint8)CLAMP (rv, 0, 255);
            *g = (guint8)CLAMP (gv, 0, 255);
            *b = (guint8)CLAMP (bv, 0, 255);
            return TRUE;
        }
        return FALSE;
    }

    /* named colours */
    len = strlen (s);
    if (len >= sizeof (lower))
        len = sizeof (lower) - 1;
    for (i = 0; i < len; i++)
        lower[i] = g_ascii_tolower (s[i]);
    lower[len] = '\0';

    for (i = 0; SVG_NAMED_COLORS[i].name != NULL; i++)
    {
        if (strcmp (lower, SVG_NAMED_COLORS[i].name) == 0)
        {
            *r = SVG_NAMED_COLORS[i].r;
            *g = SVG_NAMED_COLORS[i].g;
            *b = SVG_NAMED_COLORS[i].b;
            if (strcmp (lower, "transparent") == 0)
                *a = 0;
            return TRUE;
        }
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * Transform parsing
 *
 * We need to parse the SVG transform attribute and produce a GrlMatrix.
 * For 2D SVG purposes we only use the upper-left 2x3 of the 4x4 matrix:
 *
 *   [ m0  m4  m8  m12 ]      column-major raylib layout
 *   [ m1  m5  m9  m13 ]
 *   [ m2  m6  m10 m14 ]
 *   [ m3  m7  m11 m15 ]
 *
 * A 2D affine:
 *   [ a  c  0  tx ]
 *   [ b  d  0  ty ]
 *   [ 0  0  1   0 ]
 *   [ 0  0  0   1 ]
 *
 * In column-major: m0=a, m1=b, m4=c, m5=d, m12=tx, m13=ty
 * ------------------------------------------------------------------------- */

static void
matrix_set_identity (GrlMatrix *m)
{
    m->m0 = 1; m->m4 = 0; m->m8  = 0; m->m12 = 0;
    m->m1 = 0; m->m5 = 1; m->m9  = 0; m->m13 = 0;
    m->m2 = 0; m->m6 = 0; m->m10 = 1; m->m14 = 0;
    m->m3 = 0; m->m7 = 0; m->m11 = 0; m->m15 = 1;
}

/* result = a * b  (both 4x4, column-major, left-to-right concat) */
static void
matrix_multiply_into (GrlMatrix       *result,
                      const GrlMatrix *a,
                      const GrlMatrix *b)
{
    /* result[col][row] = sum_k a[k][row] * b[col][k]
     * Column-major: col 0 = (m0,m1,m2,m3), col 1 = (m4,m5,m6,m7), etc.
     */
    result->m0  = a->m0*b->m0  + a->m4*b->m1  + a->m8*b->m2  + a->m12*b->m3;
    result->m1  = a->m1*b->m0  + a->m5*b->m1  + a->m9*b->m2  + a->m13*b->m3;
    result->m2  = a->m2*b->m0  + a->m6*b->m1  + a->m10*b->m2 + a->m14*b->m3;
    result->m3  = a->m3*b->m0  + a->m7*b->m1  + a->m11*b->m2 + a->m15*b->m3;

    result->m4  = a->m0*b->m4  + a->m4*b->m5  + a->m8*b->m6  + a->m12*b->m7;
    result->m5  = a->m1*b->m4  + a->m5*b->m5  + a->m9*b->m6  + a->m13*b->m7;
    result->m6  = a->m2*b->m4  + a->m6*b->m5  + a->m10*b->m6 + a->m14*b->m7;
    result->m7  = a->m3*b->m4  + a->m7*b->m5  + a->m11*b->m6 + a->m15*b->m7;

    result->m8  = a->m0*b->m8  + a->m4*b->m9  + a->m8*b->m10 + a->m12*b->m11;
    result->m9  = a->m1*b->m8  + a->m5*b->m9  + a->m9*b->m10 + a->m13*b->m11;
    result->m10 = a->m2*b->m8  + a->m6*b->m9  + a->m10*b->m10+ a->m14*b->m11;
    result->m11 = a->m3*b->m8  + a->m7*b->m9  + a->m11*b->m10+ a->m15*b->m11;

    result->m12 = a->m0*b->m12 + a->m4*b->m13 + a->m8*b->m14 + a->m12*b->m15;
    result->m13 = a->m1*b->m12 + a->m5*b->m13 + a->m9*b->m14 + a->m13*b->m15;
    result->m14 = a->m2*b->m12 + a->m6*b->m13 + a->m10*b->m14+ a->m14*b->m15;
    result->m15 = a->m3*b->m12 + a->m7*b->m13 + a->m11*b->m14+ a->m15*b->m15;
}

/*
 * Skip ASCII whitespace + commas (used inside transform argument lists).
 */
static const gchar *
skip_ws_comma (const gchar *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')
        p++;
    return p;
}

/*
 * parse_svg_transform:
 * Parses a CSS/SVG transform string like
 *   "translate(10,20) scale(2) rotate(45) matrix(a,b,c,d,e,f)"
 * and accumulates the result into *mat (which must already be initialised).
 * Each function is post-multiplied onto *mat: mat = mat * T.
 *
 * The rotation variant with a centre point is also handled:
 *   rotate(angle, cx, cy)  =  translate(cx,cy) rotate(angle) translate(-cx,-cy)
 */
static void
parse_svg_transform (const gchar *s,
                     GrlMatrix   *mat)
{
    const gchar *p;
    GrlMatrix    tmp;
    GrlMatrix    result;

    if (s == NULL)
        return;

    p = s;
    while (*p)
    {
        gchar     fname[32];
        gsize     fi;
        gdouble   args[6];
        gint      nargs;
        gchar    *end;

        /* skip whitespace */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        if (!*p)
            break;

        /* read function name */
        fi = 0;
        while (*p && *p != '(' && fi < sizeof (fname) - 1)
            fname[fi++] = g_ascii_tolower (*p++);
        fname[fi] = '\0';

        if (*p != '(')
            break; /* malformed — stop */
        p++; /* skip '(' */

        /* read up to 6 comma/ws-separated numbers */
        nargs = 0;
        while (nargs < 6)
        {
            p = skip_ws_comma (p);
            if (*p == ')')
                break;
            if (!*p)
                break;
            args[nargs] = g_ascii_strtod (p, &end);
            if (end == p)
                break;
            nargs++;
            p = end;
        }
        /* skip to closing ')' */
        while (*p && *p != ')')
            p++;
        if (*p == ')')
            p++;

        /* Build the sub-matrix tmp */
        matrix_set_identity (&tmp);

        if (strcmp (fname, "translate") == 0 && nargs >= 1)
        {
            gdouble tx = args[0];
            gdouble ty = (nargs >= 2) ? args[1] : 0.0;
            tmp.m12 = (gfloat)tx;
            tmp.m13 = (gfloat)ty;
        }
        else if (strcmp (fname, "scale") == 0 && nargs >= 1)
        {
            gdouble sx = args[0];
            gdouble sy = (nargs >= 2) ? args[1] : sx;
            tmp.m0 = (gfloat)sx;
            tmp.m5 = (gfloat)sy;
        }
        else if (strcmp (fname, "rotate") == 0 && nargs >= 1)
        {
            gdouble angle = args[0] * G_PI / 180.0;
            gdouble cx = (nargs >= 3) ? args[1] : 0.0;
            gdouble cy = (nargs >= 3) ? args[2] : 0.0;
            gdouble c  = cos (angle);
            gdouble ss = sin (angle);

            if (nargs >= 3 && (cx != 0.0 || cy != 0.0))
            {
                /* translate(-cx,-cy), rotate, translate(cx,cy) */
                GrlMatrix pre, rot, post, t1;
                matrix_set_identity (&pre);
                pre.m12 = -(gfloat)cx;
                pre.m13 = -(gfloat)cy;

                matrix_set_identity (&rot);
                rot.m0 = (gfloat)c;  rot.m1 = (gfloat)ss;
                rot.m4 = -(gfloat)ss; rot.m5 = (gfloat)c;

                matrix_set_identity (&post);
                post.m12 = (gfloat)cx;
                post.m13 = (gfloat)cy;

                matrix_multiply_into (&t1,  &pre, &rot);
                matrix_multiply_into (&tmp, &t1,  &post);
            }
            else
            {
                tmp.m0 = (gfloat)c;  tmp.m1 = (gfloat)ss;
                tmp.m4 = -(gfloat)ss; tmp.m5 = (gfloat)c;
            }
        }
        else if (strcmp (fname, "matrix") == 0 && nargs >= 6)
        {
            /* SVG matrix(a,b,c,d,e,f) → column-major 4×4 */
            tmp.m0 = (gfloat)args[0]; /* a */
            tmp.m1 = (gfloat)args[1]; /* b */
            tmp.m4 = (gfloat)args[2]; /* c */
            tmp.m5 = (gfloat)args[3]; /* d */
            tmp.m12 = (gfloat)args[4]; /* e = tx */
            tmp.m13 = (gfloat)args[5]; /* f = ty */
        }
        else if (strcmp (fname, "skewx") == 0 && nargs >= 1)
        {
            tmp.m4 = (gfloat)tan (args[0] * G_PI / 180.0);
        }
        else if (strcmp (fname, "skewy") == 0 && nargs >= 1)
        {
            tmp.m1 = (gfloat)tan (args[0] * G_PI / 180.0);
        }
        /* else: unknown function — skip */

        matrix_multiply_into (&result, mat, &tmp);
        *mat = result;
    }
}

/* -------------------------------------------------------------------------
 * Arc-to-bezier conversion (SVG A/a command)
 *
 * Converts an SVG arc segment to cubic bezier segments using the standard
 * parametric form described in the SVG 1.1 specification appendix F.
 *
 * Up to 4 cubic bezier segments are emitted into *out_segments.
 * Each cubic uses 6 floats: c1x,c1y, c2x,c2y, ex,ey.
 * Returns the number of segments (0–4).
 * ------------------------------------------------------------------------- */

#define MAX_ARC_SEGMENTS 4

typedef struct
{
    gfloat c1x, c1y;
    gfloat c2x, c2y;
    gfloat ex,  ey;
} CubicSeg;

static gint
arc_to_bezier (gfloat    x1, gfloat y1,     /* start point */
               gfloat    rx, gfloat ry,      /* radii */
               gfloat    x_rot_deg,          /* x-axis-rotation */
               gint      large_arc,
               gint      sweep,
               gfloat    x2, gfloat y2,      /* end point */
               CubicSeg  out[MAX_ARC_SEGMENTS])
{
    gdouble phi;
    gdouble cos_phi, sin_phi;
    gdouble dx, dy;
    gdouble x1p, y1p;
    gdouble rxd, ryd;
    gdouble sq, fac;
    gdouble cxp, cyp;
    gdouble cx, cy;
    gdouble ux, uy, vx, vy;
    gdouble theta1, d_theta;
    gint    n_segs;
    gint    i;
    gdouble d;

    /* Degenerate: endpoints coincide → no arc */
    if (x1 == x2 && y1 == y2)
        return 0;

    if (rx == 0 || ry == 0)
    {
        /* Degenerate arc: treat as a straight line; caller handles line_to */
        out[0].c1x = x1; out[0].c1y = y1;
        out[0].c2x = x2; out[0].c2y = y2;
        out[0].ex  = x2; out[0].ey  = y2;
        return 1;
    }

    /* Force positive radii */
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;

    phi     = x_rot_deg * G_PI / 180.0;
    cos_phi = cos (phi);
    sin_phi = sin (phi);

    /* Step 1: compute (x1', y1') */
    dx  = (x1 - x2) / 2.0;
    dy  = (y1 - y2) / 2.0;
    x1p =  cos_phi * dx + sin_phi * dy;
    y1p = -sin_phi * dx + cos_phi * dy;

    /* Step 2: check / adjust radii */
    rxd = (gdouble)rx;
    ryd = (gdouble)ry;
    sq  = (x1p*x1p) / (rxd*rxd) + (y1p*y1p) / (ryd*ryd);
    if (sq > 1.0)
    {
        gdouble sqr = sqrt (sq);
        rxd *= sqr;
        ryd *= sqr;
    }

    /* Step 3: compute (cxp, cyp) */
    {
        gdouble num = rxd*rxd*ryd*ryd - rxd*rxd*y1p*y1p - ryd*ryd*x1p*x1p;
        gdouble den = rxd*rxd*y1p*y1p + ryd*ryd*x1p*x1p;
        fac = (den == 0.0) ? 0.0 : sqrt (fabs (num / den));
        if (large_arc == sweep)
            fac = -fac;
        cxp =  fac * rxd * y1p / ryd;
        cyp = -fac * ryd * x1p / rxd;
    }

    /* Step 4: compute (cx, cy) in original coords */
    cx = cos_phi * cxp - sin_phi * cyp + (x1 + x2) / 2.0;
    cy = sin_phi * cxp + cos_phi * cyp + (y1 + y2) / 2.0;

    /* Step 5: compute theta1 and d_theta */
    ux = (x1p - cxp) / rxd;
    uy = (y1p - cyp) / ryd;
    vx = (-x1p - cxp) / rxd;
    vy = (-y1p - cyp) / ryd;

    d = sqrt (ux*ux + uy*uy);
    if (d == 0.0) d = 1e-10;
    theta1 = (uy >= 0.0)
           ? acos (CLAMP (ux / d, -1.0, 1.0))
           : -acos (CLAMP (ux / d, -1.0, 1.0));

    {
        gdouble uv_cross = ux*vy - uy*vx;
        gdouble uv_dot   = ux*vx + uy*vy;
        gdouble dlen = sqrt ((ux*ux+uy*uy) * (vx*vx+vy*vy));
        if (dlen == 0.0) dlen = 1e-10;
        d_theta = (uv_cross >= 0.0)
                ? acos (CLAMP (uv_dot / dlen, -1.0, 1.0))
                : -acos (CLAMP (uv_dot / dlen, -1.0, 1.0));
    }

    if (!sweep && d_theta > 0.0)
        d_theta -= 2.0 * G_PI;
    else if (sweep && d_theta < 0.0)
        d_theta += 2.0 * G_PI;

    /* Split into ≤4 segments of ≤90 degrees each */
    n_segs = (gint)ceil (fabs (d_theta) / (G_PI / 2.0));
    if (n_segs < 1) n_segs = 1;
    if (n_segs > MAX_ARC_SEGMENTS) n_segs = MAX_ARC_SEGMENTS;

    for (i = 0; i < n_segs; i++)
    {
        gdouble t1 = theta1 + d_theta * i       / n_segs;
        gdouble t2 = theta1 + d_theta * (i + 1) / n_segs;
        gdouble dt = t2 - t1;
        /* Bezier approximation coefficient */
        gdouble alpha_k;
        gdouble sin_dt_2;
        gdouble p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;

        sin_dt_2 = sin (dt / 2.0);
        if (fabs (cos (dt / 2.0)) < 1e-10)
            alpha_k = 0.0;
        else
            alpha_k = sin (dt) * (sqrt (4.0 + 3.0 * sin_dt_2 * sin_dt_2) - 1.0) / 3.0;

        /* Endpoint 1 */
        p1x = cx + cos_phi * rxd * cos (t1) - sin_phi * ryd * sin (t1);
        p1y = cy + sin_phi * rxd * cos (t1) + cos_phi * ryd * sin (t1);
        /* Derivative direction at t1 */
        p2x = p1x + alpha_k * (-cos_phi * rxd * sin (t1) - sin_phi * ryd * cos (t1));
        p2y = p1y + alpha_k * (-sin_phi * rxd * sin (t1) + cos_phi * ryd * cos (t1));
        /* Endpoint 2 */
        p4x = cx + cos_phi * rxd * cos (t2) - sin_phi * ryd * sin (t2);
        p4y = cy + sin_phi * rxd * cos (t2) + cos_phi * ryd * sin (t2);
        /* Derivative direction at t2 */
        p3x = p4x - alpha_k * (-cos_phi * rxd * sin (t2) - sin_phi * ryd * cos (t2));
        p3y = p4y - alpha_k * (-sin_phi * rxd * sin (t2) + cos_phi * ryd * cos (t2));

        out[i].c1x = (gfloat)p2x;
        out[i].c1y = (gfloat)p2y;
        out[i].c2x = (gfloat)p3x;
        out[i].c2y = (gfloat)p3y;
        out[i].ex  = (gfloat)p4x;
        out[i].ey  = (gfloat)p4y;
    }

    return n_segs;
}

/* -------------------------------------------------------------------------
 * SVG path `d` parser
 *
 * Appends commands to an existing GrlPath.
 * Handles relative and absolute variants of: M L H V C S Q T A Z
 * Implements implicit repeated commands and smooth bezier reflection.
 * ------------------------------------------------------------------------- */

static gboolean
is_path_cmd (gchar c)
{
    gchar u = g_ascii_toupper (c);
    return u == 'M' || u == 'L' || u == 'H' || u == 'V' ||
           u == 'C' || u == 'S' || u == 'Q' || u == 'T' ||
           u == 'A' || u == 'Z';
}

static void
parse_path_d (const gchar *d,
              GrlPath     *path)
{
    const gchar *p = d;
    gchar        cmd      = 0;
    gfloat       cx = 0.f, cy = 0.f;     /* current point */
    gfloat       sx = 0.f, sy = 0.f;     /* subpath start */
    gfloat       prev_c2x = 0.f, prev_c2y = 0.f; /* for S/T reflection */
    gchar        prev_cmd = 0;
    gboolean     first = TRUE;

    while (*p)
    {
        gchar *end;
        gchar  c;

        /* skip whitespace and commas */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')
            p++;
        if (!*p)
            break;

        c = *p;

        if (is_path_cmd (c))
        {
            cmd = c;
            p++;
            first = TRUE;
        }

        /* skip whitespace again after reading command char */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')
            p++;

        if (!*p && cmd != 'Z' && cmd != 'z')
            break;

        switch (cmd)
        {
        case 'Z':
        case 'z':
            grl_path_close (path);
            cx = sx;
            cy = sy;
            first = TRUE;
            break;

        case 'M':
        case 'm':
        {
            gfloat x, y;
            x = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto done; }
            p = end;
            p = skip_ws_comma (p);
            y = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto done; }
            p = end;

            if (cmd == 'm' && !first)
            {
                x += cx;
                y += cy;
            }
            grl_path_move_to (path, x, y);
            cx = x; cy = y;
            sx = x; sy = y;
            first = FALSE;
            /* subsequent coord pairs treated as implicit L/l */
            cmd = (cmd == 'M') ? 'L' : 'l';
            break;
        }

        case 'L':
        case 'l':
        {
            gfloat x, y;
            x = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto next_cmd; }
            p = end;
            p = skip_ws_comma (p);
            y = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto next_cmd; }
            p = end;

            if (cmd == 'l') { x += cx; y += cy; }
            grl_path_line_to (path, x, y);
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        case 'H':
        case 'h':
        {
            gfloat x;
            x = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto next_cmd; }
            p = end;

            if (cmd == 'h') x += cx;
            grl_path_line_to (path, x, cy);
            cx = x;
            first = FALSE;
            break;
        }

        case 'V':
        case 'v':
        {
            gfloat y;
            y = (gfloat)g_ascii_strtod (p, &end);
            if (end == p) { goto next_cmd; }
            p = end;

            if (cmd == 'v') y += cy;
            grl_path_line_to (path, cx, y);
            cy = y;
            first = FALSE;
            break;
        }

        case 'C':
        case 'c':
        {
            gfloat c1x, c1y, c2x, c2y, x, y;
            c1x = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            c1y = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            c2x = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            c2y = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            x   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            y   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end;

            if (cmd == 'c') { c1x+=cx; c1y+=cy; c2x+=cx; c2y+=cy; x+=cx; y+=cy; }
            grl_path_cubic_to (path, c1x, c1y, c2x, c2y, x, y);
            prev_c2x = c2x; prev_c2y = c2y;
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        case 'S':
        case 's':
        {
            gfloat c1x, c1y, c2x, c2y, x, y;
            c2x = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            c2y = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            x   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            y   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end;

            if (cmd == 's') { c2x+=cx; c2y+=cy; x+=cx; y+=cy; }

            /* Reflect prev control point if previous was C/S */
            if (prev_cmd=='C'||prev_cmd=='c'||prev_cmd=='S'||prev_cmd=='s')
            {
                c1x = 2.f*cx - prev_c2x;
                c1y = 2.f*cy - prev_c2y;
            }
            else
            {
                c1x = cx;
                c1y = cy;
            }
            grl_path_cubic_to (path, c1x, c1y, c2x, c2y, x, y);
            prev_c2x = c2x; prev_c2y = c2y;
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        case 'Q':
        case 'q':
        {
            gfloat qcx, qcy, x, y;
            qcx = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            qcy = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            x   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            y   = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end;

            if (cmd == 'q') { qcx+=cx; qcy+=cy; x+=cx; y+=cy; }
            grl_path_quad_to (path, qcx, qcy, x, y);
            prev_c2x = qcx; prev_c2y = qcy;
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        case 'T':
        case 't':
        {
            gfloat qcx, qcy, x, y;
            x = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            y = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end;

            if (cmd == 't') { x+=cx; y+=cy; }

            if (prev_cmd=='Q'||prev_cmd=='q'||prev_cmd=='T'||prev_cmd=='t')
            {
                qcx = 2.f*cx - prev_c2x;
                qcy = 2.f*cy - prev_c2y;
            }
            else
            {
                qcx = cx;
                qcy = cy;
            }
            grl_path_quad_to (path, qcx, qcy, x, y);
            prev_c2x = qcx; prev_c2y = qcy;
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        case 'A':
        case 'a':
        {
            gfloat    rx, ry, x_rot, x, y;
            gint      large_arc_flag, sweep_flag;
            CubicSeg  segs[MAX_ARC_SEGMENTS];
            gint      ns;
            gint      si;

            rx         = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            ry         = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            x_rot      = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            large_arc_flag = (gint)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            sweep_flag = (gint)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            x          = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end; p=skip_ws_comma(p);
            y          = (gfloat)g_ascii_strtod (p, &end); if (end==p) { goto next_cmd; } p=end;

            if (cmd == 'a') { x += cx; y += cy; }

            ns = arc_to_bezier (cx, cy, rx, ry, x_rot,
                                large_arc_flag, sweep_flag,
                                x, y, segs);
            for (si = 0; si < ns; si++)
                grl_path_cubic_to (path,
                                   segs[si].c1x, segs[si].c1y,
                                   segs[si].c2x, segs[si].c2y,
                                   segs[si].ex,  segs[si].ey);
            cx = x; cy = y;
            first = FALSE;
            break;
        }

        default:
            /* Unknown command; skip next token */
            while (*p && !is_path_cmd (*p) &&
                   *p != ' ' && *p != '\t' &&
                   *p != '\n' && *p != '\r')
                p++;
            goto skip_prev_update;
        }

        prev_cmd = cmd;

next_cmd:
        /* loop will re-try with same cmd char */
        ;

skip_prev_update:
        ;
    }

done:
    ;
}

/* -------------------------------------------------------------------------
 * Points list parser (for <polyline> / <polygon>)
 * Returns a flat array of gfloat pairs [x0,y0, x1,y1 ...].
 * *n_pts is set to the number of XY pairs.
 * Caller frees with g_free().
 * ------------------------------------------------------------------------- */

static gfloat *
parse_points_list (const gchar *s,
                   gint        *n_pts)
{
    GArray      *arr;
    const gchar *p;
    gchar       *end;
    gfloat       v;

    arr = g_array_new (FALSE, FALSE, sizeof (gfloat));
    p = s;

    while (*p)
    {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',')
            p++;
        if (!*p)
            break;
        v = (gfloat)g_ascii_strtod (p, &end);
        if (end == p)
            break;
        g_array_append_val (arr, v);
        p = end;
    }

    *n_pts = (gint)arr->len / 2;
    return (gfloat *)g_array_free (arr, FALSE);
}

/* =========================================================================
 * Section 2: Minimal tolerant XML scanner
 *
 * We do NOT build a full DOM.  Instead we call a callback for each event:
 *   - TAG_OPEN  (name + attribute list)
 *   - TAG_CLOSE (name)
 *   - TAG_SELF  (self-closing: name + attributes)
 *
 * Limitations accepted by design:
 *   - No entity expansion (& in attribute values → literal &)
 *   - No namespace prefix handling (svg:rect treated as "svg:rect")
 *   - CDATA sections treated as text (ignored)
 *   - style="..." attribute ignored (CSS-in-SVG is out of scope)
 * ========================================================================= */

typedef enum
{
    XML_EVENT_OPEN,    /* <tag attr="val" ...>  */
    XML_EVENT_CLOSE,   /* </tag>                */
    XML_EVENT_SELF     /* <tag attr="val" ... />*/
} XmlEventType;

/* Maximum attributes per tag we keep */
#define MAX_ATTRS 32

typedef struct
{
    gchar *name;                              /* owned */
    gchar *attr_name  [MAX_ATTRS];            /* owned */
    gchar *attr_value [MAX_ATTRS];            /* owned */
    gint   n_attrs;
    XmlEventType type;
} XmlTag;

typedef void (*XmlCallback) (const XmlTag *tag, gpointer user_data);

static void
xml_tag_clear (XmlTag *tag)
{
    gint i;
    g_free (tag->name);
    tag->name = NULL;
    for (i = 0; i < tag->n_attrs; i++)
    {
        g_free (tag->attr_name[i]);
        g_free (tag->attr_value[i]);
        tag->attr_name[i] = NULL;
        tag->attr_value[i] = NULL;
    }
    tag->n_attrs = 0;
}

/*
 * parse_xml_attribute_string:
 * Parses   name="value" name2='value2' ...
 * Populates tag->attr_name/attr_value up to MAX_ATTRS.
 * Strips leading/trailing whitespace from names.
 * Handles both single and double quoted values.
 */
static void
parse_xml_attribute_string (const gchar *s,
                             XmlTag      *tag)
{
    const gchar *p = s;

    while (*p && tag->n_attrs < MAX_ATTRS)
    {
        const gchar *name_start;
        const gchar *name_end;
        gchar        quote;
        const gchar *val_start;
        const gchar *val_end;
        gsize        nlen, vlen;

        /* skip whitespace */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;
        if (!*p || *p == '/' || *p == '>')
            break;

        /* attribute name: everything up to '=' or whitespace */
        name_start = p;
        while (*p && *p != '=' && *p != ' ' && *p != '\t' &&
               *p != '\n' && *p != '\r' && *p != '/' && *p != '>')
            p++;
        name_end = p;
        if (name_end == name_start)
            break;

        /* skip whitespace after name */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;

        if (*p != '=')
        {
            /* Boolean attribute (no value) — skip */
            continue;
        }
        p++; /* skip '=' */

        /* skip whitespace before quote */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
            p++;

        if (*p != '"' && *p != '\'')
            break; /* malformed */

        quote = *p++;
        val_start = p;
        val_end = strchr (p, (gint)quote);
        if (val_end == NULL)
        {
            /* Unterminated attribute — skip to end of tag */
            break;
        }
        p = val_end + 1;

        nlen = (gsize)(name_end - name_start);
        vlen = (gsize)(val_end  - val_start);

        tag->attr_name[tag->n_attrs]  = g_strndup (name_start, nlen);
        tag->attr_value[tag->n_attrs] = g_strndup (val_start,  vlen);
        tag->n_attrs++;
    }
}

/*
 * get_attr:
 * Returns the attribute value for the given name, or NULL.
 */
static const gchar *
get_attr (const XmlTag *tag,
          const gchar  *name)
{
    gint i;
    for (i = 0; i < tag->n_attrs; i++)
    {
        if (strcmp (tag->attr_name[i], name) == 0)
            return tag->attr_value[i];
    }
    return NULL;
}

/*
 * scan_xml:
 * Scans the XML/SVG source and calls cb for each tag event.
 * Tolerates truncated input (stops early, no error).
 */
static void
scan_xml (const gchar *src,
          gsize        len,
          XmlCallback  cb,
          gpointer     user_data)
{
    const gchar *p   = src;
    const gchar *end = src + len;
    XmlTag       tag;

    while (p < end)
    {
        const gchar *lt;
        const gchar *gt;
        const gchar *q;
        gchar        namebuf[128];
        gsize        namelen;
        gboolean     is_close;
        gboolean     is_self;
        const gchar *attr_start;

        /* find next '<' */
        lt = memchr (p, '<', (gsize)(end - p));
        if (lt == NULL)
            break;
        p = lt + 1;

        /* Skip processing instructions <?...?> and DOCTYPE */
        if (p < end && (*p == '?' || *p == '!'))
        {
            /* Check for comment <!-- ... --> */
            if (p + 2 < end && p[0]=='!' && p[1]=='-' && p[2]=='-')
            {
                const gchar *ce = strstr (p + 3, "-->");
                if (ce != NULL)
                    p = ce + 3;
                else
                    p = end;
            }
            else
            {
                /* Skip to '>' */
                const gchar *ge = memchr (p, '>', (gsize)(end - p));
                if (ge != NULL)
                    p = ge + 1;
                else
                    p = end;
            }
            continue;
        }

        /* Close tag? */
        is_close = (p < end && *p == '/');
        if (is_close)
            p++;

        /* Read tag name */
        q = p;
        while (q < end && *q != ' ' && *q != '\t' && *q != '\n' &&
               *q != '\r' && *q != '>' && *q != '/')
            q++;
        namelen = (gsize)(q - p);
        if (namelen == 0)
        {
            p = q;
            continue;
        }
        if (namelen >= sizeof (namebuf))
            namelen = sizeof (namebuf) - 1;
        g_strlcpy (namebuf, p, namelen + 1);

        /* Find closing '>' */
        gt = memchr (q, '>', (gsize)(end - q));
        if (gt == NULL)
            break; /* truncated */

        /* Determine if self-closing */
        is_self = (!is_close) && (gt > q) && (*(gt - 1) == '/');

        /* Attributes live between end-of-name and '>' (or '/>' ) */
        attr_start = q;

        /* Populate tag struct */
        memset (&tag, 0, sizeof (tag));
        tag.name = g_strdup (namebuf);

        if (is_close)
        {
            tag.type = XML_EVENT_CLOSE;
        }
        else if (is_self)
        {
            tag.type = XML_EVENT_SELF;
            parse_xml_attribute_string (attr_start, &tag);
        }
        else
        {
            tag.type = XML_EVENT_OPEN;
            parse_xml_attribute_string (attr_start, &tag);
        }

        cb (&tag, user_data);
        xml_tag_clear (&tag);

        p = gt + 1;
    }
}


/* =========================================================================
 * Section 3: SVG builder
 * ========================================================================= */

/* Paint state (inheritable) */
typedef struct
{
    gboolean    has_fill;
    GrlColor    fill;
    gboolean    has_stroke;
    GrlColor    stroke;
    gfloat      stroke_width;
    GrlFillRule fill_rule;
    gfloat      opacity;         /* combined multiplier */
    gfloat      fill_opacity;
    gfloat      stroke_opacity;
} PaintState;

#define MAX_GROUP_DEPTH 64

typedef struct
{
    GPtrArray  *shapes;           /* result accumulator */
    GrlMatrix   transform_stack[MAX_GROUP_DEPTH];
    PaintState  paint_stack[MAX_GROUP_DEPTH];
    gint        depth;            /* index of current top (0 = root) */
    gboolean    parse_error;
} SvgBuilder;

static void
paint_state_init_defaults (PaintState *ps)
{
    /* SVG defaults: black fill, no stroke */
    ps->has_fill = TRUE;
    ps->fill.r = 0; ps->fill.g = 0; ps->fill.b = 0; ps->fill.a = 255;
    ps->has_stroke = FALSE;
    ps->stroke.r = 0; ps->stroke.g = 0; ps->stroke.b = 0; ps->stroke.a = 255;
    ps->stroke_width = 1.f;
    ps->fill_rule = GRL_FILL_RULE_NONZERO;
    ps->opacity = 1.f;
    ps->fill_opacity = 1.f;
    ps->stroke_opacity = 1.f;
}

/*
 * apply_paint_attrs:
 * Reads presentation attributes from a tag and updates ps.
 * Presentation attributes are inherited from the parent; this function
 * applies only those explicitly present on the current element.
 */
static void
apply_paint_attrs (const XmlTag *tag,
                   PaintState   *ps)
{
    const gchar *v;
    guint8       r, g, b;

    r = g = b = 0;

    v = get_attr (tag, "fill");
    if (v)
    {
        guint8 a = (guint8)(ps->fill_opacity * 255.f);
        if (parse_svg_color (v, &r, &g, &b, &a))
        {
            ps->has_fill = TRUE;
            ps->fill.r = r; ps->fill.g = g; ps->fill.b = b;
            ps->fill.a = (guint8)(ps->fill_opacity * 255.f);
        }
        else
        {
            ps->has_fill = FALSE;
        }
    }

    v = get_attr (tag, "stroke");
    if (v)
    {
        guint8 a = (guint8)(ps->stroke_opacity * 255.f);
        if (parse_svg_color (v, &r, &g, &b, &a))
        {
            ps->has_stroke = TRUE;
            ps->stroke.r = r; ps->stroke.g = g; ps->stroke.b = b;
            ps->stroke.a = (guint8)(ps->stroke_opacity * 255.f);
        }
        else
        {
            ps->has_stroke = FALSE;
        }
    }

    v = get_attr (tag, "stroke-width");
    if (v)
        ps->stroke_width = (gfloat)g_ascii_strtod (v, NULL);

    v = get_attr (tag, "fill-rule");
    if (v)
    {
        if (strcmp (v, "evenodd") == 0)
            ps->fill_rule = GRL_FILL_RULE_EVEN_ODD;
        else
            ps->fill_rule = GRL_FILL_RULE_NONZERO;
    }

    v = get_attr (tag, "opacity");
    if (v)
    {
        ps->opacity = (gfloat)CLAMP (g_ascii_strtod (v, NULL), 0.0, 1.0);
        /* Apply to both channels */
        ps->fill.a   = (guint8)(ps->fill.a   * ps->opacity);
        ps->stroke.a = (guint8)(ps->stroke.a * ps->opacity);
    }

    v = get_attr (tag, "fill-opacity");
    if (v)
    {
        ps->fill_opacity = (gfloat)CLAMP (g_ascii_strtod (v, NULL), 0.0, 1.0);
        ps->fill.a = (guint8)(255.f * ps->fill_opacity * ps->opacity);
    }

    v = get_attr (tag, "stroke-opacity");
    if (v)
    {
        ps->stroke_opacity = (gfloat)CLAMP (g_ascii_strtod (v, NULL), 0.0, 1.0);
        ps->stroke.a = (guint8)(255.f * ps->stroke_opacity * ps->opacity);
    }
}

/*
 * emit_shape:
 * Applies the current transform, then appends a new GrlVectorShape to
 * the builder's shape list.
 */
static void
emit_shape (SvgBuilder       *bld,
            GrlPath          *path,
            const PaintState *ps)
{
    GrlVectorShape  *shape;
    const GrlMatrix *mat;

    if (grl_path_is_empty (path))
        return;

    shape = grl_vector_shape_new ();

    /* Replace the empty path with a copy of our built one */
    g_object_unref (shape->path);
    shape->path = grl_path_copy (path);

    /* Bake the current transform into the path */
    mat = &bld->transform_stack[bld->depth];
    grl_path_transform (shape->path, mat);

    /* Paint */
    shape->has_fill    = ps->has_fill;
    shape->fill        = ps->fill;
    shape->has_stroke  = ps->has_stroke;
    shape->stroke      = ps->stroke;
    shape->stroke_width = ps->stroke_width;
    shape->fill_rule   = ps->fill_rule;

    g_ptr_array_add (bld->shapes, shape);
}

/* Convert a string attribute to float, with a fallback default */
static gfloat
attr_float (const XmlTag *tag,
            const gchar  *name,
            gfloat        def)
{
    const gchar *v = get_attr (tag, name);
    if (v == NULL)
        return def;
    return (gfloat)g_ascii_strtod (v, NULL);
}

/* -------------------------------------------------------------------------
 * Shape element handlers
 * ------------------------------------------------------------------------- */

static void
handle_path (const XmlTag *tag,
             SvgBuilder   *bld)
{
    const gchar    *d;
    g_autoptr(GrlPath) path = NULL;
    PaintState      ps;

    d = get_attr (tag, "d");
    if (d == NULL)
        return;

    /* Copy current paint state and apply element-level overrides */
    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);

    path = grl_path_new ();
    parse_path_d (d, path);
    emit_shape (bld, path, &ps);
}

static void
handle_rect (const XmlTag *tag,
             SvgBuilder   *bld)
{
    gfloat          x, y, w, h;
    PaintState      ps;
    GrlRectangle    rect;
    g_autoptr(GrlPath) path = NULL;

    x = attr_float (tag, "x", 0.f);
    y = attr_float (tag, "y", 0.f);
    w = attr_float (tag, "width", 0.f);
    h = attr_float (tag, "height", 0.f);

    if (w <= 0 || h <= 0)
        return;

    /* TODO: rounded corners via rx/ry could be added later */

    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);

    rect.x = x; rect.y = y; rect.width = w; rect.height = h;
    path = grl_path_new ();
    grl_path_add_rect (path, &rect);
    emit_shape (bld, path, &ps);
}

static void
handle_circle (const XmlTag *tag,
               SvgBuilder   *bld)
{
    gfloat          cx, cy, r;
    PaintState      ps;
    g_autoptr(GrlPath) path = NULL;

    cx = attr_float (tag, "cx", 0.f);
    cy = attr_float (tag, "cy", 0.f);
    r  = attr_float (tag, "r",  0.f);

    if (r <= 0)
        return;

    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);

    path = grl_path_new ();
    grl_path_add_circle (path, cx, cy, r);
    emit_shape (bld, path, &ps);
}

static void
handle_ellipse (const XmlTag *tag,
                SvgBuilder   *bld)
{
    gfloat          cx, cy, rx, ry;
    PaintState      ps;
    g_autoptr(GrlPath) path = NULL;

    cx = attr_float (tag, "cx", 0.f);
    cy = attr_float (tag, "cy", 0.f);
    rx = attr_float (tag, "rx", 0.f);
    ry = attr_float (tag, "ry", 0.f);

    if (rx <= 0 || ry <= 0)
        return;

    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);

    path = grl_path_new ();
    grl_path_add_ellipse (path, cx, cy, rx, ry);
    emit_shape (bld, path, &ps);
}

static void
handle_line (const XmlTag *tag,
             SvgBuilder   *bld)
{
    gfloat          x1, y1, x2, y2;
    PaintState      ps;
    g_autoptr(GrlPath) path = NULL;

    x1 = attr_float (tag, "x1", 0.f);
    y1 = attr_float (tag, "y1", 0.f);
    x2 = attr_float (tag, "x2", 0.f);
    y2 = attr_float (tag, "y2", 0.f);

    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);
    /* <line> has no fill by convention */
    ps.has_fill = FALSE;
    if (!ps.has_stroke)
    {
        ps.has_stroke = TRUE;
        ps.stroke.r = ps.stroke.g = ps.stroke.b = 0;
        ps.stroke.a = 255;
    }

    path = grl_path_new ();
    grl_path_move_to (path, x1, y1);
    grl_path_line_to (path, x2, y2);
    emit_shape (bld, path, &ps);
}

static void
handle_polyline_or_polygon (const XmlTag *tag,
                            SvgBuilder   *bld,
                            gboolean      close)
{
    const gchar    *pts_attr;
    gfloat         *pts;
    gint            n;
    gint            i;
    PaintState      ps;
    g_autoptr(GrlPath) path = NULL;

    pts_attr = get_attr (tag, "points");
    if (pts_attr == NULL)
        return;

    pts = parse_points_list (pts_attr, &n);
    if (n < 2)
    {
        g_free (pts);
        return;
    }

    ps = bld->paint_stack[bld->depth];
    apply_paint_attrs (tag, &ps);

    path = grl_path_new ();
    grl_path_move_to (path, pts[0], pts[1]);
    for (i = 1; i < n; i++)
        grl_path_line_to (path, pts[i*2], pts[i*2+1]);
    if (close)
        grl_path_close (path);

    g_free (pts);
    emit_shape (bld, path, &ps);
}

/* -------------------------------------------------------------------------
 * XML event handler
 * ------------------------------------------------------------------------- */

static void
svg_xml_callback (const XmlTag *tag,
                  gpointer      user_data)
{
    SvgBuilder  *bld = (SvgBuilder *)user_data;
    const gchar *name = tag->name;
    const gchar *transform_attr;

    /* Lower-case tag name for comparison; handle optional "svg:" prefix */
    {
        gchar lower[64];
        gsize i, len;
        len = strlen (name);
        if (len > 4 && g_ascii_strncasecmp (name, "svg:", 4) == 0)
        {
            name += 4;
            len  -= 4;
        }
        if (len >= sizeof (lower))
            len = sizeof (lower) - 1;
        for (i = 0; i < len; i++)
            lower[i] = g_ascii_tolower (name[i]);
        lower[len] = '\0';
        name = lower;

        /* Handle open/self-closing tags */
        if (tag->type == XML_EVENT_OPEN || tag->type == XML_EVENT_SELF)
        {
            if (strcmp (name, "svg") == 0)
            {
                /* Root SVG element — just enter a group */
                if (bld->depth < MAX_GROUP_DEPTH - 1)
                {
                    gint nd = bld->depth + 1;
                    bld->transform_stack[nd] = bld->transform_stack[bld->depth];
                    bld->paint_stack[nd]     = bld->paint_stack[bld->depth];
                    apply_paint_attrs (tag, &bld->paint_stack[nd]);
                    bld->depth = nd;
                }
            }
            else if (strcmp (name, "g") == 0)
            {
                if (bld->depth < MAX_GROUP_DEPTH - 1)
                {
                    GrlMatrix *child_mat;
                    gint       nd = bld->depth + 1;

                    bld->transform_stack[nd] = bld->transform_stack[bld->depth];
                    bld->paint_stack[nd]     = bld->paint_stack[bld->depth];
                    apply_paint_attrs (tag, &bld->paint_stack[nd]);

                    child_mat = &bld->transform_stack[nd];
                    transform_attr = get_attr (tag, "transform");
                    if (transform_attr)
                        parse_svg_transform (transform_attr, child_mat);

                    bld->depth = nd;
                }
            }
            else if (strcmp (name, "path") == 0)
            {
                transform_attr = get_attr (tag, "transform");
                if (transform_attr)
                {
                    /* Temporarily compose transform */
                    GrlMatrix saved = bld->transform_stack[bld->depth];
                    parse_svg_transform (transform_attr, &bld->transform_stack[bld->depth]);
                    handle_path (tag, bld);
                    bld->transform_stack[bld->depth] = saved;
                }
                else
                {
                    handle_path (tag, bld);
                }
            }
            else if (strcmp (name, "rect") == 0)
            {
                transform_attr = get_attr (tag, "transform");
                if (transform_attr)
                {
                    GrlMatrix saved = bld->transform_stack[bld->depth];
                    parse_svg_transform (transform_attr, &bld->transform_stack[bld->depth]);
                    handle_rect (tag, bld);
                    bld->transform_stack[bld->depth] = saved;
                }
                else
                {
                    handle_rect (tag, bld);
                }
            }
            else if (strcmp (name, "circle") == 0)
            {
                handle_circle (tag, bld);
            }
            else if (strcmp (name, "ellipse") == 0)
            {
                handle_ellipse (tag, bld);
            }
            else if (strcmp (name, "line") == 0)
            {
                handle_line (tag, bld);
            }
            else if (strcmp (name, "polyline") == 0)
            {
                handle_polyline_or_polygon (tag, bld, FALSE);
            }
            else if (strcmp (name, "polygon") == 0)
            {
                handle_polyline_or_polygon (tag, bld, TRUE);
            }
            /* All other elements (text, defs, linearGradient, etc.) are skipped. */
        }
        else if (tag->type == XML_EVENT_CLOSE)
        {
            if ((strcmp (name, "g")   == 0 ||
                 strcmp (name, "svg") == 0) &&
                bld->depth > 0)
            {
                bld->depth--;
            }
        }
    }
}

/*
 * build_shapes:
 * Core implementation: parse SVG source and return a GPtrArray of
 * GrlVectorShape* objects.  On hard parse error (completely invalid XML),
 * returns NULL and sets error.  An empty but valid SVG returns an empty array.
 */
static GPtrArray *
build_shapes (const gchar *src,
              gsize        len,
              GError     **error)
{
    SvgBuilder bld;
    gint       i;

    /* Minimal sanity: must contain '<' */
    if (len == 0 || memchr (src, '<', len) == NULL)
    {
        g_set_error (error, GRL_SVG_ERROR, GRL_SVG_ERROR_PARSE,
                     "SVG data does not contain any XML tags");
        return NULL;
    }

    memset (&bld, 0, sizeof (bld));
    bld.shapes = g_ptr_array_new_with_free_func ((GDestroyNotify)grl_vector_shape_free);
    bld.depth = 0;
    bld.parse_error = FALSE;

    /* Initialise root transform = identity */
    matrix_set_identity (&bld.transform_stack[0]);
    paint_state_init_defaults (&bld.paint_stack[0]);

    /* Also initialise all stack slots (avoids reading uninitialised memory) */
    for (i = 1; i < MAX_GROUP_DEPTH; i++)
    {
        matrix_set_identity (&bld.transform_stack[i]);
        paint_state_init_defaults (&bld.paint_stack[i]);
    }

    scan_xml (src, len, svg_xml_callback, &bld);

    return bld.shapes;
}

/*
 * ptr_array_to_shape_array:
 * Transfers ownership of shapes out of the GPtrArray into a plain
 * C array for the public API.  Sets *n_shapes.  Caller frees each element
 * and then the array itself.
 */
static GrlVectorShape **
ptr_array_to_shape_array (GPtrArray *arr,
                          guint     *n_shapes)
{
    GrlVectorShape **result;
    guint            n;
    guint            i;

    n = arr->len;
    *n_shapes = n;

    result = g_new0 (GrlVectorShape *, n + 1); /* NULL-terminated */
    for (i = 0; i < n; i++)
    {
        result[i] = (GrlVectorShape *)g_ptr_array_index (arr, i);
        g_ptr_array_index (arr, i) = NULL; /* steal ownership */
    }

    /* Free array without calling destroy on (now-stolen) elements */
    g_ptr_array_set_free_func (arr, NULL);
    g_ptr_array_free (arr, TRUE);

    return result;
}


/* =========================================================================
 * Section 4: Public API
 * ========================================================================= */

/**
 * grl_svg_load_from_memory:
 * @data: SVG source bytes.
 * @len: Length of @data.
 * @dpi: Device pixels per inch (0 → 96).
 * @n_shapes: (out): Number of returned shapes.
 * @error: (nullable): Return location for error.
 *
 * Parses SVG from an in-memory buffer.
 *
 * Returns: (transfer full) (array length=n_shapes) (nullable): Shape array.
 */
GrlVectorShape **
grl_svg_load_from_memory (const gchar  *data,
                          gsize         len,
                          gfloat        dpi,
                          guint        *n_shapes,
                          GError      **error)
{
    GPtrArray       *arr;
    GrlVectorShape **result;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (n_shapes != NULL, NULL);

    (void)dpi; /* Reserved for future unit conversion; currently unused */

    arr = build_shapes (data, len, error);
    if (arr == NULL)
    {
        *n_shapes = 0;
        return NULL;
    }

    result = ptr_array_to_shape_array (arr, n_shapes);
    return result;
}

/**
 * grl_svg_load_from_file:
 * @filename: (type filename): Path to an SVG file.
 * @dpi: Device pixels per inch (0 → 96).
 * @n_shapes: (out): Number of returned shapes.
 * @error: (nullable): Return location for error.
 *
 * Parses an SVG file.
 *
 * Returns: (transfer full) (array length=n_shapes) (nullable): Shape array.
 */
GrlVectorShape **
grl_svg_load_from_file (const gchar  *filename,
                        gfloat        dpi,
                        guint        *n_shapes,
                        GError      **error)
{
    gchar           *contents = NULL;
    gsize            length   = 0;
    GrlVectorShape **result;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (n_shapes != NULL, NULL);

    if (!g_file_get_contents (filename, &contents, &length, error))
    {
        if (error && *error)
        {
            GError *orig = *error;
            *error = NULL;
            g_set_error (error, GRL_SVG_ERROR, GRL_SVG_ERROR_OPEN,
                         "Could not open '%s': %s", filename, orig->message);
            g_error_free (orig);
        }
        *n_shapes = 0;
        return NULL;
    }

    result = grl_svg_load_from_memory (contents, length, dpi, n_shapes, error);
    g_free (contents);
    return result;
}

/**
 * grl_path_new_from_svg_file:
 * @filename: (type filename): Path to an SVG file.
 * @dpi: Device pixels per inch (0 → 96).
 * @error: (nullable): Return location for error.
 *
 * Convenience: loads all shapes and merges their paths into one.
 *
 * Returns: (transfer full) (nullable): A new merged #GrlPath.
 */
GrlPath *
grl_path_new_from_svg_file (const gchar  *filename,
                             gfloat        dpi,
                             GError      **error)
{
    GrlVectorShape **shapes;
    guint            n;
    GrlPath         *merged;
    guint            i;

    g_return_val_if_fail (filename != NULL, NULL);

    shapes = grl_svg_load_from_file (filename, dpi, &n, error);
    if (shapes == NULL)
        return NULL;

    merged = grl_path_new ();

    /* For a real merge we'd need a "concat subpaths" API; since we don't have
     * one, we flatten and re-emit as line segments.  This is sufficient for
     * the stated purpose (obtaining a combined outline path). */
    for (i = 0; i < n; i++)
    {
        guint      *lengths     = NULL;
        guint       n_subpaths  = 0;
        guint       total_pts   = 0;
        GrlVector2 *pts;
        guint       sp;
        guint       pt_idx = 0;

        pts = grl_path_get_flattened (shapes[i]->path, 0.f,
                                      &lengths, &n_subpaths, &total_pts);
        if (pts != NULL)
        {
            for (sp = 0; sp < n_subpaths; sp++)
            {
                guint np = lengths[sp];
                guint k;
                if (np == 0) { pt_idx += np; continue; }
                grl_path_move_to (merged, pts[pt_idx].x, pts[pt_idx].y);
                for (k = 1; k < np; k++)
                    grl_path_line_to (merged, pts[pt_idx + k].x, pts[pt_idx + k].y);
                pt_idx += np;
            }
            g_free (pts);
            g_free (lengths);
        }

        grl_vector_shape_free (shapes[i]);
    }
    g_free (shapes);

    return merged;
}


/* -------------------------------------------------------------------------
 * Export helpers
 * ------------------------------------------------------------------------- */

/*
 * path_cmd_to_svg_string:
 * Appends SVG path command strings to str for all subpaths.
 * Uses only M, L, C, Q, Z.
 *
 * This function relies on GrlPath's public API.  Since we can only read a
 * GrlPath via get_flattened() (there is no verb-level iterator in M3.1),
 * we flatten and emit M/L/Z.  This is geometry-lossless to within the
 * flattening tolerance.
 */
static void
append_path_d (GString *str,
               GrlPath *path)
{
    guint       *lengths    = NULL;
    guint        n_subpaths = 0;
    guint        total_pts  = 0;
    GrlVector2  *pts;
    guint        sp;
    guint        pt_idx = 0;
    gboolean     first_cmd = TRUE;

    pts = grl_path_get_flattened (path, 0.25f,
                                  &lengths, &n_subpaths, &total_pts);
    if (pts == NULL)
    {
        g_string_append (str, "M 0 0");
        return;
    }

    for (sp = 0; sp < n_subpaths; sp++)
    {
        guint np = lengths[sp];
        guint k;

        if (np == 0) { pt_idx += np; continue; }

        if (!first_cmd)
            g_string_append_c (str, ' ');
        first_cmd = FALSE;

        g_string_append_printf (str, "M %g %g",
                                (gdouble)pts[pt_idx].x,
                                (gdouble)pts[pt_idx].y);
        for (k = 1; k < np; k++)
        {
            g_string_append_printf (str, " L %g %g",
                                    (gdouble)pts[pt_idx + k].x,
                                    (gdouble)pts[pt_idx + k].y);
        }
        g_string_append (str, " Z");
        pt_idx += np;
    }

    g_free (pts);
    g_free (lengths);
}

/*
 * color_to_hex6:
 * Writes "#rrggbb" into buf (must be ≥8 bytes).
 */
static void
color_to_hex6 (const GrlColor *c,
               gchar           buf[8])
{
    g_snprintf (buf, 8, "#%02x%02x%02x", c->r, c->g, c->b);
}

/*
 * build_svg_string:
 * Core serialiser — builds the SVG document in a GString.
 */
static GString *
build_svg_string (GrlVectorShape * const *shapes,
                  guint                   n_shapes,
                  gint                    width,
                  gint                    height)
{
    GString *s;
    guint    i;

    s = g_string_new (NULL);

    g_string_append_printf (s,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\""
        " width=\"%d\" height=\"%d\""
        " viewBox=\"0 0 %d %d\">\n",
        width, height, width, height);

    for (i = 0; i < n_shapes; i++)
    {
        GrlVectorShape *sh = shapes[i];
        GString        *d;
        gchar           fill_hex[8];
        gchar           stroke_hex[8];

        d = g_string_new (NULL);
        append_path_d (d, sh->path);

        g_string_append_printf (s, "  <path d=\"%s\"", d->str);
        g_string_free (d, TRUE);

        /* fill */
        if (sh->has_fill)
        {
            color_to_hex6 (&sh->fill, fill_hex);
            g_string_append_printf (s, " fill=\"%s\"", fill_hex);
            if (sh->fill.a < 255)
                g_string_append_printf (s, " fill-opacity=\"%.9g\"",
                                        (gdouble)sh->fill.a / 255.0);
        }
        else
        {
            g_string_append (s, " fill=\"none\"");
        }

        /* stroke */
        if (sh->has_stroke)
        {
            color_to_hex6 (&sh->stroke, stroke_hex);
            g_string_append_printf (s, " stroke=\"%s\"", stroke_hex);
            if (sh->stroke.a < 255)
                g_string_append_printf (s, " stroke-opacity=\"%.9g\"",
                                        (gdouble)sh->stroke.a / 255.0);
            g_string_append_printf (s, " stroke-width=\"%g\"",
                                    (gdouble)sh->stroke_width);
        }
        else
        {
            g_string_append (s, " stroke=\"none\"");
        }

        /* fill-rule */
        if (sh->fill_rule == GRL_FILL_RULE_EVEN_ODD)
            g_string_append (s, " fill-rule=\"evenodd\"");

        g_string_append (s, "/>\n");
    }

    g_string_append (s, "</svg>\n");
    return s;
}

/**
 * grl_svg_to_string:
 * @shapes: (array length=n_shapes): Shapes to serialise.
 * @n_shapes: Number of shapes.
 * @width: Canvas width.
 * @height: Canvas height.
 *
 * Returns: (transfer full): SVG string; free with g_free().
 */
gchar *
grl_svg_to_string (GrlVectorShape * const *shapes,
                   guint                   n_shapes,
                   gint                    width,
                   gint                    height)
{
    GString *s;

    g_return_val_if_fail (shapes != NULL || n_shapes == 0, NULL);

    s = build_svg_string (shapes, n_shapes, width, height);
    return g_string_free (s, FALSE);
}

/**
 * grl_svg_save_to_file:
 * @shapes: (array length=n_shapes): Shapes to serialise.
 * @n_shapes: Number of shapes.
 * @width: Canvas width.
 * @height: Canvas height.
 * @filename: (type filename): Output path.
 * @error: (nullable): Return location for error.
 *
 * Writes an SVG file.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean
grl_svg_save_to_file (GrlVectorShape * const *shapes,
                      guint                   n_shapes,
                      gint                    width,
                      gint                    height,
                      const gchar            *filename,
                      GError                **error)
{
    GString  *s;
    gboolean  ok;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (shapes != NULL || n_shapes == 0, FALSE);

    s = build_svg_string (shapes, n_shapes, width, height);

    ok = g_file_set_contents (filename, s->str, (gssize)s->len, error);
    if (!ok && error && *error)
    {
        GError *orig = *error;
        *error = NULL;
        g_set_error (error, GRL_SVG_ERROR, GRL_SVG_ERROR_OPEN,
                     "Could not write '%s': %s", filename, orig->message);
        g_error_free (orig);
    }

    g_string_free (s, TRUE);
    return ok;
}

/**
 * grl_path_to_svg_string:
 * @self: A #GrlPath.
 *
 * Returns: (transfer full): Bare SVG path `d` string; free with g_free().
 */
gchar *
grl_path_to_svg_string (GrlPath *self)
{
    GString *s;

    g_return_val_if_fail (GRL_IS_PATH (self), NULL);

    s = g_string_new (NULL);
    append_path_d (s, self);
    return g_string_free (s, FALSE);
}

/**
 * grl_image_draw_svg_shapes:
 * @self: A #GrlImage.
 * @shapes: (array length=n_shapes): Shapes to render.
 * @n_shapes: Number of shapes.
 *
 * Renders each shape onto @self: fill then stroke.
 */
void
grl_image_draw_svg_shapes (GrlImage              *self,
                           GrlVectorShape * const *shapes,
                           guint                   n_shapes)
{
    guint i;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (shapes != NULL || n_shapes == 0);

    for (i = 0; i < n_shapes; i++)
    {
        GrlVectorShape *sh = shapes[i];

        if (sh == NULL || sh->path == NULL)
            continue;

        if (sh->has_fill)
            grl_image_fill_path (self, sh->path, sh->fill_rule, &sh->fill);

        if (sh->has_stroke && sh->stroke_width > 0)
        {
            gint thickness = (gint)MAX (1, (gint)roundf (sh->stroke_width));
            grl_image_stroke_path (self, sh->path, thickness, &sh->stroke);
        }
    }
}
