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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <iostream>
#include <chrono>

#include "log/log.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm.hpp"
#include "gtc/constants.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/transform.hpp"
#include "gtx/compatibility.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/hash.hpp"
#include "auxiliary/debug.h"

int engine(int argc, char* argv[]);

#undef far
#undef near
#undef CopyFile
#undef CreateDirectory
#undef CreateWindow

#define ASSERT(x) if (!(x)) std::cout << " (ASSERT on line number " << __LINE__ << " in file " << __FILE__ << ")" << std::endl;
#define MEMBER_SIZE(type, member) sizeof(type::member)
#define BIT(x) (1 << (x))

#define LOG_CORE_TRACE(...)     GfxRenderEngine::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...)      GfxRenderEngine::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...)      GfxRenderEngine::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...)     GfxRenderEngine::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CORE_CRITICAL(...)  GfxRenderEngine::Log::GetLogger()->critical(__VA_ARGS__)

#define LOG_APP_TRACE(...)      GfxRenderEngine::Log::GetAppLogger()->trace(__VA_ARGS__)
#define LOG_APP_INFO(...)       GfxRenderEngine::Log::GetAppLogger()->info(__VA_ARGS__)
#define LOG_APP_WARN(...)       GfxRenderEngine::Log::GetAppLogger()->warn(__VA_ARGS__)
#define LOG_APP_ERROR(...)      GfxRenderEngine::Log::GetAppLogger()->error(__VA_ARGS__)
#define LOG_APP_CRITICAL(...)   GfxRenderEngine::Log::GetAppLogger()->critical(__VA_ARGS__)

typedef uint8_t  uchar;
typedef uint16_t  uint16;
typedef uint32_t uint;
typedef int64_t  int64;
typedef uint64_t uint64;
typedef void (*GenericCallback) ();

using namespace std::chrono_literals;
using namespace GfxRenderEngine;

#define ENUM_CLASS_BITOPS(T) \
    static inline T operator |(const T &lhs, const T &rhs) { \
        return T((int)lhs | (int)rhs); \
    } \
    static inline T &operator |= (T &lhs, const T &rhs) { \
        lhs = lhs | rhs; \
        return lhs; \
    } \
    static inline bool operator &(const T &lhs, const T &rhs) { \
        return ((int)lhs & (int)rhs) != 0; \
    }
