/* Engine Copyright (c) 2022 Engine Development Team
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

#include <math.h>

#include "auxiliary/math.h"

namespace GfxRenderEngine
{
    namespace Math
    {
        float Linear0_1ToExponential0_256(float input)
        {
            float value, x;

            // clamp to [0.0f, 1.0f]
            x = std::max(0.0f, input);
            x = std::min(1.0f, x);

            value = exp(5.6 * x) - 1;

            // clamp to [0.0f, 256.0f]
            value = std::max(0.0f, value);
            value = std::min(256.0f, value);
            return value;
        }

        float Linear0_1ToExponential256_0(float input) { return Linear0_1ToExponential0_256(1.0f - input); }
    } // namespace Math
} // namespace GfxRenderEngine
