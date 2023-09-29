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
        
        void Set(float minValue, float maxValue, float attack, float decay)
        {
            m_MinValue = minValue;
            m_MaxValue = maxValue;
            m_Attack   = attack;
            m_Decay    = decay;
        }

        float Get(float inputValue, Timestep const& timestep)
        {
            if (inputValue == 0.f) 
            {
                if (m_Momentum > 0.f)
                {
                    m_Momentum = std::max(0.f, m_Momentum - m_Decay * timestep);
                }
                else
                {
                    m_Momentum = std::min(0.f, m_Momentum + m_Decay * timestep);
                }
            }
            else
            {
                m_Momentum = std::clamp(m_Momentum + inputValue * timestep * m_Attack, m_MinValue, m_MaxValue);
            }
            return m_Momentum;
        }

    private: 

        float m_MinValue;
        float m_MaxValue;
        float m_Attack;
        float m_Decay;
    
        float m_Momentum;
    };
}
