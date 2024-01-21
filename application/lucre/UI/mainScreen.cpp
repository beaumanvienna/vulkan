/* Engine Copyright (c) 2021 Engine Development Team
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

#include <stdlib.h>

#include "core.h"
#include "lucre.h"
#include "appEvent.h"
#include "UI/common.h"
#include "UI/offDialog.h"
#include "UI/mainScreen.h"
#include "UI/settingsScreen.h"
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
        m_SpritesheetSettings.AddSpritesheetRow(m_Spritesheet->GetSprite(I_GEAR_R), 4 /* frames */);
        m_SpritesheetOff.AddSpritesheetRow(m_Spritesheet->GetSprite(I_OFF_R), 4 /* frames */);
        m_SpritesheetScene1Button.AddSpritesheetRow(m_Spritesheet->GetSprite(I_SCENE_No_1_R), 4 /* frames */);
        m_SpritesheetScene2Button.AddSpritesheetRow(m_Spritesheet->GetSprite(I_SCENE_No_2_R), 4 /* frames */);
        m_SpritesheetScene3Button.AddSpritesheetRow(m_Spritesheet->GetSprite(I_SCENE_No_3_R), 4 /* frames */);
        m_SpritesheetScene4Button.AddSpritesheetRow(m_Spritesheet->GetSprite(I_SCENE_No_4_R), 4 /* frames */);
    }

    void MainScreen::OnDetach()
    {
    }

    bool MainScreen::key(const SCREEN_KeyInput &key)
    {
        if (!m_OffButton) return false;
        if (!m_OffButton->HasFocus())
        {
            if (key.flags & KEY_DOWN)
            {
                if ( (key.deviceId == DEVICE_ID_KEYBOARD && key.keyCode == KeyCode::ENGINE_KEY_ESCAPE) ||
                     (key.deviceId == DEVICE_ID_PAD_0    && key.keyCode == Controller::BUTTON_GUIDE) )
                {
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
        auto ma = GetI18NCategory("Main");

        root_ = new SCREEN_UI::AnchorLayout(new SCREEN_UI::LayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT));
        root_->SetTag("root_");

        SCREEN_UI::LinearLayout* verticalLayout = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_VERTICAL, new SCREEN_UI::LayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT));
        verticalLayout->SetTag("verticalLayout");
        root_->Add(verticalLayout);

        m_MainInfo = new InfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new SCREEN_UI::AnchorLayoutParams(UI::m_Common->m_AvailableWidth - UI::m_Common->m_MarginLeftRight * 3 - 2 * UI::m_Common->m_IconWidth - UI::m_Common->m_IconSpacer,
                                        SCREEN_UI::WRAP_CONTENT, UI::m_Common->m_MarginLeftRight, 0.0f, SCREEN_UI::NONE, SCREEN_UI::NONE));
        //root_->Add(m_MainInfo);

        verticalLayout->Add(new SCREEN_UI::Spacer(UI::m_Common->m_MarginLeftRight));

        // top line
        SCREEN_UI::LinearLayout* topline = new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT));
        topline->SetTag("topLine");
        verticalLayout->Add(topline);

        topline->Add(new SCREEN_UI::Spacer(UI::m_Common->m_MarginLeftRight,0.0f));
        float horizontalSpacerTopline = UI::m_Common->m_AvailableWidth - 6 * UI::m_Common->m_IconWidth - 4 * UI::m_Common->m_IconSpacer - 2 * UI::m_Common->m_MarginLeftRight;

        Sprite2D icon;
        Sprite2D icon_active;
        Sprite2D icon_depressed;

        // scene 1 button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetScene1Button.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetScene1Button.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetScene1Button.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);

            m_Scene1Button = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_Scene1Button = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        m_Scene1Button->OnClick.Handle(this, &MainScreen::Scene1Click);
        topline->Add(m_Scene1Button);
        topline->Add(new SCREEN_UI::Spacer(UI::m_Common->m_IconSpacer,0.0f));

        // scene 2 button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetScene2Button.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetScene2Button.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetScene2Button.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);

            m_Scene2Button = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_Scene2Button = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        m_Scene2Button->OnClick.Handle(this, &MainScreen::Scene2Click);
        topline->Add(m_Scene2Button);
        topline->Add(new SCREEN_UI::Spacer(UI::m_Common->m_IconSpacer,0.0f));

        // scene 3 button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetScene3Button.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetScene3Button.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetScene3Button.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);

            m_Scene3Button = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_Scene2Button = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        m_Scene3Button->OnClick.Handle(this, &MainScreen::Scene3Click);
        topline->Add(m_Scene3Button);
        topline->Add(new SCREEN_UI::Spacer(UI::m_Common->m_IconSpacer,0.0f));

        // scene 4 button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetScene4Button.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetScene4Button.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetScene4Button.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);

            m_Scene4Button = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_Scene2Button = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        m_Scene4Button->OnClick.Handle(this, &MainScreen::Scene4Click);
        topline->Add(m_Scene4Button);
        topline->Add(new SCREEN_UI::Spacer(horizontalSpacerTopline,0.0f));

        // settings button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);

            m_SettingsButton = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_SettingsButton = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
        }

        m_SettingsButton->OnClick.Handle(this, &MainScreen::SettingsClick);
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
        topline->Add(new SCREEN_UI::Spacer(UI::m_Common->m_IconSpacer,0.0f));
        if (m_SetFocus)
        {
            root_->SetDefaultFocusView(m_SettingsButton);
            m_SetFocus = false;
        }
        // off button
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetOff.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetOff.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::m_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetOff.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::m_Common->m_IconScaleRetro);
            m_OffButton = new SCREEN_UI::Choice(icon, icon_active, icon_depressed, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()),true);
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_OFF));
            icon.SetScale(UI::m_Common->m_IconScale);
            m_OffButton = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()), true);
        }
        m_OffButton->OnClick.Handle(this, &MainScreen::OffClick);
        m_OffButton->OnHold.Handle(this, &MainScreen::OffHold);
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

    SCREEN_UI::EventReturn MainScreen::SettingsClick(SCREEN_UI::EventParams &e)
    {
        SettingsScreen* settingsScreen = new SettingsScreen();
        settingsScreen->OnAttach();
        SceneChangedEvent event(GameState::State::SETTINGS);
        Lucre::m_Application->OnAppEvent(event);
        UI::m_ScreenManager->push(settingsScreen);

        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene1Click(SCREEN_UI::EventParams &e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::MAIN)
        {
            SceneChangedEvent event(GameState::State::MAIN);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene2Click(SCREEN_UI::EventParams &e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::BEACH)
        {
            SceneChangedEvent event(GameState::State::BEACH);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene3Click(SCREEN_UI::EventParams &e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::NIGHT)
        {
            SceneChangedEvent event(GameState::State::NIGHT);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene4Click(SCREEN_UI::EventParams &e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::DESSERT)
        {
            SceneChangedEvent event(GameState::State::DESSERT);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::OffClick(SCREEN_UI::EventParams &e)
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

    SCREEN_UI::EventReturn MainScreen::OffHold(SCREEN_UI::EventParams &e)
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
