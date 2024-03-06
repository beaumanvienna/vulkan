/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT
   This file is under the public domain.

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

#include <cstdlib>

namespace GfxRenderEngine
{

// #include <cstdint>
// #include <cstddef>
// #include "CommonTypes.h"

// #ifdef _WIN32
// #include <intrin.h>
// template <typename T>
// inline int CountSetBits(T v) {
//     // from https://graphics.stanford.edu/~seander/bithacks.html
//     // GCC has this built in, but MSVC's intrinsic will only emit the actual
//     // POPCNT instruction, which we're not depending on
//     v = v - ((v >> 1) & (T)~(T)0/3);
//     v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);
//     v = (v + (v >> 4)) & (T)~(T)0/255*15;
//     return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * 8;
// }
// inline int LeastSignificantSetBit(u32 val)
//{
//     unsigned long index;
//     _BitScanForward(&index, val);
//     return (int)index;
// }
// #if PPSSPP_ARCH(AMD64)
// inline int LeastSignificantSetBit(u64 val)
//{
//     unsigned long index;
//     _BitScanForward64(&index, val);
//     return (int)index;
// }
// #endif
// #else
// inline int CountSetBits(u32 val) { return __builtin_popcount(val); }
// inline int CountSetBits(u64 val) { return __builtin_popcountll(val); }
// inline int LeastSignificantSetBit(u32 val) { return __builtin_ctz(val); }
// inline int LeastSignificantSetBit(u64 val) { return __builtin_ctzll(val); }
// #endif

// Byteswapping
// Just in case this has been defined by platform
#undef swap16
#undef swap32
#undef swap64

#ifdef _WIN32
    inline uint16_t swap16(uint16_t _data) { return _byteswap_ushort(_data); }
    inline uint32_t swap32(uint32_t _data) { return _byteswap_ulong(_data); }
    inline uint64_t swap64(uint64_t _data) { return _byteswap_uint64(_data); }
#elif defined(__GNUC__)
    inline uint16_t swap16(uint16_t _data) { return __builtin_bswap16(_data); }
    inline uint32_t swap32(uint32_t _data) { return __builtin_bswap32(_data); }
    inline uint64_t swap64(uint64_t _data) { return __builtin_bswap64(_data); }
#else
    // Slow generic implementation. Hopefully this never hits
    inline uint16_t swap16(uint16_t data) { return (data >> 8) | (data << 8); }
    inline uint32_t swap32(uint32_t data) { return (swap16(data) << 16) | swap16(data >> 16); }
    inline uint64_t swap64(uint64_t data) { return ((uint64_t)swap32(data) << 32) | swap32(data >> 32); }
#endif

    inline uint16_t swap16(const uint8_t* _pData) { return swap16(*(const uint16_t*)_pData); }
    inline uint32_t swap32(const uint8_t* _pData) { return swap32(*(const uint32_t*)_pData); }
    inline uint64_t swap64(const uint8_t* _pData) { return swap64(*(const uint64_t*)_pData); }
} // namespace GfxRenderEngine
