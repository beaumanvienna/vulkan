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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "core.h"
#include "lucre.h"
#include "platform/input.h"
#include "transform/matrix.h"
#include "resources/resources.h"
#include "UI/settingsTabs/controllerSetupAnimation.h"

namespace LucreApp
{
    ControllerSetupAnimation::~ControllerSetupAnimation() {}

    void ControllerSetupAnimation::OnAttach()
    {
        m_Renderer = Engine::m_Engine->GetRenderer();
        m_SpritesheetPointers.AddSpritesheetRow(Lucre::m_Spritesheet->GetSprite(I_CONTROLLER_SETUP), 19 /* frames */, 2.0f,
                                                2.0f);
    }

    void ControllerSetupAnimation::OnDetach() {}

    void ControllerSetupAnimation::SetActiveController(int activeController)
    {
        glm::vec3 translation(0.0f);
        float windowWidthHalf = Engine::m_Engine->GetWindowWidth() / 2.0f;
        float windowHeight = Engine::m_Engine->GetWindowHeight();
        if (activeController == Controller::FIRST_CONTROLLER)
        {
            translation = glm::vec3(windowWidthHalf, windowHeight * 0.75f, 0.0f);
        }
        else if (activeController == Controller::SECOND_CONTROLLER)
        {
            translation = glm::vec3(windowWidthHalf, windowHeight * 0.3f, 0.0f);
        }
        else
        {
            LOG_APP_CRITICAL("Only two controllers in setup screen supported");
        }
        m_TranslationMatrix = Translate(translation);
    }

    void ControllerSetupAnimation::OnUpdate(const Timestep& timestep)
    {
        Sprite2D sprite = Sprite2D(m_SpritesheetPointers.GetSprite(m_Frame));

        // transformed position
        glm::mat4 position = m_TranslationMatrix * sprite.GetMat4();
        m_Renderer->DrawWithTransform(sprite, position);
    }

    void ControllerSetupAnimation::OnEvent(Event& event) {}
} // namespace LucreApp
