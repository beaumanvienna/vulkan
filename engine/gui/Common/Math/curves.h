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

namespace GfxRenderEngine
{
    // output range: [0.0, 1.0]
    float linearInOut(int t, int fadeInLength, int solidLength, int fadeOutLength);
    float linearIn(int t, int fadeInLength);
    float linearOut(int t, int fadeInLength);

    // smooth operator [0, 1] -> [0, 1]
    float ease(float val);
    float ease(int t, int fadeLength);

    float bezierEase(float val);
    float bezierEaseInOut(float val);
    float bezierEaseIn(float val);
    float bezierEaseOut(float val);

    // waveforms [0, 1]
    float sawtooth(int t, int period);

    // output range: -1.0 to 1.0
    float passWithPause(int t, int fadeInLength, int pauseLength, int fadeOutLength);
} // namespace GfxRenderEngine
