// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

/* Engine Copyright (c) 2025 Engine Development Team
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

#include <Renderer/VertexShader.h>

#include <vulkan/vulkan.h>
namespace JPH
{
    /// Vertex shader handle for Vulkan
    class VertexShaderVK : public VertexShader
    {
    public:
        /// Constructor
        VertexShaderVK(VkDevice inDevice, VkShaderModule inShaderModule) : mDevice(inDevice), mStageInfo()
        {
            mStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            mStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            mStageInfo.module = inShaderModule;
            mStageInfo.pName = "main";
        }

        /// Destructor
        virtual ~VertexShaderVK() override { vkDestroyShaderModule(mDevice, mStageInfo.module, nullptr); }

        VkDevice mDevice;
        VkPipelineShaderStageCreateInfo mStageInfo;
    };
} // namespace JPH