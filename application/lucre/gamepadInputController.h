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

#include <functional>

#include "engine.h"
#include "scene/scene.h"
#include "momentum.h"

namespace LucreApp
{

    struct GamepadInputControllerSpec
    {
        float m_Deadzone{0.05f};
        float m_Sensitivity{0.01f};
        float m_MoveSpeed{1.5f};
        float m_LookSpeed{0.5f};
    };

    class GamepadInputController
    {

    public:
        GamepadInputController(const GamepadInputControllerSpec& spec);
        ~GamepadInputController() {}

        void GetTransform(TransformComponent& transform, bool scale = false);
        void MoveInPlaneXZ(const Timestep& timestep, TransformComponent& transform);

    private:
        float m_Deadzone;
        float m_Sensitivity;
        float m_MoveSpeed;
        float m_LookSpeed;

        Momentum m_Momentum;
    };
} // namespace LucreApp
