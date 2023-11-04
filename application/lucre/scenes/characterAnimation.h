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
#include "scene/entity.h"

namespace LucreApp
{
    class CharacterAnimation
    {

    public:

        CharacterAnimation(entt::registry& registry, entt::entity gameObject, SkeletalAnimations& animations);
        ~CharacterAnimation() {}

        void Start();
        void Stop();
        void OnUpdate(const Timestep& timestep);
        void OnEvent(Event& event);
        void OnResize();

    private:

        enum MotionState
        {
          IDLE = 0,
          JUMPING,
          START_WALK,
          STOP_WALK,
          WALK
        };

        entt::registry& m_Registry;
        std::unique_ptr<GamepadInputController> m_GamepadInputController;
        entt::entity m_GameObject;
        SkeletalAnimations& m_Animations;
        TransformComponent m_Transform;
        float m_PreviousPositionX;
        bool m_DirToTheRight;

        MotionState m_MotionState;

    };
}
