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

#include "core.h"
#include "lucre.h"
#include "appEvent.h"
#include "UI/common.h"
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

    SettingsScreen::SettingsScreen() : m_Spritesheet{Lucre::m_Spritesheet} {}

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

    bool SettingsScreen::key(const SCREEN_KeyInput& key)
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
        auto ge = GetI18NCategory("General");

        root_ = new AnchorLayout(new LayoutParams(FILL_PARENT, FILL_PARENT));
        root_->SetTag("setting screen root");

        LinearLayout* verticalLayout = new LinearLayout(ORIENT_VERTICAL, new LayoutParams(FILL_PARENT, FILL_PARENT));
        verticalLayout->SetTag("main verticalLayout settings screen");
        root_->Add(verticalLayout);

        // info message
        m_SettingsInfo =
            new InfoMessage(ALIGN_CENTER | FLAG_WRAP_TEXT,
                            new AnchorLayoutParams(UI::g_Common->m_AvailableWidth - 6 * UI::g_Common->m_IconWidth,
                                                   WRAP_CONTENT, 4 * UI::g_Common->m_IconWidth, 0.0f, NONE, NONE));
        m_SettingsInfo->SetBottomCutoff(UI::g_Common->m_AvailableHeight - UI::g_Common->m_IconHeight);

        root_->Add(m_SettingsInfo);

        verticalLayout->Add(new Spacer(UI::g_Common->m_TabMargin));

        m_TabHolder = new TabHolder(ORIENT_HORIZONTAL, UI::g_Common->m_StripSize, new LinearLayoutParams(1.0f),
                                    UI::g_Common->m_TabMargin);
        m_TabHolder->SetTag("m_TabHolder");
        verticalLayout->Add(m_TabHolder);

        Sprite2D icon;
        Sprite2D icon_active;
        Sprite2D icon_depressed;
        Sprite2D icon_depressed_inactive;

        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetTab.GetSprite(BUTTON_2_STATES_NOT_FOCUSED));
            icon.SetScale(UI::g_Common->m_TabIconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetTab.GetSprite(BUTTON_2_STATES_FOCUSED));
            icon_active.SetScale(UI::g_Common->m_TabIconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetTab.GetSprite(BUTTON_2_STATES_FOCUSED));
            icon_depressed.SetScale(UI::g_Common->m_TabIconScaleRetro);
            icon_depressed_inactive = Sprite2D(m_SpritesheetTab.GetSprite(BUTTON_2_STATES_NOT_FOCUSED));
            icon_depressed_inactive.SetScale(UI::g_Common->m_TabIconScaleRetro);
            m_TabHolder->SetIcon(icon, icon_active, icon_depressed, icon_depressed_inactive);
        }

        // back button
        Choice* backButton;
        LinearLayout* horizontalLayoutBack =
            new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(FILL_PARENT, UI::g_Common->m_IconHeight));
        horizontalLayoutBack->SetTag("horizontalLayoutBack");
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            icon = Sprite2D(m_SpritesheetBack.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
            icon.SetScale(UI::g_Common->m_IconScaleRetro);
            icon_active = Sprite2D(m_SpritesheetBack.GetSprite(BUTTON_4_STATES_FOCUSED));
            icon_active.SetScale(UI::g_Common->m_IconScaleRetro);
            icon_depressed = Sprite2D(m_SpritesheetBack.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
            icon_depressed.SetScale(UI::g_Common->m_IconScaleRetro);
            backButton = new Choice(icon, icon_active, icon_depressed,
                                    new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconHeight));
        }
        else
        {
            icon = Sprite2D(m_Spritesheet->GetSprite(I_BACK));
            icon.SetScale(UI::g_Common->m_IconScale);
            backButton = new Choice(icon, new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconHeight));
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
        LinearLayout* horizontalLayoutGeneral =
            new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(UI::g_Common->m_TabLayoutWidth, FILL_PARENT));
        m_TabHolder->AddTab(ge->T("General"), horizontalLayoutGeneral);
        horizontalLayoutGeneral->Add(new Spacer(UI::g_Common->m_TabMarginLeftRight));

        ViewGroup* generalSettingsScroll =
            new ScrollView(ORIENT_VERTICAL, new LinearLayoutParams(UI::g_Common->m_TabLayoutWidth, FILL_PARENT));
        horizontalLayoutGeneral->Add(generalSettingsScroll);
        generalSettingsScroll->SetTag("GeneralSettings");
        LinearLayout* generalSettings = new LinearLayout(ORIENT_VERTICAL);
        generalSettingsScroll->Add(generalSettings);

        generalSettings->Add(new ItemHeader(ge->T("General settings for Lucre")));

        // -------- toggle fullscreen --------
        m_EnableFullscreen = Engine::m_Engine->IsFullscreen();
        CheckBox* vToggleFullscreen =
            generalSettings->Add(new CheckBox(&m_EnableFullscreen, ge->T("Fullscreen", "Fullscreen"), "",
                                              new LayoutParams(FILL_PARENT, UI::g_Common->m_SettingsBar)));
        vToggleFullscreen->OnClick.Handle(this, &SettingsScreen::OnFullscreenToggle);

        // -------- system sounds --------
        CheckBox* vSystemSounds = generalSettings->Add(
            new CheckBox(&CoreSettings::m_EnableSystemSounds, ge->T("Enable system sounds", "Enable system sounds"), "",
                         new LayoutParams(FILL_PARENT, UI::g_Common->m_SettingsBar)));
        vSystemSounds->OnClick.Add([=](EventParams& e) { return SCREEN_UI::EVENT_CONTINUE; });

        // desktop volume
        m_GlobalVolume = Sound::GetDesktopVolume();
        const int VOLUME_OFF = 0;
        const int VOLUME_MAX = 100;

        SCREEN_PopupSliderChoice* volume = generalSettings->Add(
            new SCREEN_PopupSliderChoice(&m_GlobalVolume, VOLUME_OFF, VOLUME_MAX, ge->T("Global Volume"), "",
                                         new LayoutParams(FILL_PARENT, UI::g_Common->m_SettingsBar)));
        m_GlobalVolumeEnabled = true;
        volume->SetEnabledPtr(&m_GlobalVolumeEnabled);
        volume->SetZeroLabel(ge->T("Mute"));

        volume->OnChange.Add(
            [this](EventParams& e)
            {
                Sound::SetDesktopVolume(m_GlobalVolume);
                return SCREEN_UI::EVENT_CONTINUE;
            });

// audio device list
#ifdef LINUX
        float widthSelectAudioDevice = UI::g_Common->m_AvailableWidth - UI::g_Common->m_TabMarginLeftRight;
        std::vector<std::string>& audioDeviceList = Sound::GetOutputDeviceList();
        m_AudioDevice = Sound::GetDefaultOutputDevice();
        auto selectAudioDevice = new SCREEN_PopupMultiChoiceDynamic(
            &m_AudioDevice, ge->T("Device"), audioDeviceList, nullptr, screenManager(),
            new LayoutParams(FILL_PARENT, UI::g_Common->m_SettingsBar), widthSelectAudioDevice);
        SCREEN_PopupMultiChoiceDynamic* audioDevice = generalSettings->Add(selectAudioDevice);
        audioDevice->OnChoice.Handle(this, &SettingsScreen::OnAudioDevice);
#endif

        // -------- theme --------
        static const char* uiTheme[] = {"Retro", "Plain"};

        SCREEN_PopupMultiChoice* uiThemeChoice = generalSettings->Add(new SCREEN_PopupMultiChoice(
            &CoreSettings::m_UITheme, ge->T("Theme"), uiTheme, 0, ARRAY_SIZE(uiTheme), ge->GetName(), screenManager(),
            new LayoutParams(FILL_PARENT, UI::g_Common->m_SettingsBar)));
        uiThemeChoice->OnChoice.Handle(this, &SettingsScreen::OnThemeChanged);

        // -------- controller setup --------

        // horizontal layout for margins
        LinearLayout* horizontalLayoutController =
            new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(UI::g_Common->m_TabLayoutWidth, FILL_PARENT));
        horizontalLayoutController->SetTag("horizontalLayoutController");
        m_TabHolder->AddTab(ge->T("Controller"), horizontalLayoutController);

        horizontalLayoutController->Add(new Spacer(UI::g_Common->m_TabMarginLeftRight));

        m_ControllerSetup = new ControllerSetup(m_Spritesheet);
        horizontalLayoutController->Add(m_ControllerSetup);
        m_ControllerSetup->OnMappingSuccessful.Add(
            [this](EventParams& e)
            {
                m_SettingsInfo->Show("Mapping successful", e.v);
                return SCREEN_UI::EVENT_CONTINUE;
            });

        // -------- credits --------

        // horizontal layout for margins
        LinearLayout* horizontalLayoutCredits =
            new LinearLayout(ORIENT_HORIZONTAL, new LayoutParams(UI::g_Common->m_TabLayoutWidth, FILL_PARENT));
        horizontalLayoutCredits->SetTag("horizontalLayoutCredits");
        m_TabHolder->AddTab(ge->T("Credits"), horizontalLayoutCredits);

        horizontalLayoutCredits->Add(new Spacer(UI::g_Common->m_TabMarginLeftRight));

        m_Credits = new Credits(m_Spritesheet);
        horizontalLayoutCredits->Add(m_Credits);

        LOG_APP_INFO("UI: views for setting screen created");
    }

    void SettingsScreen::onFinish(DialogResult result)
    {
        SceneFinishedEvent event;
        Lucre::m_Application->OnAppEvent(event);
    }

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

    SCREEN_UI::EventReturn SettingsScreen::OnFullscreenToggle(SCREEN_UI::EventParams& e)
    {
        Engine::m_Engine->ToggleFullscreen();
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SettingsScreen::OnThemeChanged(SCREEN_UI::EventParams& e)
    {
        UI::g_ScreenManager->RecreateAllViews();
        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn SettingsScreen::OnAudioDevice(SCREEN_UI::EventParams& e)
    {
        auto audioDevice = m_AudioDevice.substr(0, 60);
        std::vector<std::string>& audioDeviceList = Sound::GetOutputDeviceList();
        for (auto device : audioDeviceList)
        {
            if (audioDevice == device.substr(0, 60))
            {
                Sound::SetOutputDevice(device);
                UI::g_ScreenManager->RecreateAllViews();
            }
        }
        return SCREEN_UI::EVENT_DONE;
    }

    void SettingsScreen::SetSoundCallback()
    {
#ifdef PULSEAUDIO
        Sound::SetCallback([=](const LibPAmanager::Event& event) { UI::g_ScreenManager->RecreateAllViews(); });
#endif
    }
} // namespace LucreApp