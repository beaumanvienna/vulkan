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

namespace LucreApp
{

    class EasingAnimation
    {
    public:
        EasingAnimation(std::string const& name, float scale, float offset, bool invert = false)
            : m_Scale{scale}, m_Invert{invert}, m_Offset{offset}, m_Name{name}
        {
            float rangeLow = offset - scale;
            float rangeHigh = offset + scale;
            m_RangeLow = rangeLow < rangeHigh ? rangeLow : rangeHigh;
            m_RangeHigh = rangeLow < rangeHigh ? rangeHigh : rangeLow;
        }
        virtual ~EasingAnimation() {};

        void Run(float time, float& speed)
        {
            float t = std::clamp(time, 0.0f, 1.0f);
            float value = m_Offset + (m_Invert ? EasingFunction(1.0f - t) : EasingFunction(t));
            speed = std::clamp(value, m_RangeLow, m_RangeHigh);
        };
        std::string const& GetName() const { return m_Name; }

    private:
        virtual float EasingFunction(float time) = 0;

    protected:
        float m_Scale;

    private:
        bool m_Invert;
        float m_Offset, m_RangeLow, m_RangeHigh;
        std::string m_Name;
    };

} // namespace LucreApp
