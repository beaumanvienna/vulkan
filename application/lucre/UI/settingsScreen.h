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

#pragma once

#include "engine.h"
#include "gui/Common/UI/UIscreen.h"
#include "UI/settingsTabs/controllerSetup.h"
#include "UI/settingsTabs/credits.h"
#include "UI/infoMessage.h"

namespace LucreApp
{

    inline constexpr float TAB_SCALE = 1.5f;

    class SettingsScreen : public SCREEN_UIDialogScreen
    {

    public:
        SettingsScreen();
        virtual ~SettingsScreen();
        bool key(const SCREEN_KeyInput& key) override;
        void OnAttach();
        void update() override;
        void onFinish(DialogResult result) override;
        std::string tag() const override { return "settings screen"; }
        static bool m_IsCreditsScreen;
        static bool m_IsCintrollerSetupScreen;

    protected:
        void CreateViews() override;

    private:
        enum
        {
            GENERAL_SCREEN,
            CONTROLLER_SETUP_SCREEN,
            CREDITS_SCREEN
        };

    private:
        SCREEN_UI::EventReturn OnFullscreenToggle(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OnThemeChanged(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OnAudioDevice(SCREEN_UI::EventParams& e);
        ControllerSetup* m_ControllerSetup;
        Credits* m_Credits;
        void SetSoundCallback();

    private:
        InfoMessage* m_SettingsInfo;
        SCREEN_UI::TabHolder* m_TabHolder = nullptr;
        int m_LastTab;

        SpriteSheet* m_Spritesheet;
        SpriteSheet m_SpritesheetTab;
        SpriteSheet m_SpritesheetBack;

        int m_GlobalVolume;
        bool m_EnableFullscreen;
        bool m_GlobalVolumeEnabled;
        std::string m_AudioDevice;
    };
} // namespace LucreApp
