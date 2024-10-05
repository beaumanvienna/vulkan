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
#include "coreSettings.h"
#include "platform/input.h"
#include "UI/UI.h"
#include "UI/settingsTabs/controllerSetup.h"
#include "gui/Common/Render/drawBuffer.h"

namespace LucreApp
{

    ControllerSetup::ControllerSetup(SpriteSheet* spritesheet, SCREEN_UI::LayoutParams* layoutParams)
        : LinearLayout(SCREEN_UI::ORIENT_VERTICAL, layoutParams), m_Spritesheet(spritesheet),
          m_ConfigurationIsRunningCtrl1(false), m_ConfigurationIsRunningCtrl2(false)
    {
        m_SpritesheetSettings.AddSpritesheetRow(m_Spritesheet->GetSprite(I_GEAR_R), 4 /* frames */);
        Refresh();
    }

    ControllerSetup::~ControllerSetup() { Controller::m_ControllerConfiguration.Reset(); }

    bool ControllerSetup::Key(const SCREEN_KeyInput& input)
    {
        if (IsRunning())
        {
            if (input.keyCode == ENGINE_KEY_ENTER)
            {
                if (input.flags == KEY_DOWN)
                {
                    Controller::m_ControllerConfiguration.SkipConfigStep();
                }
            }
            return false;
        }
        else
        {
            return LinearLayout::Key(input);
        }
    }

    void ControllerSetup::Refresh()
    {
        using namespace SCREEN_UI;

        float availableWidth = UI::g_Common->m_AvailableWidth - 2 * UI::g_Common->m_TabMarginLeftRight;
        float availableHeight = UI::g_Common->m_AvailableHeight;

        // float halfIconWidth  = UI::g_Common->m_IconWidth / 2;
        float halfIconHeight = UI::g_Common->m_IconHeight / 2;

        // Reset content
        Clear();

        bool controllerPlugged = Input::GetControllerCount();
        double verticalSpace = (availableHeight - 4 * UI::g_Common->m_IconHeight) / 2;

        if (!controllerPlugged)
        {
            Add(new Spacer(verticalSpace - halfIconHeight));
            TextView* noController = new TextView(" Please connect a controller", ALIGN_CENTER | FLAG_WRAP_TEXT, true,
                                                  new LinearLayoutParams(availableWidth, halfIconHeight));
            if (CoreSettings::m_UITheme == THEME_RETRO)
            {
                noController->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
                noController->SetShadow(true);
            }
            Add(noController);
            return;
        }

        Add(new Spacer(halfIconHeight));

        int controllerID = Controller::m_ControllerConfiguration.GetControllerID();
        m_ConfigurationIsRunningCtrl1 = controllerID == Controller::FIRST_CONTROLLER;
        m_ConfigurationIsRunningCtrl2 = controllerID == Controller::SECOND_CONTROLLER;

        // first controller
        {
            if (controllerPlugged && !m_ConfigurationIsRunningCtrl2)
            {
                LinearLayout* controllerHorizontal =
                    new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, verticalSpace));
                Add(controllerHorizontal);

                // setup button
                LinearLayout* verticalLayout =
                    new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(UI::g_Common->m_IconHeight, verticalSpace));
                controllerHorizontal->Add(verticalLayout);

                Sprite2D icon;
                Sprite2D icon_active;
                Sprite2D icon_depressed;

                // setup button
                Choice* setupButton;
                if (CoreSettings::m_UITheme == THEME_RETRO)
                {
                    icon = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
                    icon.SetScale(UI::g_Common->m_IconScaleRetro);
                    icon_active = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED));
                    icon_active.SetScale(UI::g_Common->m_IconScaleRetro);
                    icon_depressed = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
                    icon_depressed.SetScale(UI::g_Common->m_IconScaleRetro);

                    setupButton = new Choice(icon, icon_active, icon_depressed,
                                             new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconWidth));
                }
                else
                {
                    icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
                    icon.SetScale(UI::g_Common->m_IconScale);
                    setupButton = new Choice(icon, new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconHeight));
                }

                setupButton->OnClick.Handle(this, &ControllerSetup::OnStartSetup1);

                verticalLayout->Add(new Spacer(20.0f, (verticalSpace - UI::g_Common->m_IconHeight) / 2));
                verticalLayout->Add(setupButton);
                controllerHorizontal->Add(new Spacer(UI::g_Common->m_IconWidth));

                // text view 'instruction'
                LinearLayout* textViewLayout = new LinearLayout(
                    ORIENT_VERTICAL, new LinearLayoutParams(
                                         availableWidth - verticalSpace - UI::g_Common->m_IconHeight * 2.0f, verticalSpace));
                controllerHorizontal->Add(textViewLayout);

                m_TextSetup1 = new TextView(
                    (Controller::m_ControllerConfiguration.GetControllerID() == Controller::FIRST_CONTROLLER)
                        ? "press dpad up"
                        : "Start controller setup (1)",
                    ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, true,
                    new LinearLayoutParams(availableWidth - verticalSpace - UI::g_Common->m_IconHeight, verticalSpace));
                // text view 'skip button with return'
                m_TextSetup1b = new TextView(
                    ((Controller::m_ControllerConfiguration.GetControllerID() == Controller::FIRST_CONTROLLER))
                        ? "(or use ENTER to skip this button)"
                        : "",
                    ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, true,
                    new LinearLayoutParams(availableWidth - verticalSpace - UI::g_Common->m_IconHeight, halfIconHeight / 2));
                if (CoreSettings::m_UITheme == THEME_RETRO)
                {
                    m_TextSetup1->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
                    m_TextSetup1->SetShadow(true);
                    m_TextSetup1b->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
                    m_TextSetup1b->SetShadow(true);
                }
                textViewLayout->Add(m_TextSetup1);
                if (IsRunning())
                    textViewLayout->Add(m_TextSetup1b);
                controllerHorizontal->Add(new Spacer(1.5f * UI::g_Common->m_MarginLeftRight));

                // controller pic
                LinearLayout* controllerImageLayout =
                    new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, verticalSpace));

                Sprite2D controllerSprite = Sprite2D(m_Spritesheet->GetSprite(I_CONTROLLER));
                controllerSprite.SetScale(UI::g_Common->m_ControllerScale);
                controllerImageLayout->Add(new Spacer((verticalSpace - controllerSprite.GetHeight()) / 2 + 50.0f));

                ImageView* controllerImage = new ImageView(
                    controllerSprite, new AnchorLayoutParams(controllerSprite.GetWidth(), controllerSprite.GetHeight()));

                controllerImageLayout->Add(controllerImage);
                controllerHorizontal->Add(controllerImageLayout);
            }
            else
            {
                Add(new Spacer(verticalSpace));
            }
        }

        Add(new Spacer(halfIconHeight));

        // second controller
        {
            if ((Input::GetControllerCount() >= 2) && !m_ConfigurationIsRunningCtrl1)
            {
                LinearLayout* controllerHorizontal =
                    new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, verticalSpace));
                Add(controllerHorizontal);

                // setup button
                LinearLayout* verticalLayout =
                    new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(UI::g_Common->m_IconHeight, verticalSpace));
                controllerHorizontal->Add(verticalLayout);

                Sprite2D icon;
                Sprite2D icon_active;
                Sprite2D icon_depressed;

                // setup button
                Choice* setupButton;
                if (CoreSettings::m_UITheme == THEME_RETRO)
                {
                    icon = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_NOT_FOCUSED));
                    icon.SetScale(UI::g_Common->m_IconScaleRetro);
                    icon_active = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED));
                    icon_active.SetScale(UI::g_Common->m_IconScaleRetro);
                    icon_depressed = Sprite2D(m_SpritesheetSettings.GetSprite(BUTTON_4_STATES_FOCUSED_DEPRESSED));
                    icon_depressed.SetScale(UI::g_Common->m_IconScaleRetro);

                    setupButton = new Choice(icon, icon_active, icon_depressed,
                                             new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconWidth));
                }
                else
                {
                    icon = Sprite2D(m_Spritesheet->GetSprite(I_GEAR));
                    icon.SetScale(UI::g_Common->m_IconScale);
                    setupButton = new Choice(icon, new LayoutParams(UI::g_Common->m_IconWidth, UI::g_Common->m_IconHeight));
                }

                setupButton->OnClick.Handle(this, &ControllerSetup::OnStartSetup2);

                verticalLayout->Add(new Spacer(20.0f, (verticalSpace - UI::g_Common->m_IconHeight) / 2));
                verticalLayout->Add(setupButton);
                controllerHorizontal->Add(new Spacer(UI::g_Common->m_IconWidth));

                // text view 'instruction'
                LinearLayout* textViewLayout = new LinearLayout(
                    ORIENT_VERTICAL, new LinearLayoutParams(
                                         availableWidth - verticalSpace - UI::g_Common->m_IconHeight * 2.0f, verticalSpace));
                controllerHorizontal->Add(textViewLayout);

                m_TextSetup2 = new TextView(
                    (Controller::m_ControllerConfiguration.GetControllerID() == Controller::SECOND_CONTROLLER)
                        ? "press dpad up"
                        : "Start controller setup (2)",
                    ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, true,
                    new LinearLayoutParams(availableWidth - verticalSpace - UI::g_Common->m_IconHeight, verticalSpace));
                // text view 'skip button with return'
                m_TextSetup2b = new TextView(
                    ((Controller::m_ControllerConfiguration.GetControllerID() == Controller::SECOND_CONTROLLER))
                        ? "(or use ENTER to skip this button)"
                        : "",
                    ALIGN_VCENTER | ALIGN_HCENTER | FLAG_WRAP_TEXT, true,
                    new LinearLayoutParams(availableWidth - verticalSpace - UI::g_Common->m_IconHeight, halfIconHeight / 2));
                if (CoreSettings::m_UITheme == THEME_RETRO)
                {
                    m_TextSetup2->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
                    m_TextSetup2->SetShadow(true);
                    m_TextSetup2b->SetTextColor(RETRO_COLOR_FONT_FOREGROUND);
                    m_TextSetup2b->SetShadow(true);
                }
                textViewLayout->Add(m_TextSetup2);
                if (IsRunning())
                    textViewLayout->Add(m_TextSetup2b);
                controllerHorizontal->Add(new Spacer(1.5f * UI::g_Common->m_MarginLeftRight));

                // controller pic
                LinearLayout* controllerImageLayout =
                    new LinearLayout(ORIENT_VERTICAL, new LinearLayoutParams(FILL_PARENT, verticalSpace));
                Sprite2D controllerSprite = Sprite2D(m_Spritesheet->GetSprite(I_CONTROLLER));
                controllerSprite.SetScale(UI::g_Common->m_ControllerScale);
                controllerImageLayout->Add(new Spacer((verticalSpace - controllerSprite.GetHeight()) / 2 + 50.0f));

                ImageView* controllerImage = new ImageView(
                    controllerSprite, new AnchorLayoutParams(controllerSprite.GetWidth(), controllerSprite.GetHeight()));

                controllerImageLayout->Add(controllerImage);
                controllerHorizontal->Add(controllerImageLayout);
            }
        }
    }

    void ControllerSetup::Update()
    {
        static int controllerCount = Input::GetControllerCount();
        static int controllerCountPrevious = Input::GetControllerCount();
        bool refreshControllerCount;

        controllerCount = Input::GetControllerCount();
        refreshControllerCount = controllerCountPrevious != controllerCount;
        controllerCountPrevious = controllerCount;

        static bool configurationIsRunning = IsRunning();
        static int configurationIsRunningPrevious = IsRunning();
        bool refreshConfigurationIsRunning;

        configurationIsRunning = IsRunning();
        refreshConfigurationIsRunning = configurationIsRunningPrevious != configurationIsRunning;
        configurationIsRunningPrevious = configurationIsRunning;

        if (refreshControllerCount || refreshConfigurationIsRunning)
        {
            Refresh();
        }
        if (Input::ControllerMappingCreated())
        {
            SCREEN_UI::EventParams e{};
            e.v = this;
            OnMappingSuccessful.Trigger(e);
        }

        SetControllerConfText();
        if (Controller::m_ControllerConfiguration.MappingCreated())
            Controller::m_ControllerConfiguration.Reset();

        ViewGroup::Update();
    }

    SCREEN_UI::EventReturn ControllerSetup::OnStartSetup1(SCREEN_UI::EventParams& e)
    {
        Input::StartControllerConfig(Controller::FIRST_CONTROLLER);

        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn ControllerSetup::OnStartSetup2(SCREEN_UI::EventParams& e)
    {
        Input::StartControllerConfig(Controller::SECOND_CONTROLLER);

        return SCREEN_UI::EVENT_DONE;
    }

    void ControllerSetup::SetControllerConfText()
    {
        int controllerID = Controller::m_ControllerConfiguration.GetControllerID();
        bool update = Controller::m_ControllerConfiguration.UpdateControllerText();
        std::string text1 = Controller::m_ControllerConfiguration.GetText(ControllerConfiguration::TEXT1);
        std::string text2 = Controller::m_ControllerConfiguration.GetText(ControllerConfiguration::TEXT2);

        if (update)
        {
            if (controllerID == Controller::FIRST_CONTROLLER)
            {
                m_TextSetup1->SetText(text1);
                m_TextSetup1b->SetText(text2);
            }
            else if (controllerID == Controller::SECOND_CONTROLLER)
            {
                m_TextSetup2->SetText(text1);
                m_TextSetup2b->SetText(text2);
            }
            Controller::m_ControllerConfiguration.ResetControllerText();
        }
    }
} // namespace LucreApp
