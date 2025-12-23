#!/usr/bin/env python3
# basic-window.py
#
# Copyright 2025 Zach Podbielniak
#
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Basic window example using Graylib Python bindings via GObject Introspection.
#
# Requirements:
#   - PyGObject (python3-gobject on Fedora, python3-gi on Debian/Ubuntu)
#   - Graylib library built with GIR support (make gir)
#
# Usage:
#   GI_TYPELIB_PATH=../build/gir LD_LIBRARY_PATH=../build/lib python3 basic-window.py
#
# Or after installation:
#   python3 basic-window.py

import gi
gi.require_version('Graylib', '1')
from gi.repository import Graylib as Grl

def main() -> int:
    """
    Basic window example demonstrating Graylib Python bindings.

    Creates a window and displays a simple animation with shapes.
    """
    # Initialize application
    app: Grl.Application = Grl.Application.new("com.example.basic-window-py")

    # Create window
    window: Grl.Window = Grl.Window.new(800, 600, "Graylib - Python Example")
    window.set_target_fps(60)

    # Create colors
    bg_color: Grl.Color = Grl.Color.new(40, 40, 60, 255)
    white: Grl.Color = Grl.Color.new(255, 255, 255, 255)
    red: Grl.Color = Grl.Color.new(230, 41, 55, 255)
    green: Grl.Color = Grl.Color.new(0, 228, 48, 255)
    blue: Grl.Color = Grl.Color.new(0, 121, 241, 255)

    # Animation state
    ball_x: float = 400.0
    ball_y: float = 300.0
    ball_vx: float = 200.0
    ball_vy: float = 150.0
    ball_radius: float = 30.0

    # Main loop
    while not window.should_close():
        # Get frame time
        dt: float = window.get_frame_time()

        # Update ball position
        ball_x += ball_vx * dt
        ball_y += ball_vy * dt

        # Bounce off walls
        if ball_x - ball_radius < 0 or ball_x + ball_radius > 800:
            ball_vx = -ball_vx
            ball_x = max(ball_radius, min(800 - ball_radius, ball_x))

        if ball_y - ball_radius < 0 or ball_y + ball_radius > 600:
            ball_vy = -ball_vy
            ball_y = max(ball_radius, min(600 - ball_radius, ball_y))

        # Begin drawing
        window.begin_drawing()
        Grl.Draw.clear_background(bg_color)

        # Draw title
        Grl.Draw.text("Graylib - Python GI Bindings Example", 10, 10, 20, white)
        Grl.Draw.text("Bouncing ball animation", 10, 35, 16, white)

        # Draw bouncing ball
        Grl.Draw.circle(int(ball_x), int(ball_y), ball_radius, red)
        Grl.Draw.circle_lines(int(ball_x), int(ball_y), ball_radius, white)

        # Draw some static shapes
        Grl.Draw.rectangle(50, 500, 100, 50, green)
        Grl.Draw.rectangle_lines(50, 500, 100, 50, white)

        Grl.Draw.rectangle(650, 500, 100, 50, blue)
        Grl.Draw.rectangle_lines(650, 500, 100, 50, white)

        # Draw FPS
        Grl.Draw.fps(10, 570)

        window.end_drawing()

    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
