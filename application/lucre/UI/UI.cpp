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

#include <cmath>

#include "lucre.h"
#include "UI/UI.h"
#include "UI/common.h"
#include "UI/settingsScreen.h"
#include "renderer/texture.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "events/controllerEvent.h"
#include "gui/Common/Input/inputState.h"
#include "gui/Common/Render/drawBuffer.h"
#include "auxiliary/instrumentation.h"
#include "resources/resources.h"
#include "platform/input.h"

namespace LucreApp
{
    std::unique_ptr<SCREEN_ScreenManager> UI::m_ScreenManager = nullptr;
    std::shared_ptr<Texture> UI::m_FontAtlasTexture;
    std::shared_ptr<Texture> UI::m_SpritesheetTexture;
    std::shared_ptr<Common> UI::m_Common;

    void UI::OnAttach()
    {
        auto renderer = Engine::m_Engine->GetRenderer();
        m_Spritesheet = Lucre::m_Spritesheet;
        m_ScreenManager = std::make_unique<SCREEN_ScreenManager>(renderer, m_Spritesheet);

        m_FontAtlasTexture = ResourceSystem::GetTextureFromMemory("/images/atlas/fontAtlas.png", IDB_FONTS_RETRO, "PNG");
        m_SpritesheetTexture = m_Spritesheet->GetTexture();
        m_Common = std::make_unique<Common>();

        m_MainScreen = new MainScreen(m_Spritesheet);
        m_MainScreen->OnAttach();
        m_ScreenManager->push(m_MainScreen);

        m_UIStarIcon = new UIStarIcon(false, "UI star icon");
        Engine::m_Engine->PushOverlay(m_UIStarIcon);

        m_UIControllerAnimation = new ControllerSetupAnimation("controller animation");
        Engine::m_Engine->PushOverlay(m_UIControllerAnimation);
    }

    void UI::OnDetach() 
    {
        m_MainScreen->OnDetach();
        m_ScreenManager.reset();
        m_FontAtlasTexture.reset();
        m_SpritesheetTexture.reset();

        delete m_MainScreen;
        delete m_UIStarIcon;
    }

    void UI::OnUpdate(const Timestep& timestep)
    {
        PROFILE_FUNCTION();
        m_ScreenManager->update();
        m_ScreenManager->render();

        if (SettingsScreen::m_IsCreditsScreen)
        {
            m_UIStarIcon->Start();
        }
        else
        {
            m_UIStarIcon->Stop();
        }

        if ((SettingsScreen::m_IsCintrollerSetupScreen) && Input::ConfigurationRunning())
        {
            m_UIControllerAnimation->SetActiveController(Input::GetConfigurationActiveController());
            m_UIControllerAnimation->SetFrame(Input::GetConfigurationStep());
            m_UIControllerAnimation->OnUpdate(timestep);
        }

        m_UIStarIcon->OnUpdate(timestep);
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
                return false;
            }
        );

        dispatcher.Dispatch<ControllerButtonReleasedEvent>([this](ControllerButtonReleasedEvent event) 
            {
                Key(KEY_UP, event.GetControllerButton(), DEVICE_ID_PAD_0);
                return false;
            }
        );

        dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent event) 
            {
                bool clicked = false;
                if (event.GetButton() == MouseButtonEvent::Left) 
                {
                    // output context coordinates adjusted for orthographic projection
                    float contextPositionX = event.GetX();
                    float contextPositionY = event.GetY();

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

    void UI::OnResize()
    {
        m_Common->OnResize();
        m_ScreenManager->resized();
    }

    void UI::Health(const float health)
    {
        //draw health bar
        Sprite whiteSprite = m_Spritesheet->GetSprite(I_WHITE);
        float x1 = 32.0f, y1 = 52.0f, x2 = 132.0f, y2 = 90.0f;
        Color colorForeground = 0xFF442a28;
        Color colorBackground = 0xC0000000;

        Draw()->DrawImageStretch(whiteSprite, x1-2.0f, y1-2.0f, x2+2.0f, y2+2.0f, colorBackground);
        Draw()->DrawImageStretch(whiteSprite, x1, y1, x1 + (x2-x1)*health/100.0f, y2, colorForeground);
    }
}
