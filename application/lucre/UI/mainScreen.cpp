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

#include <stdlib.h>

#include "core.h"
#include "lucre.h"
#include "UI/offDialog.h"
#include "UI/mainScreen.h"
#include "platform/keyCodes.h"
#include "gui/Common/Data/Text/i18n.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/UI/root.h"
#include "auxiliary/instrumentation.h"
#include "sprite/spritesheet.h"

namespace LucreApp
{
    void MainScreen::OnAttach()
    {
        m_SpritesheetSettings.AddSpritesheetRow(m_Spritesheet->GetSprite(/*I_GEAR_R*/I_DISK_LOAD_R), 4 /* frames */);
        m_SpritesheetOff.AddSpritesheetRow(m_Spritesheet->GetSprite(I_OFF_R), 4 /* frames */);
    }

    void MainScreen::OnDetach()
    {
    }

    bool MainScreen::key(const SCREEN_KeyInput &key)
    {
        if (!m_OffButton->HasFocus())
        {
            if (key.flags & KEY_DOWN)
            {
                if ( (key.deviceId == DEVICE_ID_KEYBOARD && key.keyCode == KeyCode::ENGINE_KEY_ESCAPE) ||
                     (key.deviceId == DEVICE_ID_PAD_0    && key.keyCode == Controller::BUTTON_GUIDE) )
                {
                    //GfxRenderEngine::SCREEN_UI::SetFocusedView(m_OffButton);
                    return true;
                }
            }
        }
        if (m_OffButton->HasFocus())
        {
            if (key.flags & KEY_DOWN)
            {
                if ( (key.deviceId == DEVICE_ID_KEYBOARD && key.keyCode == ENGINE_KEY_ESCAPE) ||
                     (key.deviceId == DEVICE_ID_PAD_0    && key.keyCode == Controller::BUTTON_GUIDE) )
                {
                    {
                        Engine::m_Engine->Shutdown();
                        return true;
                    }
                }
            }
        }
        return SCREEN_UIDialogScreen::key(key);
    }

    void MainScreen::CreateViews()
    {
        PROFILE_FUNCTION();
        auto ma = GetI18NCategory("Main");

        root_ = new SCREEN_UI::AnchorLayout(new SCREEN_UI::LayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT));
        root_->SetTag("root_");

        SCREEN_UI::LinearLayout *verticalLayout = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new SCREEN_UI::LayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT));
        verticalLayout->SetTag("verticalLayout");
        root_->Add(verticalLayout);

        float availableWidth = Engine::m_Engine->GetContextWidth();
        float availableHeight = Engine::m_Engine->GetContextHeight();
        float marginLeftRight = 128.0f;
        float marginUpDown = 128.0f;
        float iconWidth = 128.0f;
        float iconHeight = 128.0f;
        float iconSpacer = 10.0f;

        float verticalSpacer = 10.0f;

        m_MainInfo = new InfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new SCREEN_UI::AnchorLayoutParams(availableWidth - marginLeftRight * 3 - 2 * iconWidth - iconSpacer,
                                        SCREEN_UI::WRAP_CONTENT, marginLeftRight, 0.0f, SCREEN_UI::NONE, SCREEN_UI::NONE));
        //root_->Add(m_MainInfo);

        verticalLayout->Add(new SCREEN_UI::Spacer(marginUpDown));

        // top line
        SCREEN_UI::LinearLayout *topline = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT));
        topline->SetTag("topLine");
        verticalLayout->Add(topline);

        float horizontalSpacerTopline = availableWidth - marginLeftRight - 2 * iconWidth - iconSpacer;
        topline->Add(new SCREEN_UI::Spacer(horizontalSpacerTopline,0.0f));

        Sprite* icon;
        Sprite* icon_active;
        Sprite* icon_depressed;

        // settings button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_NOT_FOCUSED);
            icon_active = m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED);
            icon_depressed = m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED);

            m_SettingsButton = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(iconWidth, iconWidth));
        }
        else
        {
            icon = m_Spritesheet->GetSprite(I_GEAR);
            m_SettingsButton = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(iconWidth, iconHeight));
        }

        m_SettingsButton->OnClick.Handle(this, &MainScreen::settingsClick);
        m_SettingsButton->OnHighlight.Add([=](SCREEN_UI::EventParams &e)
        {
            if (!m_ToolTipsShown[MAIN_SETTINGS])
            {
                m_ToolTipsShown[MAIN_SETTINGS] = true;
                //m_MainInfo->Show(ma->T("Settings", "Settings"), e.v);
            }
            return SCREEN_UI::EVENT_CONTINUE;
        });
        topline->Add(m_SettingsButton);
        topline->Add(new SCREEN_UI::Spacer(iconSpacer,0.0f));
        root_->SetDefaultFocusView(m_SettingsButton);
        // off button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = m_SpritesheetOff.GetSprite(BUTTON_4_STATES_NOT_FOCUSED);
            icon_active = m_SpritesheetOff.GetSprite(BUTTON_4_STATES_FOCUSED);
            icon_depressed = m_SpritesheetOff.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED);
            m_OffButton = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(iconWidth, iconHeight),true);
        }
        else
        {
            icon = m_Spritesheet->GetSprite(I_OFF);
            m_OffButton = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(iconWidth, iconHeight), true);
        }
        m_OffButton->OnClick.Handle(this, &MainScreen::offClick);
        m_OffButton->OnHold.Handle(this, &MainScreen::offHold);
        m_OffButton->OnHighlight.Add([=](SCREEN_UI::EventParams &e)
        {
            if (!m_ToolTipsShown[MAIN_OFF])
            {
                m_ToolTipsShown[MAIN_OFF] = true;
                //m_MainInfo->Show(ma->T("Off", "Off: exit Lucre; keep this button pressed to switch the computer off"), e.v);
            }
            return SCREEN_UI::EVENT_CONTINUE;
        });
        topline->Add(m_OffButton);

        LOG_APP_INFO("UI: views for main screen created");
    }

    void MainScreen::onFinish(DialogResult result)
    {
    }

    void MainScreen::update()
    {
        SCREEN_UIScreen::update();
    }

    SCREEN_UI::EventReturn MainScreen::settingsClick(SCREEN_UI::EventParams &e)
    {
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::offClick(SCREEN_UI::EventParams &e)
    {
        auto ma = GetI18NCategory("System");
        auto offClick = new OffDialog(ma->T("Exit Lucre?"), OFFDIAG_QUIT);
        if (e.v)
        {
            offClick->SetPopupOrigin(e.v);
        }

        screenManager()->push(offClick);

        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::offHold(SCREEN_UI::EventParams &e)
    {
        auto ma = GetI18NCategory("System");
        auto offDiag = new OffDialog(ma->T("Switch off computer?"), OFFDIAG_SHUTDOWN);
        if (e.v)
        {
            offDiag->SetPopupOrigin(e.v);
        }

        screenManager()->push(offDiag);

        return SCREEN_UI::EVENT_DONE;
    }
}
