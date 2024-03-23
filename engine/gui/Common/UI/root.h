/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
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

#include <functional>

#include "gui/Common/UI/view.h"

namespace GfxRenderEngine
{

    namespace SCREEN_UI
    {

        void EnableFocusMovement(bool enable);
        bool IsFocusMovementEnabled();
        View* GetFocusedView();
        void SetFocusedView(View* view, bool force = false);
        void RemoveQueuedEventsByEvent(Event* e);
        void RemoveQueuedEventsByView(View* v);

        void EventTriggered(Event* e, EventParams params);
        void DispatchEvents();

        class ViewGroup;

        void LayoutViewHierarchy(const SCREEN_UIContext& dc, ViewGroup* root, bool ignoreInsets);
        void UpdateViewHierarchy(ViewGroup* root);

        bool KeyEvent(const SCREEN_KeyInput& key, ViewGroup* root);
        bool TouchEvent(const SCREEN_TouchInput& touch, ViewGroup* root);
        bool AxisEvent(const SCREEN_AxisInput& axis, ViewGroup* root);

        enum class SCREEN_UISound
        {
            SELECT = 0,
            BACK,
            CONFIRM,
            TOGGLE_ON,
            TOGGLE_OFF,
            COUNT,
        };

        void SetSoundEnabled(bool enabled);
        void SetSoundCallback(std::function<void(SCREEN_UISound)> func);

        void PlayUISound(SCREEN_UISound sound);

    } // namespace SCREEN_UI
} // namespace GfxRenderEngine
