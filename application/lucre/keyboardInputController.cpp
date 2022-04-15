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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include <limits>

#include "platform/input.h"

#include "keyboardInputController.h"

namespace LucreApp
{

    KeyboardInputController::KeyboardInputController(const KeyboardInputControllerSpec& spec)
        : m_MoveSpeed{spec.m_MoveSpeed}, m_LookSpeed{spec.m_LookSpeed}
    {}

    void KeyboardInputController::MoveInPlaneXZ(const Timestep& timestep, TransformComponent& transform)
    {

        glm::vec3 rotate{0};
        if (Input::IsKeyPressed(LOOK_RIGHT)) rotate.y -= 1.f;
        if (Input::IsKeyPressed(LOOK_LEFT))  rotate.y += 1.f;
        if (Input::IsKeyPressed(LOOK_UP))    rotate.x -= 1.f;
        if (Input::IsKeyPressed(LOOK_DOWN))  rotate.x += 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            transform.SetRotation(transform.GetRotation() + m_LookSpeed * (float)timestep * glm::normalize(rotate));
        }

        // limit pitch values between about +/- 85ish degrees
        transform.SetRotationX(glm::clamp(transform.GetRotation().x, -1.5f, 1.5f));
        transform.SetRotationY(glm::mod(transform.GetRotation().y, glm::two_pi<float>()));

        float yaw = transform.GetRotation().y;
        const glm::vec3 forwardDir{std::sin(yaw), 0.f, std::cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (Input::IsKeyPressed(MOVE_FORWARD))  moveDir -= forwardDir;
        if (Input::IsKeyPressed(MOVE_BACKWARD)) moveDir += forwardDir;
        if (Input::IsKeyPressed(MOVE_RIGHT))    moveDir += rightDir;
        if (Input::IsKeyPressed(MOVE_LEFT))     moveDir -= rightDir;
        if (Input::IsKeyPressed(MOVE_UP))       moveDir -= upDir;
        if (Input::IsKeyPressed(MOVE_DOWN))     moveDir += upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            transform.SetTranslation(transform.GetTranslation() + m_MoveSpeed * (float)timestep * glm::normalize(moveDir));
        }
    }
}
