/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
   https://github.com/beaumanvienna/vulkan

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <cmath>

namespace GfxRenderEngine
{

    struct Point
    {
        Point() : x(0.0f), y(0.0f) {}
        Point(float x_, float y_) : x(x_), y(y_) {}

        float x;
        float y;

        float distanceTo(const Point& other) const
        {
            float dx = other.x - x, dy = other.y - y;
            return sqrtf(dx * dx + dy * dy);
        }

        bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    };

    struct Bounds
    {
        Bounds() : x(0), y(0), w(0), h(0) {}
        Bounds(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

        bool Contains(float px, float py) const { return (px >= x && py >= y && px < x + w && py < y + h); }

        bool Intersects(const Bounds& other) const
        {
            return !(x > other.x2() || x2() < other.x || y > other.y2() || y2() < other.y);
        }

        void Clip(const Bounds& clipTo)
        {
            if (x < clipTo.x)
            {
                w -= clipTo.x - x;
                x = clipTo.x;
            }
            if (y < clipTo.y)
            {
                h -= clipTo.y - y;
                y = clipTo.y;
            }
            if (x2() > clipTo.x2())
            {
                w = clipTo.x2() - x;
            }
            if (y2() > clipTo.y2())
            {
                h = clipTo.y2() - y;
            }
        }

        float x2() const { return x + w; }
        float y2() const { return y + h; }
        float centerX() const { return x + w * 0.5f; }
        float centerY() const { return y + h * 0.5f; }
        Point Center() const { return Point(centerX(), centerY()); }
        Bounds Expand(float amount) const { return Bounds(x - amount, y - amount, w + amount * 2, h + amount * 2); }
        Bounds Offset(float xAmount, float yAmount) const { return Bounds(x + xAmount, y + yAmount, w, h); }

        float x;
        float y;
        float w;
        float h;
    };
} // namespace GfxRenderEngine
