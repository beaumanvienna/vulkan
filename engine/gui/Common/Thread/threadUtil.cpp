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
#include <cstring>
#include <cstdint>

#include "engine.h"
#include "gui/Common/Thread/threadUtil.h"

#if defined(__ANDROID__) || defined(__APPLE__) || (defined(__GLIBC__) && defined(_GNU_SOURCE))
#include <pthread.h>
#endif

namespace GfxRenderEngine
{
#ifdef TLS_SUPPORTED
    static thread_local const char* curThreadName;
#endif

    void setCurrentThreadName(const char* threadName)
    {
        // Set the locally known threadname using a thread local variable.
#ifdef TLS_SUPPORTED
        curThreadName = threadName;
#endif
    }

    void AssertCurrentThreadName(const char* threadName)
    {
#ifdef TLS_SUPPORTED
        if (strcmp(curThreadName, threadName) != 0)
        {
            LOG_CORE_ERROR("Thread name assert failed: Expected {0}, was {1}", threadName, curThreadName);
        }
#endif
    }
} // namespace GfxRenderEngine
