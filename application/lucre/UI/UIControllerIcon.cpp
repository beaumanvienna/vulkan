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

#include "UI/UIControllerIcon.h"
#include "transform/matrix.h"
#include "platform/input.h"

#include "lucre.h"

namespace LucreApp
{

    extern float duration;

    void UIControllerIcon::OnAttach()
    {
        // controller 1
        m_Controller1Detected = false;
        // controller icon
        m_ControllerSprite = Lucre::m_Spritesheet->GetSprite(I_CONTROLLER);
        m_Renderer = Engine::m_Engine->GetRenderer();

        glm::vec2 finalOutOfScreenPosition(-1150.0f, -445.0f);
        glm::vec2 finalScreenPosition(-687.0f, -445.0f);

        // controller icon: move left to center
        m_Controller1MoveIn.AddTranslation(Translation(1.0f, finalOutOfScreenPosition, finalScreenPosition));
        m_Controller1MoveIn.AddRotation(Rotation(      1.0f,    0.0f,   0.0f));                                           // idle
        m_Controller1MoveIn.AddScaling(Scaling(        0.9f,    0.6f,   0.6f));
        m_Controller1MoveIn.AddScaling(Scaling(        0.1f,    0.6f,   1.0f));

        // controller icon: wiggle
        const float rotationTiming = 0.75f;
        m_Controller1MoveIn.AddTranslation(Translation(1.0f * rotationTiming, finalScreenPosition, finalScreenPosition)); // idle
        m_Controller1MoveIn.AddRotation(Rotation(      0.1f * rotationTiming,   0.0f,   0.2f));
        m_Controller1MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,   0.2f,  -0.2f));
        m_Controller1MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,  -0.2f,   0.2f));
        m_Controller1MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,   0.2f,  -0.1f));
        m_Controller1MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,  -0.1f,   0.1f));
        m_Controller1MoveIn.AddRotation(Rotation(      0.1f * rotationTiming,   0.1f,   0.0f));
        m_Controller1MoveIn.AddScaling(Scaling(        1.0f * rotationTiming,   1.0f,   1.0f));                           // idle

        // controller icon: idle
        m_Controller1MoveOut.AddTranslation(Translation(0.5f, finalScreenPosition, finalScreenPosition));                 // idle
        m_Controller1MoveOut.AddRotation(Rotation(      0.4f,   0.0f,   0.0f));                                           // idle
        m_Controller1MoveOut.AddScaling(Scaling(        0.5f,   1.0f,   1.0f));                                           // idle

        // controller icon: move center to left
        m_Controller1MoveOut.AddTranslation(Translation(1.0f, finalScreenPosition, finalOutOfScreenPosition));
        m_Controller1MoveOut.AddRotation(Rotation(      0.1f, -0.05f,    0.0f));
        m_Controller1MoveOut.AddRotation(Rotation(      0.9f,   0.0f,    0.0f));                                          // idle
        m_Controller1MoveOut.AddScaling(Scaling(        0.1f,   1.0f,    0.6f));
        m_Controller1MoveOut.AddScaling(Scaling(        0.9f,   0.6f,    0.6f));

        // controller 2
        m_Controller2Detected = false;

        finalScreenPosition = glm::vec2{-387.0f, -445.0f};

        // controller icon: move left to center
        m_Controller2MoveIn.AddTranslation(Translation(1.0f, finalOutOfScreenPosition, finalScreenPosition));
        m_Controller2MoveIn.AddRotation(Rotation(      1.0f,    0.0f,   0.0f));                                           // idle
        m_Controller2MoveIn.AddScaling(Scaling(        0.9f,    0.6f,   0.6f));
        m_Controller2MoveIn.AddScaling(Scaling(        0.1f,    0.6f,   1.0f));

        // controller icon: wiggle
        m_Controller2MoveIn.AddTranslation(Translation(1.0f * rotationTiming, finalScreenPosition, finalScreenPosition)); // idle
        m_Controller2MoveIn.AddRotation(Rotation(      0.1f * rotationTiming,   0.0f,   0.2f));
        m_Controller2MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,   0.2f,  -0.2f));
        m_Controller2MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,  -0.2f,   0.2f));
        m_Controller2MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,   0.2f,  -0.1f));
        m_Controller2MoveIn.AddRotation(Rotation(      0.2f * rotationTiming,  -0.1f,   0.1f));
        m_Controller2MoveIn.AddRotation(Rotation(      0.1f * rotationTiming,   0.1f,   0.0f));
        m_Controller2MoveIn.AddScaling(Scaling(        1.0f * rotationTiming,   1.0f,   1.0f));                           // idle

        // controller icon: idle
        m_Controller2MoveOut.AddTranslation(Translation(0.5f, finalScreenPosition, finalScreenPosition));                 // idle
        m_Controller2MoveOut.AddRotation(Rotation(      0.4f,   0.0f,   0.0f));                                           // idle
        m_Controller2MoveOut.AddScaling(Scaling(        0.5f,   1.0f,   1.0f));                                           // idle

        // controller icon: move center to left
        m_Controller2MoveOut.AddTranslation(Translation(1.0f, finalScreenPosition, finalOutOfScreenPosition));
        m_Controller2MoveOut.AddRotation(Rotation(      0.1f, -0.05f,    0.0f));
        m_Controller2MoveOut.AddRotation(Rotation(      0.9f,   0.0f,    0.0f));                                          // idle
        m_Controller2MoveOut.AddScaling(Scaling(        0.1f,   1.0f,    0.6f));
        m_Controller2MoveOut.AddScaling(Scaling(        0.9f,   0.6f,    0.6f));
    }

    void UIControllerIcon::OnDetach()  {}

    void UIControllerIcon::OnUpdate()
    {
        uint controllerCount = Input::GetControllerCount();
        // controller 1

        // controller is conneted
        if (!m_Controller1Detected && controllerCount)
        {
            m_Controller1Detected = true;
            m_Controller1MoveIn.Start();
        }
        if (m_Controller1Detected)
        {
            auto animationMatrix = m_Controller1MoveIn.GetTransformation();

            // transformed position
            glm::mat4 position = animationMatrix * m_ControllerSprite->GetScaleMatrix();
            m_Renderer->Draw(m_ControllerSprite, position, -0.07f);
        }

        // controller disconnected
        if (m_Controller1Detected && !controllerCount)
        {
            m_Controller1Detected = false;
            m_Controller1MoveOut.Start();
        }
        if (!m_Controller1Detected && m_Controller1MoveOut.IsRunning())
        {
            auto animationMatrix = m_Controller1MoveOut.GetTransformation();

            // transformed position
            glm::mat4 position = animationMatrix * m_ControllerSprite->GetScaleMatrix();
            m_Renderer->Draw(m_ControllerSprite, position, -0.07f);
        }

        // controller 2

        // controller is conneted
        if (!m_Controller2Detected && controllerCount > 1)
        {
            m_Controller2Detected = true;
            m_Controller2MoveIn.Start();
        }
        if (m_Controller2Detected)
        {
            auto animationMatrix = m_Controller2MoveIn.GetTransformation();

            // transformed position
            glm::mat4 position = animationMatrix * m_ControllerSprite->GetScaleMatrix();
            m_Renderer->Draw(m_ControllerSprite, position, -0.07f);
        }

        // controller disconnected
        if (m_Controller2Detected && controllerCount < 2)
        {
            m_Controller2Detected = false;
            m_Controller2MoveOut.Start();
        }
        if (!m_Controller2Detected && m_Controller2MoveOut.IsRunning())
        {
            auto animationMatrix = m_Controller2MoveOut.GetTransformation();

            // transformed position
            glm::mat4 position = animationMatrix * m_ControllerSprite->GetScaleMatrix();
            m_Renderer->Draw(m_ControllerSprite, position, -0.07f);
        }
    }

    bool UIControllerIcon::IsMovingIn()
    {
        bool isMovingIn = m_Controller1MoveIn.IsRunning();
        isMovingIn |= m_Controller2MoveIn.IsRunning();
        return isMovingIn;
    }

    void UIControllerIcon::OnEvent(Event& event)  {}
}
