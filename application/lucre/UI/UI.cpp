/* Engine Copyright (c) 2021 Engine Development Team 
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

#include <cmath>

#include "lucre.h"
#include "UI/UI.h"
#include "renderer/texture.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "events/controllerEvent.h"
#include "gui/Common/Input/inputState.h"
#include "auxiliary/instrumentation.h"
#include "resources/resources.h"
#include "platform/input.h"

namespace LucreApp
{
    std::unique_ptr<SCREEN_ScreenManager> UI::m_ScreenManager = nullptr;
    std::shared_ptr<Texture> UI::m_FontAtlasTexture;
    std::shared_ptr<Texture> UI::m_SpritesheetTexture;

    void UI::OnAttach()
    {
        auto renderer = Engine::m_Engine->GetRenderer();
        auto spritesheet = Lucre::m_Spritesheet;
        m_ScreenManager = std::make_unique<SCREEN_ScreenManager>(renderer, spritesheet);

        m_FontAtlasTexture = ResourceSystem::GetTextureFromMemory("/images/atlas/fontAtlas.png", IDB_FONTS_RETRO, "PNG");
        m_SpritesheetTexture = spritesheet->GetTexture();

        m_MainScreen = new MainScreen(spritesheet);
        m_MainScreen->OnAttach();
        m_ScreenManager->push(m_MainScreen);

    }

    void UI::OnDetach() 
    {
        m_MainScreen->OnDetach();
        m_ScreenManager.reset();
        m_FontAtlasTexture.reset();
        m_SpritesheetTexture.reset();
    }

    void UI::OnUpdate()
    {
        PROFILE_FUNCTION();
        m_ScreenManager->update();
        m_ScreenManager->render();
    }

    void UI::OnEvent(Event& event)
    {
        if (!m_ScreenManager)
        {
            return;
        }

        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<ControllerButtonPressedEvent>([this](ControllerButtonPressedEvent event) 
            {
                Key(KEY_DOWN, event.GetControllerButton(), DEVICE_ID_PAD_0);
                return true;
            }
        );

        dispatcher.Dispatch<ControllerButtonReleasedEvent>([this](ControllerButtonReleasedEvent event) 
            {
                Key(KEY_UP, event.GetControllerButton(), DEVICE_ID_PAD_0);
                return true;
            }
        );

        dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent event) 
            {
                bool clicked = false;
                if (event.GetButton() == MouseButtonEvent::Left) 
                {
                    // output context coordinates adjusted for orthographic projection
                    float windowScale = Engine::m_Engine->GetWindowScale();
                    float contextPositionX = event.GetX()/windowScale;
                    float contextPositionY = event.GetY()/windowScale;

                    int flags = TOUCH_DOWN | TOUCH_MOUSE;
                    float x = contextPositionX;
                    float y = contextPositionY;
                    int deviceID = 0;
                    clicked = Touch(flags, x, y, deviceID);
                }
                return clicked;
            }
        );

        dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent event) 
            {
                bool clicked = false;
                if (event.GetMouseButton() == MouseButtonEvent::Left) 
                {
                    int flags = TOUCH_UP | TOUCH_MOUSE;
                    float x = 0.0f;
                    float y = 0.0f;
                    int deviceID = 0;
                    return Touch(flags, x, y, deviceID);
                }
                return clicked;
            }
        );

        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent event) 
            {
                int flags = TOUCH_WHEEL;
                float x = 0.0f;
                float y = event.GetY();
                int deviceID = 0;
                return Touch(flags, x, y, deviceID);
            }
        );

        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent event) 
            { 
                Key(KEY_DOWN, event.GetKeyCode(), DEVICE_ID_KEYBOARD);
                return false;
            }
        );

        dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent event) 
            { 
                Key(KEY_UP, event.GetKeyCode(), DEVICE_ID_KEYBOARD);
                return false;
            }
        );
    }

    bool UI::Touch(int flags, float x, float y, int deviceID)
    {
        bool clicked = false;
        {
            SCREEN_TouchInput touch;
            touch.x = x;
            touch.y = y;
            touch.flags = flags;
            touch.id = deviceID;
            touch.timestamp = Engine::m_Engine->GetTimeDouble();
            clicked = m_ScreenManager->touch(touch);
        }
        return clicked;
    }

    void UI::Key(int keyFlag, int keyCode, int deviceID)
    {
        {
            SCREEN_KeyInput key;
            key.flags = keyFlag;
            key.keyCode = keyCode;
            key.deviceId = deviceID;
            m_ScreenManager->key(key);
        }
    }

    void UI::Axis()
    {
        if (!Input::GetControllerCount()) return;
        glm::vec2 controllerAxisInput = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);

        SCREEN_AxisInput axis;
        axis.flags = 0;
        axis.deviceId = DEVICE_ID_PAD_0;
        if (std::abs(controllerAxisInput.x) > std::abs(controllerAxisInput.y))
        {
            axis.axisId = Controller::RIGHT_STICK_HORIZONTAL;
            axis.value  = controllerAxisInput.x;
        }
        else
        {
            axis.axisId = Controller::RIGHT_STICK_VERTICAL;
            axis.value  = controllerAxisInput.y;
        }
        m_ScreenManager->axis(axis);
    }
}
