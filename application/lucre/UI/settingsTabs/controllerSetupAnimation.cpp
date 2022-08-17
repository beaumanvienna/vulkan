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

#include "core.h"
#include "lucre.h"
#include "platform/input.h"
#include "transform/matrix.h"
#include "resources/resources.h"
#include "UI/settingsTabs/controllerSetupAnimation.h"

namespace LucreApp
{

    void ControllerSetupAnimation::OnAttach() 
    {
        m_Renderer = Engine::m_Engine->GetRenderer();
        m_SetupController = Lucre::m_Spritesheet->GetSprite(I_PS3_CONTROLLER);
        m_SpritesheetPointers.AddSpritesheetRow("/images/images/I_CONTROLLER_SETUP.png", IDB_CONTROLLER_SETUP, "PNG", 19 /* frames */, 1.0f /* scaleX) */, 1.0f /* scaleY) */);
    }

    void ControllerSetupAnimation::OnDetach()  {}

    void ControllerSetupAnimation::SetActiveController(int activeController)
    {
        if (activeController == Controller::FIRST_CONTROLLER)
        {
            m_Translation = glm::vec3(0.0f, -300.0f, 0.0f);
        }
        else if (activeController == Controller::SECOND_CONTROLLER)
        {
            m_Translation = glm::vec3(0.0f,  200.0f, 0.0f);
        }
        else
        {
            LOG_APP_CRITICAL("Only two controllers in setup screen supported");
        }
    }

    void ControllerSetupAnimation::OnUpdate(const Timestep& timestep)
    {
        glm::mat4 translationMatrix = Translate(m_Translation);

        // controller picture
        {
            // transformed position
            glm::mat4 position = translationMatrix * m_SetupController->GetMat4();

            m_Renderer->Draw(m_SetupController, position);
        }

        // arrows
        {
            Sprite* sprite = m_SpritesheetPointers.GetSprite(m_Frame);

            // transformed position
            glm::mat4 position = translationMatrix * sprite->GetMat4();

            m_Renderer->Draw(sprite, position);
        }
    }

    void ControllerSetupAnimation::OnEvent(Event& event)  {}
}
