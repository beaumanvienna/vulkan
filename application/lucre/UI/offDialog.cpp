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

#include "core.h"
#include "UI/UI.h"
#include "UI/offDialog.h"
#include "gui/Common/UI/viewGroup.h"
#include "gui/Common/Data/Text/i18n.h"
#include "gui/common.h"

#define TRANSPARENT_BACKGROUND true

namespace LucreApp
{
    OffDialog::OffDialog(std::string label, OffDiagEvent offDiagEvent)
        : SCREEN_PopupScreen(label), m_offDiagEvent(offDiagEvent)
    {
    }
    void OffDialog::CreatePopupContents(SCREEN_UI::ViewGroup* parent)
    {
        using namespace SCREEN_UI;

        auto ma = GetI18NCategory("Main");

        Choice* yesButton;
        Choice* cancelButton;

        LinearLayout* items = new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(WRAP_CONTENT, WRAP_CONTENT));
        float scale = UI::g_Common->m_ScaleAll * 720.0f / 1080.0f;
        float iconWidth = 265.0f * scale; // width is 530.0f * scale, see SCREEN_PopupScreen::SCREEN_PopupScreen()
        float iconHeight = UI::g_Common->m_IconHeight * 720.0f / 1080.0f;

        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            if (m_offDiagEvent == OFFDIAG_QUIT)
            {
                yesButton = new Choice(ma->T("YES"), TRANSPARENT_BACKGROUND, new LayoutParams(iconWidth, iconHeight));
                cancelButton = new Choice(ma->T("CANCEL"), TRANSPARENT_BACKGROUND, new LayoutParams(iconWidth, iconHeight));
                yesButton->OnClick.Handle(this, &OffDialog::QuitMarley);
                cancelButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
            }
            else
            {
                yesButton = new Choice(ma->T("YES"), TRANSPARENT_BACKGROUND, new LayoutParams(iconWidth, iconHeight));
                cancelButton = new Choice(ma->T("CANCEL"), TRANSPARENT_BACKGROUND, new LayoutParams(iconWidth, iconHeight));
                yesButton->OnClick.Handle(this, &OffDialog::SwitchOff);
                cancelButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
            }
        }
        else
        {
            if (m_offDiagEvent == OFFDIAG_QUIT)
            {
                yesButton = new Choice(ma->T("YES"), new LayoutParams(iconWidth, iconHeight));
                cancelButton = new Choice(ma->T("CANCEL"), new LayoutParams(iconWidth, iconHeight));
                yesButton->OnClick.Handle(this, &OffDialog::QuitMarley);
                cancelButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
            }
            else
            {
                yesButton = new Choice(ma->T("YES"), new LayoutParams(iconWidth, iconHeight));
                cancelButton = new Choice(ma->T("CANCEL"), new LayoutParams(iconWidth, iconHeight));
                yesButton->OnClick.Handle(this, &OffDialog::SwitchOff);
                cancelButton->OnClick.Handle<SCREEN_UIScreen>(this, &SCREEN_UIScreen::OnBack);
            }
        }

        yesButton->SetCentered(true);
        cancelButton->SetCentered(true);

        items->Add(yesButton);
        items->Add(cancelButton);

        parent->Add(items);
    }

    SCREEN_UI::EventReturn OffDialog::SwitchOff(SCREEN_UI::EventParams& e)
    {
        Engine::m_Engine->Shutdown(Engine::SWITCH_OFF_COMPUTER);

        return SCREEN_UI::EVENT_DONE;
    }

    SCREEN_UI::EventReturn OffDialog::QuitMarley(SCREEN_UI::EventParams& e)
    {
        Engine::m_Engine->Shutdown();

        return SCREEN_UI::EVENT_DONE;
    }
} // namespace LucreApp
