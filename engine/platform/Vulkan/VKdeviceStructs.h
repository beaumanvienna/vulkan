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

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "VKpool.h"

#include "auxiliary/threadPool.h"

namespace GfxRenderEngine
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    enum QueueTypes
    {
        GRAPHICS = 0,
        PRESENT,
        TRANSFER,
        NUMBER_OF_QUEUE_TYPES
    };

    struct QueueFamilyIndices
    {
        int m_GraphicsFamily{-1};
        int m_PresentFamily{-1};
        int m_TransferFamily{-1};
        int m_NumberOfQueues{0};
        std::vector<int> m_UniqueFamilyIndices;
        int m_QueueIndices[QueueTypes::NUMBER_OF_QUEUE_TYPES] = {};
        bool IsComplete() { return (m_GraphicsFamily >= 0) && (m_PresentFamily >= 0); }
    };
} // namespace GfxRenderEngine
