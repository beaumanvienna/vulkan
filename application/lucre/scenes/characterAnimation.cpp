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
#include "platform/input.h"

namespace LucreApp
{

    CharacterAnimation::CharacterAnimation(Registry& registry, entt::entity gameObject, SkeletalAnimations& animations)
        : m_Registry{registry}, m_GameObject{gameObject}, m_Animations{animations}, m_DirToTheRight{false},
          m_Transform{glm::mat4(1.0f)}, m_PreviousPositionX{0.0f}, m_MotionState{MotionState::IDLE},
          m_FramesPerRotation{FRAMES_PER_ROTATION}, m_FramesToRotate{0}, m_Speed{0.0f}, m_WaitStartWalk{0.0f},
          m_WalkSpeedScaled{0.0f}
    {
        m_DurationStartWalk = animations.GetDuration("StartWalk");
        m_DurationStopWalk = animations.GetDuration("StopWalk");

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);
    }

    void CharacterAnimation::Start()
    {
        {
            m_AnimationIndices.resize(m_AnimationsNames.size());
            for (uint index = 0; index < NUMBER_OF_MOTION_STATES; ++index)
            {
                std::string name = m_AnimationsNames[index];
                m_AnimationIndices[index] = m_Animations.GetIndex(name);
                LOG_APP_INFO("name: {0}, found: {1}", name, (m_AnimationIndices[index] != MotionState::NO_FOUND));
            }
        }
        m_Animations.SetRepeatAll(false);
        m_MotionState = MotionState::IDLE;
        m_Animations.Start(m_AnimationIndices[MotionState::IDLE]);
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
        float controllerInputSpeed = (translation.x - m_PreviousPositionX) / timestep;
        m_PreviousPositionX = translation.x;

        auto view = m_Registry.view<TransformComponent>();
        auto& characterTransform = view.get<TransformComponent>(m_GameObject);
        // float characterScale = characterTransform.GetScale().x;
        m_WalkSpeedScaled = WALK_SPEED; // character scale does not work as models can also be scaled on the vertex level :-(

        if (Input::IsControllerButtonPressed(Controller::FIRST_CONTROLLER, Controller::Controller::BUTTON_A))
        {
            if (m_MotionState != MotionState::JUMPING)
            {
                if ((m_MotionState == MotionState::WALK) || (m_MotionState == MotionState::START_WALK) ||
                    (m_MotionState == MotionState::STOP_WALK))
                {
                    InitiateRotation(-TransformComponent::DEGREES_90, TransformComponent::DEGREES_90,
                                     m_FramesPerRotation +
                                         1 // plus one because it starts to rotate later (normally it starts to rotate here)
                    );
                }

                SetState(MotionState::JUMPING);
                return;
            }
        }

        if (m_MotionState == MotionState::JUMPING)
        {
            if (m_Animations.WillExpire(timestep))
            {
                SetState(MotionState::IDLE);
            }
        }

        if (std::abs(controllerInputSpeed) > 0.1f)
        {
            switch (m_MotionState)
            {
                case MotionState::IDLE:
                {
                    // set direction for movement
                    m_DirToTheRight = (controllerInputSpeed > 0.0f);

                    EliminateRoundingErrorsRotationY(characterTransform);

                    // initiate rotation of the character in the direction of travel
                    InitiateRotation(TransformComponent::DEGREES_90, -TransformComponent::DEGREES_90, m_FramesPerRotation);
                    RotateY(characterTransform, m_RotationPerFrame);

                    SetState(MotionState::START_WALK);
                    m_WaitStartWalk = WAIT_START_WALK;
                    m_Speed = 0.0f;
                    break;
                }
                case MotionState::JUMPING:
                {
                    // slow down
                    m_Speed = std::max(0.0f, m_Speed - (m_WalkSpeedScaled * 2.0f * timestep / TIME_TO_GET_TO_WALK_SPEED));

                    // move
                    MoveAtSpeed(timestep, characterTransform);
                    break;
                }
                case MotionState::START_WALK:
                {
                    PerformRotation(characterTransform); // turn towards walking direction

                    // pick up speed
                    if (m_WaitStartWalk > 0.0f)
                    {
                        m_WaitStartWalk -= timestep;
                    }
                    else
                    {
                        m_Speed = std::max(m_WalkSpeedScaled, m_Speed + (m_WalkSpeedScaled * timestep * timestep * timestep /
                                                                         TIME_TO_GET_TO_WALK_SPEED));
                    }

                    // move
                    MoveAtSpeed(timestep, characterTransform);

                    if (m_Animations.WillExpire(timestep))
                    {
                        SetState(MotionState::WALK);
                        m_Speed = m_WalkSpeedScaled;
                        m_Animations.SetRepeat(true);
                    }
                    break;
                }
                case MotionState::STOP_WALK:
                {
                    break;
                }
                case MotionState::WALK:
                {
                    // move
                    MoveAtSpeed(timestep, characterTransform);
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
                    // perform rotation
                    PerformRotation(characterTransform); // turn towards camera
                    break;
                }
                case MotionState::JUMPING:
                {
                    // slow down
                    m_Speed = std::max(0.0f, m_Speed - (m_WalkSpeedScaled * 0.5f * timestep / TIME_TO_GET_TO_WALK_SPEED));
                    // move
                    MoveAtSpeed(timestep, characterTransform);
                    break;
                }
                case MotionState::START_WALK:
                {
                    SetState(MotionState::STOP_WALK);
                    break;
                }
                case MotionState::STOP_WALK:
                {
                    // slow down
                    m_Speed = std::max(0.0f, m_Speed - (m_WalkSpeedScaled * 0.5f * timestep / TIME_TO_GET_TO_WALK_SPEED));

                    // move
                    MoveAtSpeed(timestep, characterTransform);

                    if (m_Animations.WillExpire(timestep))
                    {
                        // initiate rotation of the character in the direction of travel
                        InitiateRotation(-TransformComponent::DEGREES_90, TransformComponent::DEGREES_90,
                                         m_FramesPerRotation);
                        RotateY(characterTransform, m_RotationPerFrame);

                        SetState(MotionState::IDLE);
                    }
                    break;
                }
                case MotionState::WALK:
                {
                    EliminateRoundingErrorsRotationY(characterTransform);
                    SetState(MotionState::STOP_WALK);
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

    void CharacterAnimation::MoveAtSpeed(const Timestep& timestep, TransformComponent& characterTransform)
    {
        const float moveSpeed = m_Speed * timestep;
        const float speedX = m_DirToTheRight ? moveSpeed : -moveSpeed;
        characterTransform.AddTranslationX(speedX);
    }

    void CharacterAnimation::RotateY(TransformComponent& characterTransform, float deltaRotation)
    {
        characterTransform.AddRotationY(deltaRotation);
    }

    void CharacterAnimation::SetState(MotionState state)
    {
        m_MotionState = state;
        m_Animations.Start(m_AnimationIndices[state]);
    }

    void CharacterAnimation::PerformRotation(TransformComponent& characterTransform)
    {
        if ((m_FramesToRotate - 1) > 0)
        {
            --m_FramesToRotate;
            RotateY(characterTransform, m_RotationPerFrame);
        }
    }

    void CharacterAnimation::InitiateRotation(float rotateDirRight, float rotateDirLeft, int framesToRotate)
    {
        float rotationY = m_DirToTheRight ? rotateDirRight : rotateDirLeft;
        m_FramesToRotate = framesToRotate; // plus one because it starts to rotate later (normally it starts to rotate here)
        m_RotationPerFrame = rotationY / m_FramesPerRotation;
    }

    float CharacterAnimation::ToDegree(float rotation) { return rotation * 180.0f / TransformComponent::DEGREES_180; }

    void CharacterAnimation::EliminateRoundingErrorsRotationY(TransformComponent& characterTransform)
    {

        float rotationY = 0.0f;

        switch (m_MotionState)
        {
            case MotionState::IDLE:
            {
                rotationY = TransformComponent::DEGREES_0;
                break;
            }
            case MotionState::JUMPING:
            {
                rotationY = TransformComponent::DEGREES_0;
                break;
            }
            case MotionState::START_WALK:
            {
                break;
            }
            case MotionState::STOP_WALK:
            {
                break;
            }
            case MotionState::WALK:
            {
                rotationY = m_DirToTheRight ? TransformComponent::DEGREES_90 : -TransformComponent::DEGREES_90;
                break;
            }
            default:
            {
                LOG_APP_ERROR("CharacterAnimation state machine error (3)");
                break;
            }
        }
        characterTransform.SetRotationY(rotationY);
    }
} // namespace LucreApp
