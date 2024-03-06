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
#include "sprite/spritesheet.h"
#include "gui/Common/UI/viewGroup.h"
#include "platform/SDL/controllerConfiguration.h"

namespace LucreApp
{

    class ControllerSetup : public SCREEN_UI::LinearLayout
    {

    public:
        ControllerSetup(SpriteSheet* spritesheet, SCREEN_UI::LayoutParams* layoutParams = 0);
        ~ControllerSetup();

        virtual bool Key(const SCREEN_KeyInput& input) override;
        virtual void Update() override;
        bool IsRunning() const { return Controller::m_ControllerConfiguration.IsRunning(); }

        SCREEN_UI::Event OnMappingSuccessful;

    private:
        void Refresh();
        SCREEN_UI::EventReturn OnStartSetup1(SCREEN_UI::EventParams& e);
        SCREEN_UI::EventReturn OnStartSetup2(SCREEN_UI::EventParams& e);
        void SetControllerConfText();

    private:
        SpriteSheet* m_Spritesheet;
        SpriteSheet m_SpritesheetSettings;

        SCREEN_UI::TextView* m_TextSetup1;
        SCREEN_UI::TextView* m_TextSetup1b;

        SCREEN_UI::TextView* m_TextSetup2;
        SCREEN_UI::TextView* m_TextSetup2b;

        bool m_ConfigurationIsRunningCtrl1;
        bool m_ConfigurationIsRunningCtrl2;
    };
} // namespace LucreApp
