/* Engine Copyright (c) 2023 Engine Development Team
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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <algorithm>

#include "engine.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    class Momentum
    {
    public:
        void Set(float absoluteMaxValue, float attackTime, float decayTime, float falloff)
        {
            m_AbsoluteMaxValue = absoluteMaxValue;
            m_AttackTime = attackTime;
            m_DecayTime = decayTime;
            m_Falloff = falloff;

            m_SpeedNormalized = 0.f;           // range [-1.f, 1.f]
            m_DecayTimeNormalizedActual = 1.f; // range [ 0.f, 1.f] during decay
            m_FalloffAtOne = std::exp(-m_Falloff);
        }

        float Get(float inputValue, Timestep const& timestep)
        {
            if (inputValue == 0.f)
            {
                const float coastOffset = 0.01f;
                float speedNormalized = std::exp(-m_Falloff * m_DecayTimeNormalizedActual) + coastOffset;

                if (m_SpeedNormalized > 0.f)
                {
                    m_SpeedNormalized = speedNormalized;
                }
                else
                {
                    m_SpeedNormalized = -speedNormalized;
                }
                if (std::abs(m_SpeedNormalized) <= (coastOffset + m_FalloffAtOne))
                    m_SpeedNormalized = 0.f;
                m_DecayTimeNormalizedActual =
                    std::min(1.f, m_DecayTimeNormalizedActual + timestep / (m_DecayTime)); // [0.f, 1.f]
            }
            else
            {
                m_SpeedNormalized = std::clamp(m_SpeedNormalized + inputValue * timestep * m_AttackTime, -1.f, 1.f);
                m_DecayTimeNormalizedActual = 1.f - std::abs(m_SpeedNormalized);
            }

            float speed = m_SpeedNormalized * m_AbsoluteMaxValue;
            return speed;
        }

    private:
        float m_AbsoluteMaxValue;
        float m_AttackTime;
        float m_DecayTime;
        float m_Falloff;
        float m_FalloffAtOne;

        float m_SpeedNormalized;
        float m_DecayTimeNormalizedActual;
    };
} // namespace GfxRenderEngine
