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

#pragma once

#include <iostream>

#include "engine.h"
#include "gui/Common/UI/UIscreen.h"
#include "gui/Common/UI/context.h"

namespace LucreApp
{

    class InfoMessage : public SCREEN_UI::LinearLayout
    {
    public:
        InfoMessage(int align, SCREEN_UI::AnchorLayoutParams* lp);

        void SetBottomCutoff(float y) { m_CutOffY = y; }
        void Show(const std::string& text, SCREEN_UI::View* refView = nullptr);
        void Draw(SCREEN_UIContext& dc);

    private:
        SCREEN_UI::TextView* m_TextView = nullptr;
        double m_TimeShown = 0.0;
        double m_TimeToShow = 0.0;
        float m_CutOffY = 0.0f;

        float m_ContextWidth;
        float m_ContextHeight;
    };
} // namespace LucreApp
