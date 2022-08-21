/* Engine Copyright (c) 2022 Engine Development Team 
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

#include "core.h"
#include "lucre.h"
#include "UI/settingsScreen.h"
#include "auxiliary/instrumentation.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/Data/Text/i18n.h"
#include "platform/keyCodes.h"
#include "UI/UI.h"

namespace LucreApp
{

    bool SettingsScreen::m_IsCreditsScreen = false;
    bool SettingsScreen::m_IsCintrollerSetupScreen = false;

    SettingsScreen::SettingsScreen()
        : m_Spritesheet{Lucre::m_Spritesheet}
    {}

    SettingsScreen::~SettingsScreen()
    {
        m_IsCreditsScreen = false;
        m_IsCintrollerSetupScreen = false;
    }

    void SettingsScreen::OnAttach()
    { 
        m_SpritesheetTab.AddSpritesheetRow(m_Spritesheet->GetSprite(I_TAB_R), 2 /* frames */, TAB_SCALE);
        m_SpritesheetBack.AddSpritesheetRow(m_Spritesheet->GetSprite(I_BACK_R), 4 /* frames */);
        m_LastTab = 0;

        SetSoundCallback();
    }

    bool SettingsScreen::key(const SCREEN_KeyInput &key)
    {
        if (m_ControllerSetup->IsRunning())
        {
            if (key.keyCode == ENGINE_KEY_ENTER)
            {
                m_ControllerSetup->Key(key);
            }
            else if (key.keyCode == ENGINE_KEY_ESCAPE)
            {
                return SCREEN_UIDialogScreen::key(key);
            }
            return false;
        }
        else
        {
            return SCREEN_UIDialogScreen::key(key);
        }
    }

    void SettingsScreen::CreateViews()
    {
        PROFILE_FUNCTION();
        using namespace SCREEN_UI;
        auto ge  = GetI18NCategory("General");

        root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
        root_->SetTag("setting screen root");

        LinearLayout *verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
        verticalLayout->SetTag("main verticalLayout settings screen");
        root_->Add(verticalLayout);

        float availableWidth  = Engine::m_Engine->GetWindowWidth();
        float availableHeight = Engine::m_Engine->GetWindowHeight();

        float iconWidth = 50.0f;
        float iconHeight = iconWidth;
        float stripSize = 100.0f * TAB_SCALE;
        float tabMargin = 50.0f;
        float tabMarginLeftRight = 80.0f;
        float tabLayoutWidth = availableWidth - 2 * tabMarginLeftRight;

        // info message
        m_SettingsInfo = new InfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT, new AnchorLayoutParams(availableWidth - 6 * iconWidth, WRAP_CONTENT, 4 * iconWidth, 0.0f, NONE, NONE));
        m_SettingsInfo->SetBottomCutoff(availableHeight - iconHeight);

        root_->Add(m_SettingsInfo);

        verticalLayout->Add(new Spacer(tabMargin));

        m_TabHolder = new TabHolder(ORIENT_HORIZONTAL, stripSize, new LinearLayoutParams(1.0f), tabMargin);
        m_TabHolder->SetTag("m_TabHolder");
        verticalLayout->Add(m_TabHolder);

        if (CoreSettings::m_UITheme == THEME_RETRO)
        { 
            Sprite* icon;
            Sprite* icon_active;
            Sprite* icon_depressed;
            Sprite* icon_depressed_inactive;

            icon = m_SpritesheetTab.GetSprite(BUTTON_2_STATES_NOT_FOCUSED);
            icon->SetScale(1.0f);
            icon_active = m_SpritesheetTab.GetSprite(BUTTON_2_STATES_FOCUSED);
            icon_active->SetScale(1.0f);
            icon_depressed = m_SpritesheetTab.GetSprite(BUTTON_2_STATES_FOCUSED);
            icon_depressed->SetScale(1.0f);
            icon_depressed_inactive = m_SpritesheetTab.GetSprite(BUTTON_2_STATES_NOT_FOCUSED);
            icon_depressed_inactive->SetScale(1.0f);
            m_TabHolder->SetIcon(icon,icon_active,icon_depressed,icon_depressed_inactive);
        }

        // back button
        Choice* backButton;
        LinearLayout *horizontalLayoutBack = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(FILL_PARENT, iconHeight));
        horizontalLayoutBack->SetTag("horizontalLayoutBack");
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            Sprite* icon = m_SpritesheetBack.GetSprite(BUTTON_4_STATES_NOT_FOCUSED);
            icon->SetScale(iconWidth);
            Sprite* icon_active = m_SpritesheetBack.GetSprite(BUTTON_4_STATES_FOCUSED);
            icon_active->SetScale(iconWidth);
            Sprite* icon_depressed = m_SpritesheetBack.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED);
            icon_depressed->SetScale(iconWidth);
            backButton = new Choice(icon, icon_active, icon_depressed, new LayoutParams(iconWidth, iconHeight));
        }
        else
        {
            Sprite* icon = m_Spritesheet->GetSprite(I_BACK);
            icon->SetScale(1.0f);
            backButton = new Choice(icon, new LayoutParams(iconWidth, iconHeight));
        }
        backButton->SetTag("backButton");
        backButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
        horizontalLayoutBack->Add(new Spacer(40.0f));
        horizontalLayoutBack->Add(backButton);
        verticalLayout->Add(horizontalLayoutBack);
        verticalLayout->Add(new Spacer(40.0f));

        root_->SetDefaultFocusView(m_TabHolder);

        // -------- general --------

        // horizontal layout for margins
        LinearLayout *horizontalLayoutGeneral = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(tabLayoutWidth, FILL_PARENT));
        m_TabHolder->AddTab(ge->T("General"), horizontalLayoutGeneral);
        horizontalLayoutGeneral->Add(new Spacer(tabMarginLeftRight));

        ViewGroup *generalSettingsScroll = new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(tabLayoutWidth, FILL_PARENT));
        horizontalLayoutGeneral->Add(generalSettingsScroll);
        generalSettingsScroll->SetTag("GeneralSettings");
        LinearLayout *generalSettings = new LinearLayout(ORIENT_VERTICAL);
        generalSettingsScroll->Add(generalSettings);

        generalSettings->Add(new ItemHeader(ge->T("General settings for Lucre")));

        // -------- toggle fullscreen --------
        CheckBox *vToggleFullscreen = generalSettings->Add(new CheckBox(&CoreSettings::m_EnableFullscreen, ge->T("Fullscreen", "Fullscreen"),"", new LayoutParams(FILL_PARENT,85.0f)));
        vToggleFullscreen->OnClick.Handle(this, &SettingsScreen::OnFullscreenToggle);

        // -------- system sounds --------
        CheckBox *vSystemSounds = generalSettings->Add(new CheckBox(&CoreSettings::m_EnableSystemSounds, ge->T("Enable system sounds", "Enable system sounds"),"", new LayoutParams(FILL_PARENT,85.0f)));
        vSystemSounds->OnClick.Add([=](EventParams &e) 
        {
            return SCREEN_UI::EVENT_CONTINUE;
        });

        // desktop volume
        m_GlobalVolume = Sound::GetDesktopVolume();
        const int VOLUME_OFF = 0;
        const int VOLUME_MAX = 100;

        SCREEN_PopupSliderChoice *volume = generalSettings->Add(new SCREEN_PopupSliderChoice(&m_GlobalVolume, VOLUME_OFF, VOLUME_MAX, ge->T("Global Volume"), "", new LayoutParams(FILL_PARENT,85.0f)));
        m_GlobalVolumeEnabled = true;
        volume->SetEnabledPtr(&m_GlobalVolumeEnabled);
        volume->SetZeroLabel(ge->T("Mute"));

        volume->OnChange.Add([=](EventParams &e) 
        {
            Sound::SetDesktopVolume(m_GlobalVolume);
            return SCREEN_UI::EVENT_CONTINUE;
        });

        // audio device list
        #ifdef LINUX
            std::vector<std::string>& audioDeviceList = Sound::GetOutputDeviceList();
            m_AudioDevice = Sound::GetDefaultOutputDevice();
            auto selectAudioDevice = new SCREEN_PopupMultiChoiceDynamic(&m_AudioDevice,
                                                                        ge->T("Device"),
                                                                        audioDeviceList,
                                                                        nullptr,
                                                                        screenManager(),
                                                                        new LayoutParams(FILL_PARENT, 85.0f), 1800.0f);
            SCREEN_PopupMultiChoiceDynamic* audioDevice = generalSettings->Add(selectAudioDevice);
            audioDevice->OnChoice.Handle(this, &SettingsScreen::OnAudioDevice);
        #endif

        // -------- theme --------
        static const char *uiTheme[] = 
        {
            "Retro",
            "Plain"
        };

        SCREEN_PopupMultiChoice *uiThemeChoice = generalSettings->Add(new SCREEN_PopupMultiChoice(&CoreSettings::m_UITheme, ge->T("Theme"),
            uiTheme, 0, ARRAY_SIZE(uiTheme), ge->GetName(), screenManager(), new LayoutParams(FILL_PARENT, 85.0f)));
        uiThemeChoice->OnChoice.Handle(this, &SettingsScreen::OnThemeChanged);

        // -------- controller setup --------

        // horizontal layout for margins
        LinearLayout *horizontalLayoutController = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(tabLayoutWidth, FILL_PARENT));
        horizontalLayoutController->SetTag("horizontalLayoutController");
        m_TabHolder->AddTab(ge->T("Controller"), horizontalLayoutController);

        horizontalLayoutController->Add(new Spacer(tabMarginLeftRight));

        m_ControllerSetup = new ControllerSetup(m_Spritesheet);
        horizontalLayoutController->Add(m_ControllerSetup);
        m_ControllerSetup->OnMappingSuccessful.Add([=](EventParams &e) 
        {
            m_SettingsInfo->Show("Mapping successful", e.v);
            return SCREEN_UI::EVENT_CONTINUE;
        });

        // -------- credits --------

        // horizontal layout for margins
        LinearLayout *horizontalLayoutCredits = new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(tabLayoutWidth, FILL_PARENT));
        horizontalLayoutCredits->SetTag("horizontalLayoutCredits");
        m_TabHolder->AddTab(ge->T("Credits"), horizontalLayoutCredits);
        horizontalLayoutCredits->Add(new Spacer(iconWidth));

        LinearLayout *logos = new LinearLayout(ORIENT_VERTICAL);
        logos->SetTag("logos");
        horizontalLayoutCredits->Add(logos);
        ImageView* ppssppLogo   = new ImageView(m_Spritesheet->GetSprite(I_LOGO_PPSSPP),   new AnchorLayoutParams(192.0f, 128.0f, 1.0f, 1.0f, NONE, NONE, false));
        ppssppLogo->SetTag("ppssppLogo");
        logos->Add(new Spacer(27.0f));
        logos->Add(ppssppLogo);

        LinearLayout *credits = new LinearLayout(ORIENT_VERTICAL);
        credits->SetTag("credits");
        horizontalLayoutCredits->Add(credits);
        credits->Add(new Spacer(iconWidth));

        credits->Add(new TextView
        (
            "\n"
            "     The in-game GUI of this project is based on the project\n"
            "\n"
            "     PPSSPP:          www.ppsspp.org (license: GNU GPLv2)\n"
            "\n",
            ALIGN_LEFT | ALIGN_VCENTER | FLAG_WRAP_TEXT, true, new LinearLayoutParams(availableWidth - 3.0f * iconWidth - 64.0f, 500.0f)));

        LOG_APP_INFO("UI: views for setting screen created");
    }

    void SettingsScreen::onFinish(DialogResult result) {}

    void SettingsScreen::update()
    {
        m_IsCreditsScreen = m_TabHolder->GetCurrentTab() == CREDITS_SCREEN;
        m_IsCintrollerSetupScreen = m_TabHolder->GetCurrentTab() == CONTROLLER_SETUP_SCREEN;

        if (m_TabHolder->HasFocus(m_LastTab))
        {
            m_TabHolder->enableAllTabs();
        }
        else
        {
            m_TabHolder->disableAllTabs();
            m_TabHolder->SetEnabled(m_LastTab);
        }

        SCREEN_UIScreen::update();
    }

    SCREEN_UI::EventReturn SettingsScreen::OnFullscreenToggle(SCREEN_UI::EventParams &e)
    {
        Engine::m_Engine->ToggleFullscreen();
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SettingsScreen::OnThemeChanged(SCREEN_UI::EventParams &e)
    {
        UI::m_ScreenManager->RecreateAllViews();
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SettingsScreen::OnAudioDevice(SCREEN_UI::EventParams &e)
    {
        auto audioDevice = m_AudioDevice.substr(0, 60);
        std::vector<std::string>& audioDeviceList = Sound::GetOutputDeviceList();
        for (auto device : audioDeviceList)
        {
            if (audioDevice == device.substr(0, 60))
            {
                Sound::SetOutputDevice(device);
                UI::m_ScreenManager->RecreateAllViews();
            }
        }
        return SCREEN_UI::EVENT_DONE;
    }

    void SettingsScreen::SetSoundCallback()
    {
        #ifdef LINUX
            Sound::SetCallback([=](const LibPAmanager::Event& event)
            {
                UI::m_ScreenManager->RecreateAllViews();
            });
        #endif
    }
}
