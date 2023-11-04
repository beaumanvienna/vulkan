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


#include "characterAnimation.h"
#include "scene/components.h"
#include "auxiliary/debug.h"
#include "platform/input.h"

namespace LucreApp
{

    CharacterAnimation::CharacterAnimation(entt::registry& registry, entt::entity gameObject, SkeletalAnimations& animations)
        : m_Registry{registry}, m_GameObject{gameObject}, m_Animations{animations}, m_DirToTheRight{false},
          m_Transform{glm::mat4(1.0f)}, m_PreviousPositionX{0.0f}, m_MotionState{MotionState::IDLE}
    {
        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);
    }

    void CharacterAnimation::Start()
    {
        m_Animations.SetRepeatAll(false);
        m_MotionState = MotionState::IDLE;
        m_Animations.Start(MotionState::IDLE);
        m_Animations.SetRepeat(true);
    }

    void CharacterAnimation::Stop()
    {
        m_MotionState = MotionState::IDLE;
        m_Animations.Stop();
    }

    void CharacterAnimation::OnUpdate(const Timestep& timestep)
    {
        m_GamepadInputController->GetTransform(m_Transform);

        auto& translation = m_Transform.GetTranslation();
        float speed = (translation.x - m_PreviousPositionX) / timestep;
        m_PreviousPositionX = translation.x;

        auto view = m_Registry.view<TransformComponent>();
        auto& characterTransform  = view.get<TransformComponent>(m_GameObject);

        if (Input::IsControllerButtonPressed(Controller::FIRST_CONTROLLER, Controller::Controller::BUTTON_A))
        {
            if (m_MotionState != MotionState::JUMPING)
            {
                m_MotionState = MotionState::JUMPING;
                m_Animations.Start(MotionState::JUMPING);
                return;
            }
        }

        if (m_MotionState == MotionState::JUMPING)
        {
            if (!m_Animations.IsRunning())
            {
                m_MotionState = MotionState::IDLE;
                m_Animations.Start(MotionState::IDLE);
            }
        }

        if (std::abs(speed) > 0.1f)
        {
            const float rotationY = (speed < 0.0f) ? -1.57078826f: 1.57078826f;
            characterTransform.SetRotationY(rotationY);

            switch (m_MotionState)
            {
                case MotionState::IDLE:
                {
                    m_MotionState = MotionState::START_WALK;
                    m_Animations.Start(MotionState::START_WALK);
                    m_DirToTheRight = (speed > 0.0f);
                    break;
                }
                case MotionState::JUMPING:
                {
                    break;
                }
                case MotionState::START_WALK:
                {
                    if (!m_Animations.IsRunning())
                    {
                        m_MotionState = MotionState::WALK;
                        m_Animations.Start(MotionState::WALK);
                        m_Animations.SetRepeat(true);
                        const float startWalkTranslation = 1.0f;
                        const float translationX = m_DirToTheRight ? startWalkTranslation : -startWalkTranslation;
                        characterTransform.AddTranslation({translationX, 0.0f, 0.0f});
                    }
                    break;
                }
                case MotionState::STOP_WALK:
                {
                    break;
                }
                case MotionState::WALK:
                {
                    const float walkTranslation = 0.4f * timestep;
                    const float translationX = m_DirToTheRight ? walkTranslation : -walkTranslation;
                    characterTransform.AddTranslation({translationX, 0.0f, 0.0f});
                    break;
                }
                default:
                {
                    LOG_APP_ERROR("CharacterAnimation state machine error (1)");
                    break;
                }
            }
        }
        else
        {
            switch (m_MotionState)
            {
                case MotionState::IDLE:
                {
                    break;
                }
                case MotionState::JUMPING:
                {
                    break;
                }
                case MotionState::START_WALK:
                {
                    m_MotionState = MotionState::STOP_WALK;
                    m_Animations.Start(MotionState::STOP_WALK);
                    break;
                }
                case MotionState::STOP_WALK:
                {
                    if (!m_Animations.IsRunning())
                    {
                        m_MotionState = MotionState::IDLE;
                        m_Animations.Start(MotionState::IDLE);
                        characterTransform.SetRotationY(0.0f);
                        const float stopWalkTranslation = 0.5f;
                        const float translationX = m_DirToTheRight ? stopWalkTranslation : -stopWalkTranslation;
                        characterTransform.AddTranslation({translationX, 0.0f, 0.0f});
                    }
                    break;
                }
                case MotionState::WALK:
                {
                    m_MotionState = MotionState::STOP_WALK;
                    m_Animations.Start(MotionState::STOP_WALK);
                    break;
                }
                default:
                {
                    LOG_APP_ERROR("CharacterAnimation state machine error (2)");
                    break;
                }
            }
        }
    }

    void CharacterAnimation::OnEvent(Event& event)
    {
    }

    void CharacterAnimation::OnResize()
    {
    }
}
