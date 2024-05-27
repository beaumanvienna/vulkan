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
#include "platform/keyCodes.h"
#include "scene/scene.h"
#include "auxiliary/timestep.h"
#include "momentum.h"

namespace LucreApp
{
    struct KeyboardInputControllerSpec
    {
        float m_MoveSpeed{1.5f};
        float m_LookSpeed{0.5f};
    };

    class KeyboardInputController
    {

    public:
        static constexpr GfxRenderEngine::KeyCode MOVE_LEFT = ENGINE_KEY_A;
        static constexpr GfxRenderEngine::KeyCode MOVE_RIGHT = ENGINE_KEY_D;
        static constexpr GfxRenderEngine::KeyCode MOVE_FORWARD = ENGINE_KEY_W;
        static constexpr GfxRenderEngine::KeyCode MOVE_BACKWARD = ENGINE_KEY_S;
        static constexpr GfxRenderEngine::KeyCode MOVE_UP = ENGINE_KEY_E;
        static constexpr GfxRenderEngine::KeyCode MOVE_DOWN = ENGINE_KEY_Q;
        static constexpr GfxRenderEngine::KeyCode LOOK_LEFT = ENGINE_KEY_LEFT;
        static constexpr GfxRenderEngine::KeyCode LOOK_RIGHT = ENGINE_KEY_RIGHT;
        static constexpr GfxRenderEngine::KeyCode LOOK_UP = ENGINE_KEY_UP;
        static constexpr GfxRenderEngine::KeyCode LOOK_DOWN = ENGINE_KEY_DOWN;

    public:
        KeyboardInputController(const KeyboardInputControllerSpec& spec);

        void MoveInPlaneXZ(const Timestep& timestep, TransformComponent& transform);

    private:
        float m_MoveSpeed;
        float m_LookSpeed;
        Momentum m_MomentumX;
        Momentum m_MomentumY;
        Momentum m_MomentumZ;
    };
} // namespace LucreApp
