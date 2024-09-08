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

#include <memory>

#include "engine.h"
#include "lucre.h"
#include "renderer/model.h"
#include "gamepadInputController.h"
#include "renderer/skeletalAnimation/skeleton.h"
#include "renderer/skeletalAnimation/skeletalAnimations.h"
#include "auxiliary/timestep.h"

namespace LucreApp
{
    class CharacterAnimation
    {

    public:
        CharacterAnimation(Registry& registry, entt::entity gameObject, SkeletalAnimations& animations);
        ~CharacterAnimation() {}

        void Start();
        void Stop();
        void OnUpdate(const Timestep& timestep);

    private:
        static constexpr float WALK_SPEED = 1.0f;
        static constexpr float TIME_TO_GET_TO_WALK_SPEED = 1.0f;
        static constexpr float WAIT_START_WALK = 0.8f;
        static constexpr int FRAMES_PER_ROTATION = 7;

        enum MotionState
        {
            NO_FOUND = -1,
            IDLE,
            JUMPING,
            START_WALK,
            STOP_WALK,
            WALK,
            JOGGING,
            JUMPING_DOWN,
            FALLING_IDLE,
            PUNCHING,
            RUNNING,
            NUMBER_OF_MOTION_STATES
        };

        std::vector<std::string> m_AnimationsNames = {
            "Idle",    "Jumping",      "StartWalk",    "StopWalk", "Walk",
            "Jogging", "Jumping Down", "Falling Idle", "Punching", "Running",
        };

    private:
        void SetState(MotionState state);
        void InitiateRotation(float rotateDirRight, float rotateDirLeft, int framesToRotate);
        void MoveAtSpeed(const Timestep& timestep, TransformComponent& characterTransform);
        void RotateY(TransformComponent& characterTransform, float deltaRotation);
        void PerformRotation(TransformComponent& characterTransform);
        void EliminateRoundingErrorsRotationY(TransformComponent& characterTransform);
        float ToDegree(float rotation);

    private:
        Registry& m_Registry;
        std::unique_ptr<GamepadInputController> m_GamepadInputController;
        entt::entity m_GameObject;
        SkeletalAnimations& m_Animations;
        TransformComponent m_Transform;

        float m_DurationStartWalk;
        float m_DurationStopWalk;

        float m_Speed;
        float m_PreviousPositionX;
        bool m_DirToTheRight;
        int m_FramesPerRotation;
        int m_FramesToRotate;
        float m_RotationPerFrame;
        float m_WaitStartWalk;
        float m_WalkSpeedScaled;

        MotionState m_MotionState;
        std::vector<int> m_AnimationIndices;
    };
} // namespace LucreApp
