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

#pragma once

#include "engine.h"

#include "VKdevice.h"

namespace GfxRenderEngine
{
    class VK_Core
    {

    public:

        static std::shared_ptr<VK_Device> m_Device;

    };

    using MemoryFlags = uint;
    struct MemoryFlagBits {
        static inline constexpr MemoryFlags NONE = { 0x00000000 };
        static inline constexpr MemoryFlags DEDICATED_MEMORY = { 0x00000001 };
        static inline constexpr MemoryFlags CAN_ALIAS = { 0x00000200 };
        static inline constexpr MemoryFlags HOST_ACCESS_SEQUENTIAL_WRITE = { 0x00000400 };
        static inline constexpr MemoryFlags HOST_ACCESS_RANDOM = { 0x00000800 };
        static inline constexpr MemoryFlags STRATEGY_MIN_MEMORY = { 0x00010000 };
        static inline constexpr MemoryFlags STRATEGY_MIN_TIME = { 0x00020000 };
    };
}
