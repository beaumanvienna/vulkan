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
    SCREEN_ScreenManager* UI::g_ScreenManager{nullptr};
    Common* UI::g_Common{nullptr};

    UI::UI(const std::string& name) : Layer(name) {}

    UI::~UI()
    {
        g_ScreenManager = nullptr;
        g_Common = nullptr;
    }

    void UI::OnAttach()
    {
        auto renderer = Engine::m_Engine->GetRenderer();
        m_Spritesheet = Lucre::m_Spritesheet;
        m_ScreenManager = std::make_unique<SCREEN_ScreenManager>(renderer, m_Spritesheet);
        g_ScreenManager = m_ScreenManager.get();

        m_FontAtlasTexture = ResourceSystem::GetTextureFromMemory("/images/atlas/fontAtlas.png", IDB_FONTS_RETRO, "PNG");
        m_SpritesheetTexture = m_Spritesheet->GetTexture();
        m_Common = std::make_unique<Common>();
        g_Common = m_Common.get();

        m_MainScreen = new MainScreen(*m_Spritesheet); // deleted by screen manager
        m_MainScreen->OnAttach();
        m_ScreenManager->push(m_MainScreen);

        m_UIStarIcon = std::make_unique<UIStarIcon>("UI star icon");
        Engine::m_Engine->PushOverlay(m_UIStarIcon.get());

        m_UIControllerAnimation = std::make_unique<ControllerSetupAnimation>("controller animation");
        Engine::m_Engine->PushOverlay(m_UIControllerAnimation.get());
    }

    void UI::OnDetach() { m_MainScreen->OnDetach(); }

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

        if (Lucre::m_Application->DebugWindowIsRunning())
        {
            return;
        }

        if (!Lucre::m_Application->InGameGuiIsRunning())
        {
            return;
        }

        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<ControllerButtonPressedEvent>(
            [this](ControllerButtonPressedEvent l_Event)
            {
                Key(KEY_DOWN, l_Event.GetControllerButton(), DEVICE_ID_PAD_0);
                return false;
            });

        dispatcher.Dispatch<ControllerButtonReleasedEvent>(
            [this](ControllerButtonReleasedEvent l_Event)
            {
                Key(KEY_UP, l_Event.GetControllerButton(), DEVICE_ID_PAD_0);
                return false;
            });

        dispatcher.Dispatch<MouseButtonPressedEvent>(
            [this](MouseButtonPressedEvent l_Event)
            {
                bool clicked = false;
                if (l_Event.GetButton() == MouseButtonEvent::Left)
                {
                    // output context coordinates adjusted for orthographic projection
                    float contextPositionX = l_Event.GetX();
                    float contextPositionY = l_Event.GetY();

                    int flags = TOUCH_DOWN | TOUCH_MOUSE;
                    float x = contextPositionX;
                    float y = contextPositionY;
                    int deviceID = 0;
                    clicked = Touch(flags, x, y, deviceID);
                }
                return clicked;
            });

        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            [this](MouseButtonReleasedEvent l_Event)
            {
                bool clicked = false;
                if (l_Event.GetMouseButton() == MouseButtonEvent::Left)
                {
                    int flags = TOUCH_UP | TOUCH_MOUSE;
                    float x = 0.0f;
                    float y = 0.0f;
                    int deviceID = 0;
                    return Touch(flags, x, y, deviceID);
                }
                return clicked;
            });

        dispatcher.Dispatch<MouseScrolledEvent>(
            [this](MouseScrolledEvent l_Event)
            {
                int flags = TOUCH_WHEEL;
                float x = 0.0f;
                float y = l_Event.GetY();
                int deviceID = 0;
                return Touch(flags, x, y, deviceID);
            });

        dispatcher.Dispatch<KeyPressedEvent>(
            [this](KeyPressedEvent l_Event)
            {
                Key(KEY_DOWN, l_Event.GetKeyCode(), DEVICE_ID_KEYBOARD);
                return false;
            });

        dispatcher.Dispatch<KeyReleasedEvent>(
            [this](KeyReleasedEvent l_Event)
            {
                Key(KEY_UP, l_Event.GetKeyCode(), DEVICE_ID_KEYBOARD);
                return false;
            });
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
        if (!Input::GetControllerCount())
            return;
        glm::vec2 controllerAxisInput = Input::GetControllerStick(Controller::FIRST_CONTROLLER, Controller::RIGHT_STICK);

        SCREEN_AxisInput axis;
        axis.flags = 0;
        axis.deviceId = DEVICE_ID_PAD_0;
        if (std::abs(controllerAxisInput.x) > std::abs(controllerAxisInput.y))
        {
            axis.axisId = Controller::RIGHT_STICK_HORIZONTAL;
            axis.value = controllerAxisInput.x;
        }
        else
        {
            axis.axisId = Controller::RIGHT_STICK_VERTICAL;
            axis.value = controllerAxisInput.y;
        }
        m_ScreenManager->axis(axis);
    }

    void UI::OnResize()
    {
        m_Common->OnResize();
        m_UIStarIcon->OnResize();
        m_ScreenManager->resized();
    }

    void UI::Health(const float health)
    {
        // draw health bar
        Sprite whiteSprite = m_Spritesheet->GetSprite(I_WHITE);
        float x1 = 32.0f * UI::g_Common->m_ScaleAll;
        float y1 = 8.0f * UI::g_Common->m_ScaleAll;
        float x2 = 132.0f * UI::g_Common->m_ScaleAll;
        float y2 = 50.0f * UI::g_Common->m_ScaleAll;
        Color colorForeground = 0xFF442a28;
        Color colorBackground = 0xC0000000;

        Draw()->DrawImageStretch(whiteSprite, x1 - 2.0f, y1 - 2.0f, x2 + 2.0f, y2 + 2.0f, colorBackground);
        Draw()->DrawImageStretch(whiteSprite, x1, y1, x1 + (x2 - x1) * health / 100.0f, y2, colorForeground);
    }
} // namespace LucreApp
