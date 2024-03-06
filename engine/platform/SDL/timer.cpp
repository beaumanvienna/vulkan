/* Controller Copyright (c) 2021 Controller Development Team
   https://github.com/beaumanvienna/gfxRenderController

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

#include <SDL.h>

#include "platform/SDL/timer.h"

namespace GfxRenderEngine
{

    Timer::Timer(uint interval) : m_Interval(interval), m_TimerID(0), m_TimerCallback(nullptr) {}

    Timer::Timer(uint interval, TimerCallbackFunction& callback)
        : m_Interval(interval), m_TimerID(0), m_TimerCallback(callback)
    {
    }

    Timer::~Timer() { Stop(); }

    void Timer::Start() { m_TimerID = SDL_AddTimer(m_Interval, (SDL_TimerCallback)m_TimerCallback, (void*)(&m_TimerID)); }

    void Timer::Stop() { SDL_RemoveTimer(m_TimerID); }

    void Timer::SetEventCallback(const TimerCallbackFunction& callback) { m_TimerCallback = callback; }
} // namespace GfxRenderEngine
