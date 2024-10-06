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

#pragma once

#include <iostream>

#include "engine.h"
#include "UI/infoMessage.h"
#include "gui/Common/UI/UIscreen.h"
#include "gui/Common/UI/viewGroup.h"

namespace LucreApp
{

    class MainScreen : public SCREEN_UIDialogScreen
    {

    public:
        MainScreen(SpriteSheet& spritesheet);
        virtual ~MainScreen();

        MainScreen(MainScreen const&) = delete;
        MainScreen& operator=(MainScreen const&) = delete;

        bool key(SCREEN_KeyInput const& key) override;
        void OnAttach();
        void OnDetach();
        void update() override;
        void onFinish(DialogResult result) override;
        std::string tag() const override { return "main screen"; }

    protected:
        void CreateViews() override;

        SCREEN_UI::EventReturn SettingsClick(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene1Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene2Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene3Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene4Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene5Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene6Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene7Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn Scene8Click(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OffClick(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OffHold(SCREEN_UI::EventParams& e);

    private:
        enum toolTipID
        {
            MAIN_SETTINGS,
            MAIN_OFF,
            MAX_TOOLTIP_IDs
        };

    private:
        enum SceneButtons
        {
            SCENE_BUTTON_1 = 0,
            SCENE_BUTTON_2,
            SCENE_BUTTON_3,
            SCENE_BUTTON_4,
            SCENE_BUTTON_5,
            SCENE_BUTTON_6,
            SCENE_BUTTON_7,
            SCENE_BUTTON_8,
            NUM_SCENE_BUTTONS
        };

        SCREEN_UI::Choice* m_OffButton{nullptr};
        SCREEN_UI::Choice* m_SettingsButton{nullptr};
        SCREEN_UI::Choice* m_SceneButtons[SceneButtons::NUM_SCENE_BUTTONS] = {};
        InfoMessage* m_MainInfo{nullptr};

        bool m_SetFocus = true;
        bool m_ToolTipsShown[MAX_TOOLTIP_IDs] = {false};

        SpriteSheet& m_Spritesheet;
        SpriteSheet m_SpritesheetSettings;
        SpriteSheet m_SpritesheetOff;
        SpriteSheet m_SpritesheetSceneButtons[SceneButtons::NUM_SCENE_BUTTONS];
    };
} // namespace LucreApp
