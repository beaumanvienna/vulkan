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

#include "platform/input.h"
#include "scene/components.h"

#include "gamepadInputController.h"

namespace LucreApp
{
    GamepadInputController::GamepadInputController(const GamepadInputControllerSpec& spec)
        : m_Deadzone{spec.m_Deadzone}, m_Sensitivity{spec.m_Sensitivity}, m_MoveSpeed{spec.m_MoveSpeed},
          m_LookSpeed{spec.m_LookSpeed}
    {
        m_Momentum.Set(/*absoluteMaxValue*/ 5.f, /*attackTime*/ 1.f, /*decayTime*/ 1.f, /*falloff*/ 8);
    }

    void GamepadInputController::GetTransform(TransformComponent& transform, bool scale)
    {
        // left
        glm::vec2 controllerAxisInputLeft = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::LEFT_STICK);

        if (std::abs(controllerAxisInputLeft.x) > m_Deadzone)
        {
            transform.SetTranslationX(transform.GetTranslation().x + controllerAxisInputLeft.x * m_Sensitivity);
        }

        if (std::abs(controllerAxisInputLeft.y) > m_Deadzone)
        {
            transform.SetTranslationY(transform.GetTranslation().y - controllerAxisInputLeft.y * m_Sensitivity);
        }

        // right
        if (scale)
        {
            glm::vec2 controllerAxisInputRight =
                Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);

            if (std::abs(controllerAxisInputRight.y) > m_Deadzone)
            {
                transform.SetScaleX(transform.GetScale().x - controllerAxisInputRight.y * m_Sensitivity);
                transform.SetScaleX(std::clamp(transform.GetScale().x, 0.025f, 0.1f));
                transform.SetScaleY(transform.GetScale().x);
                transform.SetScaleZ(transform.GetScale().x);
            }
        }

        if (Input::IsControllerButtonPressed(Controller::FIRST_CONTROLLER, Controller::BUTTON_DPAD_UP))
        {
            transform.SetTranslationZ(transform.GetTranslation().z + m_Sensitivity);
        }
        else if (Input::IsControllerButtonPressed(Controller::FIRST_CONTROLLER, Controller::BUTTON_DPAD_DOWN))
        {
            transform.SetTranslationZ(transform.GetTranslation().z - m_Sensitivity);
        }
    }

    void GamepadInputController::MoveInPlaneXZ(const Timestep& timestep, TransformComponent& transform)
    {

        glm::vec2 controllerAxisInputRight =
            Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);

        // rotate
        if (std::abs(controllerAxisInputRight.x) > m_Deadzone)
        {
            float rotate = -controllerAxisInputRight.x;
            transform.AddRotationY(m_LookSpeed * (float)timestep * rotate);
            transform.SetRotationY(glm::mod(transform.GetRotation().y, glm::two_pi<float>()));
        }

        // translate
        {
            float translate = 0.0f;
            if (std::abs(controllerAxisInputRight.y) > m_Deadzone)
            {
                translate = controllerAxisInputRight.y;
            }

            float yaw = transform.GetRotation().y;
            const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};
            const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};

            glm::vec3 moveDir{0.0f};
            moveDir -= forwardDir * m_Momentum.Get(translate, timestep);

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
            {
                transform.AddTranslation(m_MoveSpeed * timestep * moveDir);
            }
        }
    }
} // namespace LucreApp
