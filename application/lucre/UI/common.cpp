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

#include "UI/common.h"
#include "core.h"
#include "gui/common.h"

namespace LucreApp
{
    Common::Common() { Init(); }

    void Common::Init()
    {
        m_AvailableWidth = Engine::m_Engine->GetWindowWidth();
        m_AvailableHeight = Engine::m_Engine->GetWindowHeight();

        m_ScaleAll = m_AvailableHeight / 720.0f;

        m_IconWidth = 64.0f * m_ScaleAll;
        m_IconHeight = 64.0f * m_ScaleAll;
        m_IconSpacer = 20.0f * m_ScaleAll;

        m_StripSize = 150.0f * m_ScaleAll;
        m_MarginLeftRight = 32.0f * m_ScaleAll;
        m_SettingsBar = 85.0f * m_ScaleAll;

        m_TabMargin = 50.0f * m_ScaleAll;
        m_TabMarginLeftRight = 80.0f * m_ScaleAll;
        m_TabLayoutWidth = (m_AvailableWidth - 2.0f * m_TabMarginLeftRight);

        m_ControllerScale = 1.0f * m_ScaleAll;

        m_TabIconScaleRetro = 1.0 * m_ScaleAll;
        m_IconScaleRetro = 0.5 * m_ScaleAll;
        m_IconScale = 1.0 * m_ScaleAll;
    }

    void Common::OnResize() { Init(); }
} // namespace LucreApp
