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
#include "UI/settingsTabs/credits.h"
#include "gui/Common/Render/drawBuffer.h"

namespace LucreApp
{
    Credits::Credits(SpriteSheet* spritesheet, SCREEN_UI::LayoutParams* layoutParams)
        : LinearLayout(SCREEN_UI::ORIENT_VERTICAL, layoutParams), m_Spritesheet(spritesheet)
    {
        CreateViews();
    }

    Credits::~Credits() {}

    bool Credits::Key(const SCREEN_KeyInput& input) { return LinearLayout::Key(input); }

    void Credits::CreateViews()
    {
        using namespace SCREEN_UI;

        float availableWidth = UI::g_Common->m_AvailableWidth - 2 * UI::g_Common->m_TabMarginLeftRight;
        float availableHeight = UI::g_Common->m_AvailableHeight;

        // Reset content
        Clear();

        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            Add(new Spacer(UI::g_Common->m_IconWidth / 2.0f));
        }
        else
        {
            Add(new Spacer(160.0f * UI::g_Common->m_ScaleAll - 69.0f - UI::g_Common->m_TabMargin));
        }

        float verticalSpace = (availableHeight - 4 * UI::g_Common->m_IconHeight);
        LinearLayout* creditsHorizontal =
            new LinearLayout(ORIENT_HORIZONTAL, new LinearLayoutParams(FILL_PARENT, verticalSpace));
        Add(creditsHorizontal);

        // vertical column for logos
        auto sprite = Sprite2D(m_Spritesheet->GetSprite(I_LOGO_PPSSPP));
        sprite.SetScale(UI::g_Common->m_ScaleAll);
        LinearLayout* logos = new LinearLayout(ORIENT_VERTICAL);
        creditsHorizontal->Add(logos);
        logos->Add(new Spacer(0.0f, verticalSpace / 2.0f));
        ImageView* ppssppLogo = new ImageView(sprite, new AnchorLayoutParams(sprite.GetWidth(), sprite.GetHeight()));
        logos->Add(ppssppLogo);

        creditsHorizontal->Add(new TextView("\n"
                                            "The in-game GUI used in Lucre is based on\n"
                                            "\n"
                                            "PPSSPP:\n"
                                            "www.ppsspp.org\n"
                                            "(license: GNU GPLv2)\n",
                                            ALIGN_HCENTER | ALIGN_VCENTER | FLAG_WRAP_TEXT, true,
                                            new LinearLayoutParams(availableWidth - sprite.GetWidth(), verticalSpace)));

        return;
    }

    void Credits::Update() { ViewGroup::Update(); }
} // namespace LucreApp
