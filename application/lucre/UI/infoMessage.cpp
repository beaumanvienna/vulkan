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
#include "gui/common.h"
#include "UI/infoMessage.h"
#include "gui/Common/UI/viewGroup.h"

namespace LucreApp
{

    InfoMessage::InfoMessage(int align, SCREEN_UI::AnchorLayoutParams* lp)
        : SCREEN_UI::LinearLayout(SCREEN_UI::ORIENT_HORIZONTAL, lp)
    {
        using namespace SCREEN_UI;

        Add(new SCREEN_UI::Spacer(10.0f));
        m_TextView = Add(new SCREEN_UI::TextView("", align, false, new LinearLayoutParams(1.0, Margins(0, 10))));
        Add(new SCREEN_UI::Spacer(10.0f));

        m_ContextWidth = Engine::m_Engine->GetWindowWidth();
        m_ContextHeight = Engine::m_Engine->GetWindowHeight();
    }

    void InfoMessage::Show(const std::string& text, SCREEN_UI::View* refView)
    {
        if (refView)
        {
            Bounds b = refView->GetBounds();
            const SCREEN_UI::AnchorLayoutParams* lp = GetLayoutParams()->As<SCREEN_UI::AnchorLayoutParams>();
            if (b.y >= m_CutOffY)
            {
                ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(lp->width, lp->height, lp->left, 20.0f, lp->right,
                                                                      lp->bottom, lp->center));
            }
            else
            {
                ReplaceLayoutParams(new SCREEN_UI::AnchorLayoutParams(
                    lp->width, lp->height, lp->left, m_ContextHeight - 80.0f - 40.0f, lp->right, lp->bottom, lp->center));
            }
        }
        m_TextView->SetText(text);
        m_TimeShown = Engine::m_Engine->GetTimeDouble();
        m_TimeToShow = std::max(1.5, m_TextView->GetText().size() * 0.05);
    }

    void InfoMessage::Draw(SCREEN_UIContext& dc)
    {
        static constexpr double FADE_TIME = 1.0;
        static constexpr float MAX_ALPHA = 0.9f;
        double sinceShow;
        float alpha;

        if (m_TimeShown == 0.0)
        {
            return;
        }

        sinceShow = Engine::m_Engine->GetTimeDouble() - m_TimeShown;
        if (sinceShow > m_TimeToShow + FADE_TIME)
        {
            m_TimeShown = 0.0;
            return;
        }
        if (sinceShow > m_TimeToShow)
        {
            alpha = MAX_ALPHA - MAX_ALPHA * (float)((sinceShow - m_TimeToShow) / FADE_TIME);
        }
        else
        {
            alpha = MAX_ALPHA;
        }

        if (alpha >= 0.1f)
        {
            SCREEN_UI::Style style = dc.theme->popupTitle;
            style.background.color = colorAlpha(style.background.color, alpha - 0.1f);
            dc.FillRect(style.background, bounds_);
        }

        m_TextView->SetTextColor(whiteAlpha(alpha));
        m_TextView->SetShadow(false);
        ViewGroup::Draw(dc);
    }
} // namespace LucreApp
