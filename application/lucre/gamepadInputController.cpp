/* Engine Copyright (c) 2022 Engine Development Team
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include "gamepadInputController.h"
#include "platform/input.h"

namespace LucreApp
{
    GamepadInputController::GamepadInputController(const GamepadInputControllerSpec& spec)
        : m_Deadzone{spec.m_Deadzone}, m_Sensitivity{spec.m_Sensitivity}
    {}

    void GamepadInputController::GetTransform(TransformComponent& transform, bool scale)
    {
        // left
        glm::vec2 controllerAxisInputLeft = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::LEFT_STICK);

        if (std::abs(controllerAxisInputLeft.x) > m_Deadzone)
        {
            transform.m_Translation.x += controllerAxisInputLeft.x * m_Sensitivity;
        }

        if (std::abs(controllerAxisInputLeft.y) > m_Deadzone)
        {
            transform.m_Translation.y -= controllerAxisInputLeft.y * m_Sensitivity;
        }

        // right
        if (scale)
        {
            glm::vec2 controllerAxisInputRight = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);
    
            if (std::abs(controllerAxisInputRight.y) > m_Deadzone)
            {
                transform.m_Scale.x -= controllerAxisInputRight.y * m_Sensitivity;
                transform.m_Scale.x = std::clamp(transform.m_Scale.x, 0.1f, 10.0f);
                transform.m_Scale.y = transform.m_Scale.x;
                transform.m_Scale.z = transform.m_Scale.x;
            }
        }
    }
}
