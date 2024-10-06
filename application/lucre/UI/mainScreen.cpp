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

    MainScreen::MainScreen(SpriteSheet& spritesheet) : m_Spritesheet{spritesheet} {}

    MainScreen::~MainScreen() {}

    void MainScreen::OnAttach()
    {
        m_SpritesheetSettings.AddSpritesheetRow(m_Spritesheet.GetSprite(I_GEAR_R), 4 /* frames */);
        m_SpritesheetOff.AddSpritesheetRow(m_Spritesheet.GetSprite(I_OFF_R), 4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_1].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_1_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_2].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_2_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_3].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_3_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_4].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_4_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_5].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_5_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_6].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_6_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_7].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_7_R),
                                                                                  4 /* frames */);
        m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_8].AddSpritesheetRow(m_Spritesheet.GetSprite(I_SCENE_No_8_R),
                                                                                  4 /* frames */);
    }

    void MainScreen::OnDetach() {}

    bool MainScreen::key(SCREEN_KeyInput const& key)
    {
        if (!m_OffButton)
            return false;
        if (!m_OffButton->HasFocus())
        {
            if (key.flags & KEY_DOWN)
            {
                if ((key.deviceId == DEVICE_ID_KEYBOARD && key.keyCode == KeyCode::ENGINE_KEY_ESCAPE) ||
                    (key.deviceId == DEVICE_ID_PAD_0 && key.keyCode == Controller::BUTTON_GUIDE))
                {
                    return true;
                }
            }
        }
        if (m_OffButton->HasFocus())
        {
            if (key.flags & KEY_DOWN)
            {
                if ((key.deviceId == DEVICE_ID_KEYBOARD && key.keyCode == ENGINE_KEY_ESCAPE) ||
                    (key.deviceId == DEVICE_ID_PAD_0 && key.keyCode == Controller::BUTTON_GUIDE))
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

        SCREEN_UI::LinearLayout* verticalLayout = new SCREEN_UI::LinearLayout(
            SCREEN_UI::ORIENT_VERTICAL, new SCREEN_UI::LayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::FILL_PARENT));
        verticalLayout->SetTag("verticalLayout");
        root_->Add(verticalLayout);

        m_MainInfo = new InfoMessage(
            ALIGN_CENTER | FLAG_WRAP_TEXT,
            new SCREEN_UI::AnchorLayoutParams(UI::g_Common->m_AvailableWidth - UI::g_Common->m_MarginLeftRight * 3 -
                                                  2 * UI::g_Common->m_IconWidth - UI::g_Common->m_IconSpacer,
                                              SCREEN_UI::WRAP_CONTENT, UI::g_Common->m_MarginLeftRight, 0.0f,
                                              SCREEN_UI::NONE, SCREEN_UI::NONE));
        // root_->Add(m_MainInfo);

        verticalLayout->Add(new SCREEN_UI::Spacer(UI::g_Common->m_MarginLeftRight));

        // top line
        SCREEN_UI::LinearLayout* topline =
            new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL,
                                        new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT));
        topline->SetTag("topLine");
        verticalLayout->Add(topline);

        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_MarginLeftRight, 0.0f));

        auto createButton = [](SpriteSheet& buttonSpritesheet, SCREEN_UI::Choice*& button, int plainThemeSpriteID)
        {
            Sprite2D icon;
            Sprite2D icon_active;
            Sprite2D icon_depressed;

            if (CoreSettings::m_UITheme == THEME_RETRO)
            {
                icon = Sprite2D(buttonSpritesheet.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
                icon.SetScale(UI::g_Common->m_IconScaleRetro);
                icon_active = Sprite2D(buttonSpritesheet.GetSprite(BUTTON_4_STATES_FOCUSED));
                icon_active.SetScale(UI::g_Common->m_IconScaleRetro);
                icon_depressed = Sprite2D(buttonSpritesheet.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
                icon_depressed.SetScale(UI::g_Common->m_IconScaleRetro);

                button = new SCREEN_UI::Choice(icon, icon_active, icon_depressed,
                                               new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
            }
            else
            {
                icon = Sprite2D(Lucre::m_Spritesheet->GetSprite(plainThemeSpriteID));
                icon.SetScale(UI::g_Common->m_IconScale);
                button = new SCREEN_UI::Choice(icon, new SCREEN_UI::LayoutParams(icon.GetWidth(), icon.GetHeight()));
            }
        };

        // scene 1 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_1], m_SceneButtons[SCENE_BUTTON_1], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_1]->OnClick.Handle(this, &MainScreen::Scene1Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_1]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 2 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_2], m_SceneButtons[SCENE_BUTTON_2], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_2]->OnClick.Handle(this, &MainScreen::Scene2Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_2]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 3 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_3], m_SceneButtons[SCENE_BUTTON_3], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_3]->OnClick.Handle(this, &MainScreen::Scene3Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_3]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 4 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_4], m_SceneButtons[SCENE_BUTTON_4], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_4]->OnClick.Handle(this, &MainScreen::Scene4Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_4]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 5 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_5], m_SceneButtons[SCENE_BUTTON_5], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_5]->OnClick.Handle(this, &MainScreen::Scene5Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_5]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 6 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_6], m_SceneButtons[SCENE_BUTTON_6], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_6]->OnClick.Handle(this, &MainScreen::Scene6Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_6]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 7 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_7], m_SceneButtons[SCENE_BUTTON_7], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_7]->OnClick.Handle(this, &MainScreen::Scene7Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_7]);
        topline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));

        // scene 8 button
        createButton(m_SpritesheetSceneButtons[SceneButtons::SCENE_BUTTON_8], m_SceneButtons[SCENE_BUTTON_8], I_GEAR);
        m_SceneButtons[SCENE_BUTTON_8]->OnClick.Handle(this, &MainScreen::Scene8Click);
        topline->Add(m_SceneButtons[SCENE_BUTTON_8]);

        float verticalSpacerBottomline =
            UI::g_Common->m_AvailableHeight - 2 * UI::g_Common->m_IconHeight - 2 * UI::g_Common->m_MarginLeftRight;

        verticalLayout->Add(new SCREEN_UI::Spacer(verticalSpacerBottomline));

        // bottom line
        SCREEN_UI::LinearLayout* bottomline =
            new SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL,
                                        new SCREEN_UI::LinearLayoutParams(SCREEN_UI::FILL_PARENT, SCREEN_UI::WRAP_CONTENT));
        bottomline->SetTag("bottomLine");
        verticalLayout->Add(bottomline);

        bottomline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_MarginLeftRight, 0.0f));

        // settings button
        createButton(m_SpritesheetSettings, m_SettingsButton, I_GEAR);
        m_SettingsButton->OnClick.Handle(this, &MainScreen::SettingsClick);
        m_SettingsButton->OnHighlight.Add(
            [&](SCREEN_UI::EventParams& e)
            {
                if (!m_ToolTipsShown[MAIN_SETTINGS])
                {
                    m_ToolTipsShown[MAIN_SETTINGS] = true;
                }
                return SCREEN_UI::EVENT_CONTINUE;
            });
        bottomline->Add(m_SettingsButton);
        bottomline->Add(new SCREEN_UI::Spacer(UI::g_Common->m_IconSpacer, 0.0f));
        if (m_SetFocus)
        {
            root_->SetDefaultFocusView(m_SettingsButton);
            m_SetFocus = false;
        }
        // off button
        createButton(m_SpritesheetOff, m_OffButton, I_OFF);
        m_OffButton->OnClick.Handle(this, &MainScreen::OffClick);
        m_OffButton->OnHold.Handle(this, &MainScreen::OffHold);
        m_OffButton->OnHighlight.Add(
            [&](SCREEN_UI::EventParams& e)
            {
                if (!m_ToolTipsShown[MAIN_OFF])
                {
                    m_ToolTipsShown[MAIN_OFF] = true;
                    // m_MainInfo->Show(ma->T("Off", "Off: exit Lucre; keep this button pressed to switch the computer off"),
                    // e.v);
                }
                return SCREEN_UI::EVENT_CONTINUE;
            });
        bottomline->Add(m_OffButton);

        LOG_APP_INFO("UI: views for main screen created");
    }

    void MainScreen::onFinish(DialogResult result) {}

    void MainScreen::update() { SCREEN_UIScreen::update(); }

    SCREEN_UI::EventReturn MainScreen::SettingsClick(SCREEN_UI::EventParams& e)
    {
        SettingsScreen* settingsScreen = new SettingsScreen();
        settingsScreen->OnAttach();
        SceneChangedEvent event(GameState::State::SETTINGS);
        Lucre::m_Application->OnAppEvent(event);
        UI::g_ScreenManager->push(settingsScreen);

        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene1Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::MAIN)
        {
            SceneChangedEvent event(GameState::State::MAIN);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene2Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::BEACH)
        {
            SceneChangedEvent event(GameState::State::BEACH);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene3Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::NIGHT)
        {
            SceneChangedEvent event(GameState::State::NIGHT);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::Scene4Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::DESSERT)
        {
            SceneChangedEvent event(GameState::State::DESSERT);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }
    SCREEN_UI::EventReturn MainScreen::Scene5Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::TERRAIN)
        {
            SceneChangedEvent event(GameState::State::TERRAIN);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }
    SCREEN_UI::EventReturn MainScreen::Scene6Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::ISLAND_2)
        {
            SceneChangedEvent event(GameState::State::ISLAND_2);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }
    SCREEN_UI::EventReturn MainScreen::Scene7Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::VOLCANO)
        {
            SceneChangedEvent event(GameState::State::VOLCANO);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }
    SCREEN_UI::EventReturn MainScreen::Scene8Click(SCREEN_UI::EventParams& e)
    {
        if (Lucre::m_Application->GetState() != GameState::State::RESERVED0)
        {
            SceneChangedEvent event(GameState::State::RESERVED0);
            Lucre::m_Application->OnAppEvent(event);
        }
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn MainScreen::OffClick(SCREEN_UI::EventParams& e)
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

    SCREEN_UI::EventReturn MainScreen::OffHold(SCREEN_UI::EventParams& e)
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
} // namespace LucreApp
