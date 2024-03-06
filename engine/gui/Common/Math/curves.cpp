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

#include <cmath>
#include "gui/Common/Math/curves.h"

namespace GfxRenderEngine
{
    // float linearInOut(int t, int fadeInLength, int solidLength, int fadeOutLength)
    //{
    //     if (t < 0) return 0;
    //     if (t < fadeInLength)
    //     {
    //         return (float)t / fadeInLength;
    //     }
    //     t -= fadeInLength;
    //     if (t < solidLength)
    //     {
    //         return 1.0f;
    //     }
    //     t -= solidLength;
    //     if (t < fadeOutLength)
    //     {
    //         return 1.0f - (float)t / fadeOutLength;
    //     }
    //     return 0.0f;
    // }
    //
    // float linearIn(int t, int fadeInLength)
    //{
    //     if (t < 0) return 0;
    //     if (t < fadeInLength)
    //     {
    //         return (float)t / fadeInLength;
    //     }
    //     return 1.0f;
    // }
    //
    // float linearOut(int t, int fadeOutLength)
    //{
    //     return 1.0f - linearIn(t, fadeOutLength);
    // }
    //
    // float ease(float val)
    //{
    //     if (val > 1.0f) return 1.0f;
    //     if (val < 0.0f) return 0.0f;
    //     return ((-cosf(val * PI)) + 1.0f) * 0.5;
    // }
    //
    // float ease(int t, int fadeLength)
    //{
    //     if (t < 0) return 0.0f;
    //     if (t >= fadeLength) return 1.0f;
    //     return ease((float)t / (float)fadeLength);
    // }

    template <int hundredthsX1, int hundredthsX2, int hundredthsY1 = 0, int hundredthsY2 = 100>
    inline float bezierEaseFunc(float val)
    {
        constexpr float x1 = hundredthsX1 / 100.0f;
        constexpr float x2 = hundredthsX2 / 100.0f;
        constexpr float a = 1.0f - 3.0f * x2 + 3.0f * x1;
        constexpr float b = 3.0f * x2 - 6.0f * x1;
        constexpr float c = 3.0f * x1;

        constexpr float y1 = hundredthsY1 / 100.0f;
        constexpr float y2 = hundredthsY2 / 100.0f;
        constexpr float ya = 1.0f - 3.0f * y2 + 3.0f * y1;
        constexpr float yb = 3.0f * y2 - 6.0f * y1;
        constexpr float yc = 3.0f * y1;

        float guess = val;
        // Newton-Raphson calculation
        for (int i = 0; i < 4; ++i)
        {
            float slope = 3.0f * a * guess * guess + 2.0f * b * guess + c;
            if (slope == 0.0f)
            {
                break;
            }

            float x = ((a * guess + b) * guess + c) * guess - val;
            guess -= x / slope;
        }

        return ((ya * guess + yb) * guess + yc) * guess;
    }

    // float bezierEase(float val)
    //{
    //     return bezierEaseFunc<25, 25, 10, 100>(val);
    // }

    float bezierEaseInOut(float val) { return bezierEaseFunc<42, 58>(val); }

    // float bezierEaseIn(float val)
    //{
    //     return bezierEaseFunc<42, 100>(val);
    // }
    //
    // float bezierEaseOut(float val)
    //{
    //     return bezierEaseFunc<0, 58>(val);
    // }
    //
    // float sawtooth(int t, int period)
    //{
    //     return (t % period) * (1.0f / (period - 1));
    // }
    //
    // float passWithPause(int t, int fadeInLength, int pauseLength, int fadeOutLength)
    //{
    //     if (t < fadeInLength)
    //     {
    //         return -1.0f + (float)t / fadeInLength;
    //     }
    //     t -= fadeInLength;
    //     if (t < pauseLength)
    //     {
    //         return 0.0f;
    //     }
    //     t -= pauseLength;
    //     if (t < fadeOutLength)
    //     {
    //         return (float)t / fadeOutLength;
    //     }
    //     return 1.0f;
    // }
} // namespace GfxRenderEngine
