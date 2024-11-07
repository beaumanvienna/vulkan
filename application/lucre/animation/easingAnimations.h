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

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Duration = std::chrono::duration<float, std::chrono::milliseconds::period>;

namespace LucreApp
{

    template <int dimensions> class EasingAnimations
    {
    public:
        // multi-dimensional element type
        class AnimationsXY
        {
        public:
            AnimationsXY(Duration duration, std::initializer_list<std::shared_ptr<EasingAnimation>> arguments)
                : m_Duration{duration}
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
            void Run(float normalizedTime, float speedXY[dimensions])
            {
                for (int index = 0; index < dimensions; ++index)
                {
                    m_AnimationsXY[index]->Run(normalizedTime, speedXY[index]);
                }
            }
            Duration GetDuration() const { return m_Duration; }

        private:
            Duration m_Duration;
            std::shared_ptr<EasingAnimation> m_AnimationsXY[dimensions];
        };

    public:
        EasingAnimations() : m_IsRunning{false}, m_Duration{0.0ms}, m_Loop{false}, m_PrintNotRunning{true} {}
        void PushAnimation(AnimationsXY& animationXY)
        {
            m_Duration += animationXY.GetDuration();
            m_Animations.push_back(animationXY);
        }
        void Print()
        {
            for (auto& animationXY : m_Animations)
            {
                animationXY.Print();
            }
        }
        void SetLoop(bool loop) { m_Loop = loop; }
        void Start()
        {
            if (!m_Animations.size())
            {
                LOG_APP_ERROR("EasingAnimations: no animations found");
                return;
            }

            m_IsRunning = true;
            m_PrintNotRunning = true;
            m_StartTime = Engine::m_Engine->GetTime();
            {
                m_StartTimes.clear();
                m_StartTimes.resize(m_Animations.size());
                m_StartTimes[0] = 0.0ms;
                for (size_t index = 0; index < m_Animations.size() - 1; ++index)
                {
                    m_StartTimes[index + 1] = m_StartTimes[index] + m_Animations[index].GetDuration();
                }
            }
        }
        void Stop() { m_IsRunning = false; }
        bool IsRunning() const { return m_IsRunning; }
        bool Run(float speedXY[dimensions])
        {
            for (uint index = 0; index < dimensions; ++index)
            {
                speedXY[index] = 0.0f;
            }
            if (!m_IsRunning)
            {
                if (m_PrintNotRunning)
                {
                    m_PrintNotRunning = false;
                    LOG_APP_INFO("EasingAnimations not running; start it or set loop flag");
                }
                return m_IsRunning;
            }
            Duration timeElapsedTotal;
            {
                TimePoint currentTime = Engine::m_Engine->GetTime();
                timeElapsedTotal = currentTime - m_StartTime;
                if (timeElapsedTotal > m_Duration)
                {
                    if (m_Loop)
                    {
                        m_StartTime = Engine::m_Engine->GetTime();
                    }
                    else
                    {
                        m_IsRunning = false;
                        return m_IsRunning;
                    }
                }
            }
            {
                for (size_t currentAnimation = 0; currentAnimation < m_Animations.size(); ++currentAnimation)
                {
                    Duration currentAnimationDuration = m_Animations[currentAnimation].GetDuration();
                    if (timeElapsedTotal < m_StartTimes[currentAnimation] + currentAnimationDuration)
                    {
                        Duration currentAnimationElapsed = timeElapsedTotal - m_StartTimes[currentAnimation];
                        float normalizedTime = currentAnimationElapsed / currentAnimationDuration;
                        m_Animations[currentAnimation].Run(normalizedTime, speedXY);
                        break;
                    }
                }
            }
            return m_IsRunning;
        }

    private:
        bool m_IsRunning;
        TimePoint m_StartTime;
        Duration m_Duration;
        bool m_Loop;
        bool m_PrintNotRunning;
        std::vector<Duration> m_StartTimes; // accumulated sequence durations 0s, 3s, 8s, ...

        std::vector<AnimationsXY> m_Animations;
    };

} // namespace LucreApp
