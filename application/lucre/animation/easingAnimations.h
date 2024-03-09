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

#include <vector>

#include "engine.h"
#include "easingAnimation.h"

using Duration = std::chrono::duration<float, std::chrono::seconds::period>;

namespace LucreApp
{

    template <int dimensions> class EasingAnimations
    {
    public:
        // multi-dimensional element type
        class AnimationsXY
        {
        public:
            AnimationsXY(std::initializer_list<std::shared_ptr<EasingAnimation>> arguments)
            {
                CORE_ASSERT(arguments.size() == dimensions,
                            "EasingAnimations::AnimationsXY: wrong number of arguments to initialze animation sequence");
                int animationIndex = 0;
                for (auto& animation : arguments)
                {
                    m_AnimationsXY[animationIndex] = animation;
                    ++animationIndex;
                }
            }
            void Print()
            {
                for (int index = 0; index < dimensions; ++index)
                {
                    std::cout << m_AnimationsXY[index]->GetName() << "\n";
                }
            }
            void Run(float normalizedTime)
            {
                for (int index = 0; index < dimensions; ++index)
                {
                    m_AnimationsXY[index]->Run(normalizedTime);
                }
            }

        private:
            std::shared_ptr<EasingAnimation> m_AnimationsXY[dimensions];
        };

    public:
        EasingAnimations()
            : m_IsRunning{false}, m_Duration{0.0ms}, m_TimePerAnimation{0.0ms}, m_Loop{false}, m_PrintNotRunning{true}
        {
        }

        void PushAnimation(AnimationsXY& animations) { m_Animations.push_back(animations); }
        void Print()
        {
            for (auto& animationXY : m_Animations)
            {
                animationXY.Print();
            }
        }
        void SetLoop(bool loop) { m_Loop = loop; }
        void SetDuration(Duration duration) { m_Duration = duration; }
        void Start()
        {
            if (!m_Animations.size())
            {
                LOG_APP_ERROR("EasingAnimations: no animations found");
                return;
            }

            m_IsRunning = true;
            m_PrintNotRunning = true;
            m_StartTime = Engine::m_Engine->GetTime().time_since_epoch();
            {
                m_TimePerAnimation = m_Duration / m_Animations.size();
                m_StartTimes.clear();
                m_StartTimes.resize(m_Animations.size());
                for (size_t index = 0; index < m_Animations.size(); ++index)
                {
                    m_StartTimes[index] = m_StartTime + index * m_TimePerAnimation;
                }
            }
        }
        void Stop() { m_IsRunning = false; }
        void Run()
        {
            if (!m_IsRunning)
            {
                if (m_PrintNotRunning)
                {
                    m_PrintNotRunning = false;
                    LOG_APP_INFO("EasingAnimations not running; start it or set loop flag");
                }
                return;
            }
            Duration currentTime = Engine::m_Engine->GetTime().time_since_epoch();
            Duration timeElapsedTotal = currentTime - m_StartTime;
            if (timeElapsedTotal > m_Duration)
            {
                if (m_Loop)
                {
                    m_StartTime = Engine::m_Engine->GetTime().time_since_epoch();
                    for (uint index = 0; index < m_Animations.size(); ++index)
                    {
                        m_StartTimes[index] = m_StartTime + index * m_TimePerAnimation;
                    }
                }
                else
                {
                    return;
                }
            }
            float currentAnimationFloat = timeElapsedTotal / m_TimePerAnimation;
            int currentAnimation = static_cast<int>(std::floor(currentAnimationFloat));
            Duration timeElapsedCurrentAnimation = currentTime - m_StartTimes[currentAnimation];
            float normalizedTime = timeElapsedCurrentAnimation / m_TimePerAnimation;
            LOG_APP_INFO("currentAnimation: {0}", currentAnimation);
            m_Animations[currentAnimation].Run(normalizedTime);
        }

    private:
        bool m_IsRunning;
        Duration m_StartTime;
        Duration m_Duration;
        Duration m_TimePerAnimation;
        bool m_Loop;
        bool m_PrintNotRunning;
        std::vector<Duration> m_StartTimes;

        std::vector<AnimationsXY> m_Animations;
    };

} // namespace LucreApp
