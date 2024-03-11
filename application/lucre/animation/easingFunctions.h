/* Engine Copyright (c) 2024 Engine Development Team
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

#include "engine.h"
#include "animation/easingAnimation.h"

namespace LucreApp
{

    class EaseConstant : public EasingAnimation
    {
    public:
        EaseConstant(std::string const& name, float scale, float offset, bool invert = false)
            : EasingAnimation(name, scale, offset, invert)
        {
        }

    private:
        virtual float EasingFunction(float time) override
        {
            float returnValue = m_Scale * 1.0f;
            return returnValue;
        }

    private:
    };

    class EaseInOutQuart : public EasingAnimation
    {
    public:
        EaseInOutQuart(std::string const& name, float scale, float offset, bool invert = false)
            : EasingAnimation(name, scale, offset, invert)
        {
        }

    private:
        virtual float EasingFunction(float time) override
        {
            float returnValue = m_Scale * ((time < 0.5f) ? (8.0f * time * time * time * time)
                                                         : (1.0f - std::pow(-2.0f * time + 2.0f, 4.0f) / 2.0f));
            return returnValue;
        }

    private:
    };

} // namespace LucreApp
