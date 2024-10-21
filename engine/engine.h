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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <iostream>
#include <chrono>

#include "log/log.h"
#include "auxiliary/instrumentation.h"
#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#endif

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm.hpp"
#include "gtc/constants.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/transform.hpp"
#include "gtx/compatibility.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/hash.hpp"

typedef uint8_t uchar;
typedef uint16_t uint16;
typedef uint32_t uint;
typedef int64_t int64;
typedef uint64_t uint64;
typedef void (*GenericCallback)();

using namespace std::chrono_literals;
using namespace GfxRenderEngine;

int engine(int argc, char* argv[]);

#undef far
#undef near
#undef CopyFile
#undef CreateDirectory
#undef CreateWindow

#define MEMBER_SIZE(type, member) sizeof(type::member)

#define BIT(x) (1 << (x))

// profiling
extern std::unique_ptr<GfxRenderEngine::Instrumentation::Profiler> g_Profiler;

// logging
#ifndef DISTRIBUTION_BUILD
#define LOGGING_AND_ASSERTS
#endif
#ifdef LOGGING_AND_ASSERTS

extern std::unique_ptr<GfxRenderEngine::Log> g_Logger;
#define LOG_CORE_TRACE(...) g_Logger->GetLogger().trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) g_Logger->GetLogger().info(__VA_ARGS__)
#define LOG_CORE_WARN(...) g_Logger->GetLogger().warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) g_Logger->GetLogger().error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...) g_Logger->GetLogger().critical(__VA_ARGS__)

#define LOG_APP_TRACE(...) g_Logger->GetAppLogger().trace(__VA_ARGS__)
#define LOG_APP_INFO(...) g_Logger->GetAppLogger().info(__VA_ARGS__)
#define LOG_APP_WARN(...) g_Logger->GetAppLogger().warn(__VA_ARGS__)
#define LOG_APP_ERROR(...) g_Logger->GetAppLogger().error(__VA_ARGS__)
#define LOG_APP_CRITICAL(...) g_Logger->GetAppLogger().critical(__VA_ARGS__)

#define CORE_ASSERT(x, str) \
    if (!(x))               \
    LOG_CORE_CRITICAL("ASSERT on line number {0} in file {1}: {2} (error)", __LINE__, __FILE__, str)

#define CORE_HARD_STOP(str)                                                                              \
    LOG_CORE_CRITICAL("hard stop on line number {0} in file {1}: {2} (error)", __LINE__, __FILE__, str); \
    std::cout << "terminating because of " << str << std::endl;                                          \
    exit(1)

#define APP_ASSERT(x, str) \
    if (!(x))              \
    LOG_APP_CRITICAL("ASSERT on line number {0} in file {1}: {2} (error)", __LINE__, __FILE__, str)

#else
#define CORE_ASSERT(x, str) \
    {                       \
    }
#define APP_ASSERT(x, str) \
    {                      \
    }
#define LOG_CORE_TRACE(...) \
    {                       \
    }
#define LOG_CORE_INFO(...) \
    {                      \
    }
#define LOG_CORE_WARN(...) \
    {                      \
    }
#define LOG_CORE_ERROR(...) \
    {                       \
    }
#define LOG_CORE_CRITICAL(...) \
    {                          \
    }
#define LOG_APP_TRACE(...) \
    {                      \
    }
#define LOG_APP_INFO(...) \
    {                     \
    }
#define LOG_APP_WARN(...) \
    {                     \
    }
#define LOG_APP_ERROR(...) \
    {                      \
    }
#define LOG_APP_CRITICAL(...) \
    {                         \
    }
#endif
