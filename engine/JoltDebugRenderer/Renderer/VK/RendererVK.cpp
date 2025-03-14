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

#include <TestFramework.h>

#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/RenderPrimitiveVK.h>
#include <Renderer/VK/RenderInstancesVK.h>
#include <Renderer/VK/PipelineStateVK.h>
#include <Renderer/VK/VertexShaderVK.h>
#include <Renderer/VK/PixelShaderVK.h>
#include <Renderer/VK/TextureVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
#include <Utils/Log.h>
#include <Utils/ReadData.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/QuickSort.h>
#include <Jolt/Core/RTTI.h>

#include "VKcore.h"
#include "VKrenderer.h"
#include "core.h"

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#ifdef JPH_PLATFORM_WINDOWS
#include <vulkan/vulkan_win32.h>
#include <Window/ApplicationWindowWin.h>
#elif defined(JPH_PLATFORM_LINUX)
#elif defined(JPH_PLATFORM_MACOS)
#include <vulkan/vulkan_metal.h>
#include <Window/ApplicationWindowMacOS.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

namespace JPH
{
    RendererVK::~RendererVK()
    {
        vkDeviceWaitIdle(mDevice);

        // Trace allocation stats
        Trace("VK: Max allocations: %u, max size: %u MB", mMaxNumAllocations, uint32(mMaxTotalAllocated >> 20));

        // Destroy the shadow map
        mShadowMap = nullptr;

        // Release constant buffers
        for (unique_ptr<ConstantBufferVK>& cb : mVertexShaderConstantBufferProjection)
        {
            cb = nullptr;
        }
        for (unique_ptr<ConstantBufferVK>& cb : mVertexShaderConstantBufferOrtho)
        {
            cb = nullptr;
        }
        for (unique_ptr<ConstantBufferVK>& cb : mPixelShaderConstantBuffer)
        {
            cb = nullptr;
        }

        // Free all buffers
        for (BufferCache& bc : mFreedBuffers)
        {
            for (BufferCache::value_type& vt : bc)
            {
                for (BufferVK& bvk : vt.second)
                {
                    FreeBufferInternal(bvk);
                }
            }
        }
        for (BufferCache::value_type& vt : mBufferCache)
        {
            for (BufferVK& bvk : vt.second)
            {
                FreeBufferInternal(bvk);
            }
        }

        // Free all blocks in the memory cache
        for (MemoryCache::value_type& mc : mMemoryCache)
        {
            for (Memory& m : mc.second)
            {
                if (m.mOffset == 0)
                {
                    vkFreeMemory(mDevice, m.mMemory, nullptr); // Don't care about memory tracking anymore
                }
            }
        }

        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

        vkDestroySampler(mDevice, mTextureSamplerRepeat, nullptr);

        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayoutUBO, nullptr);
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayoutTexture, nullptr);
    }

    void RendererVK::Initialize()
    {
        // Flip the sign of the projection matrix
        mPerspectiveYSign = -1.0f;
        mDevice = VK_Core::m_Device->Device();

        // Create constant buffer. One per frame to avoid overwriting the constant buffer while the GPU is still using it.
        for (uint n = 0; n < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; ++n)
        {
            mVertexShaderConstantBufferProjection[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
            mVertexShaderConstantBufferOrtho[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
            mPixelShaderConstantBuffer[n] = CreateConstantBuffer(sizeof(PixelShaderConstantBuffer));
        }

        // Create descriptor set layout for the uniform buffers
        VkDescriptorSetLayoutBinding ubo_layout_binding[2] = {};
        ubo_layout_binding[0].binding = 0;
        ubo_layout_binding[0].descriptorCount = 1;
        ubo_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding[1].binding = 1;
        ubo_layout_binding[1].descriptorCount = 1;
        ubo_layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo ubo_dsl = {};
        ubo_dsl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ubo_dsl.bindingCount = std::size(ubo_layout_binding);
        ubo_dsl.pBindings = ubo_layout_binding;
        FatalErrorIfFailed(vkCreateDescriptorSetLayout(mDevice, &ubo_dsl, nullptr, &mDescriptorSetLayoutUBO));

        // Create descriptor set layout for the texture binding
        VkDescriptorSetLayoutBinding texture_layout_binding = {};
        texture_layout_binding.binding = 0;
        texture_layout_binding.descriptorCount = 1;
        texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo texture_dsl = {};
        texture_dsl.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        texture_dsl.bindingCount = 1;
        texture_dsl.pBindings = &texture_layout_binding;
        FatalErrorIfFailed(vkCreateDescriptorSetLayout(mDevice, &texture_dsl, nullptr, &mDescriptorSetLayoutTexture));
        std::cout << "mDescriptorSetLayoutTexture: " << mDescriptorSetLayoutTexture << std::endl;
        // Create pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout = {};
        VkDescriptorSetLayout layout_handles[] = {mDescriptorSetLayoutUBO, mDescriptorSetLayoutTexture};
        pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout.setLayoutCount = std::size(layout_handles);
        pipeline_layout.pSetLayouts = layout_handles;
        pipeline_layout.pushConstantRangeCount = 1;
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 128;
        pipeline_layout.pPushConstantRanges = &pushConstantRange;
        FatalErrorIfFailed(vkCreatePipelineLayout(mDevice, &pipeline_layout, nullptr, &mPipelineLayout));
        std::cout << "mPipelineLayout: " << mPipelineLayout << std::endl;
        // Create descriptor pool
        VkDescriptorPoolSize descriptor_pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 128},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128},
        };
        VkDescriptorPoolCreateInfo descriptor_info = {};
        descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_info.poolSizeCount = std::size(descriptor_pool_sizes);
        descriptor_info.pPoolSizes = descriptor_pool_sizes;
        descriptor_info.maxSets = 256;
        FatalErrorIfFailed(vkCreateDescriptorPool(mDevice, &descriptor_info, nullptr, &mDescriptorPool));

        // Allocate descriptor sets for 3d rendering
        Array<VkDescriptorSetLayout> layouts(VK_SwapChain::MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayoutUBO);
        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
        descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_alloc_info.descriptorPool = mDescriptorPool;
        descriptor_set_alloc_info.descriptorSetCount = VK_SwapChain::MAX_FRAMES_IN_FLIGHT;
        descriptor_set_alloc_info.pSetLayouts = layouts.data();
        FatalErrorIfFailed(vkAllocateDescriptorSets(mDevice, &descriptor_set_alloc_info, mDescriptorSets));
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo vs_buffer_info = {};
            vs_buffer_info.buffer = mVertexShaderConstantBufferProjection[i]->GetBuffer();
            vs_buffer_info.range = sizeof(VertexShaderConstantBuffer);

            VkDescriptorBufferInfo ps_buffer_info = {};
            ps_buffer_info.buffer = mPixelShaderConstantBuffer[i]->GetBuffer();
            ps_buffer_info.range = sizeof(PixelShaderConstantBuffer);

            VkWriteDescriptorSet descriptor_write[2] = {};
            descriptor_write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write[0].dstSet = mDescriptorSets[i];
            descriptor_write[0].dstBinding = 0;
            descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write[0].descriptorCount = 1;
            descriptor_write[0].pBufferInfo = &vs_buffer_info;
            descriptor_write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write[1].dstSet = mDescriptorSets[i];
            descriptor_write[1].dstBinding = 1;
            descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write[1].descriptorCount = 1;
            descriptor_write[1].pBufferInfo = &ps_buffer_info;
            vkUpdateDescriptorSets(mDevice, 2, descriptor_write, 0, nullptr);
            std::cout << "mDescriptorSets[i]: " << mDescriptorSets[i] << std::endl;
        }

        // Allocate descriptor sets for 2d rendering
        FatalErrorIfFailed(vkAllocateDescriptorSets(mDevice, &descriptor_set_alloc_info, mDescriptorSetsOrtho));
        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorBufferInfo vs_buffer_info = {};
            vs_buffer_info.buffer = mVertexShaderConstantBufferOrtho[i]->GetBuffer();
            vs_buffer_info.range = sizeof(VertexShaderConstantBuffer);

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = mDescriptorSetsOrtho[i];
            descriptor_write.dstBinding = 0;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.descriptorCount = 1;
            descriptor_write.pBufferInfo = &vs_buffer_info;
            vkUpdateDescriptorSets(mDevice, 1, &descriptor_write, 0, nullptr);
        }

        // Create regular texture sampler
        VkSamplerCreateInfo sampler_info = {};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = VK_LOD_CLAMP_NONE;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        FatalErrorIfFailed(vkCreateSampler(mDevice, &sampler_info, nullptr, &mTextureSamplerRepeat));

        // Create dummy texture
        mShadowMap = new TextureVK(this, cShadowMapSize, cShadowMapSize);
    }

    void RendererVK::BeginFrame(const CameraState& inCamera, float inWorldScale, GfxRenderEngine::Camera const& cam0)
    {
        std::cout << "RendererVK::BeginFrame" << std::endl;
        JPH_PROFILE_FUNCTION();

        Renderer::BeginFrame(inCamera, inWorldScale, cam0);

        // Update frame index
        mFrameIndex = (mFrameIndex + 1) % VK_SwapChain::MAX_FRAMES_IN_FLIGHT;

        // Free buffers that weren't used this frame
        for (BufferCache::value_type& vt : mBufferCache)
        {
            for (BufferVK& bvk : vt.second)
            {
                FreeBufferInternal(bvk);
            }
        }
        mBufferCache.clear();

        // Recycle the buffers that were freed
        mBufferCache.swap(mFreedBuffers[mFrameIndex]);

        // Set constants for vertex shader in projection mode
        VertexShaderConstantBuffer* vs =
            mVertexShaderConstantBufferProjection[mFrameIndex]->Map<VertexShaderConstantBuffer>();
        *vs = mVSBuffer;
        mVertexShaderConstantBufferProjection[mFrameIndex]->Unmap();

        // Set constants for pixel shader
        PixelShaderConstantBuffer* ps = mPixelShaderConstantBuffer[mFrameIndex]->Map<PixelShaderConstantBuffer>();
        *ps = mPSBuffer;
        mPixelShaderConstantBuffer[mFrameIndex]->Unmap();

        // Switch to 3d projection mode
        SetProjectionMode();
    }

    void RendererVK::EndFrame()
    {
        JPH_PROFILE_FUNCTION();

        Renderer::EndFrame();
    }

    void RendererVK::SetProjectionMode()
    {
        JPH_ASSERT(mInFrame);

        // Bind descriptor set for 3d rendering
        std::cout << "RendererVK::SetProjectionMode() vkCmdBindDescriptorSets " << mDescriptorSets[mFrameIndex]
                  << ", frame index: " << mFrameIndex << " with pipeline layout " << mPipelineLayout << std::endl;
        vkCmdBindDescriptorSets(GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                                &mDescriptorSets[mFrameIndex], 0, nullptr);
    }

    void RendererVK::SetOrthoMode()
    {
        JPH_ASSERT(mInFrame);

        // Bind descriptor set for 2d rendering
        std::cout << "RendererVK::SetProjectionMode() vkCmdBindDescriptorSets" << mDescriptorSetsOrtho[mFrameIndex]
                  << ", frame index: " << mFrameIndex << " with pipeline layout " << mPipelineLayout << std::endl;
        vkCmdBindDescriptorSets(GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                                &mDescriptorSetsOrtho[mFrameIndex], 0, nullptr);
    }

    Ref<Texture> RendererVK::CreateTexture(const Surface* inSurface) { return new TextureVK(this, inSurface); }

    Ref<VertexShader> RendererVK::CreateVertexShader(const char* inName)
    {
        Array<uint8> data = ReadData((String("bin-int/") + inName + ".vert.spv").c_str());

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = data.size();
        create_info.pCode = reinterpret_cast<const uint32*>(data.data());
        VkShaderModule shader_module;
        FatalErrorIfFailed(vkCreateShaderModule(mDevice, &create_info, nullptr, &shader_module));

        return new VertexShaderVK(mDevice, shader_module);
    }

    Ref<PixelShader> RendererVK::CreatePixelShader(const char* inName)
    {
        Array<uint8> data = ReadData((String("bin-int/") + inName + ".frag.spv").c_str());

        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = data.size();
        create_info.pCode = reinterpret_cast<const uint32*>(data.data());
        VkShaderModule shader_module;
        FatalErrorIfFailed(vkCreateShaderModule(mDevice, &create_info, nullptr, &shader_module));

        return new PixelShaderVK(mDevice, shader_module);
    }

    unique_ptr<PipelineState> RendererVK::CreatePipelineState(
        const VertexShader* inVertexShader, const PipelineState::EInputDescription* inInputDescription,
        uint inInputDescriptionCount, const PixelShader* inPixelShader, PipelineState::EDrawPass inDrawPass,
        PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest,
        PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode, std::string const& debugName)
    {
        return make_unique<PipelineStateVK>(this, static_cast<const VertexShaderVK*>(inVertexShader), inInputDescription,
                                            inInputDescriptionCount, static_cast<const PixelShaderVK*>(inPixelShader),
                                            inDrawPass, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode,
                                            debugName);
    }

    RenderPrimitive* RendererVK::CreateRenderPrimitive(PipelineState::ETopology inType)
    {
        return new RenderPrimitiveVK(this);
    }

    RenderInstances* RendererVK::CreateRenderInstances() { return new RenderInstancesVK(this); }

    uint32 RendererVK::FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties)
    {
        return VK_Core::m_Device->FindMemoryType(inTypeFilter, inProperties);
    }

    void RendererVK::AllocateMemory(VkDeviceSize inSize, uint32 inMemoryTypeBits, VkMemoryPropertyFlags inProperties,
                                    VkDeviceMemory& outMemory)
    {
        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = inSize;
        alloc_info.memoryTypeIndex = FindMemoryType(inMemoryTypeBits, inProperties);
        FatalErrorIfFailed(vkAllocateMemory(mDevice, &alloc_info, nullptr, &outMemory));

        // Track allocation
        ++mNumAllocations;
        mTotalAllocated += inSize;

        // Track max usage
        mMaxTotalAllocated = max(mMaxTotalAllocated, mTotalAllocated);
        mMaxNumAllocations = max(mMaxNumAllocations, mNumAllocations);
    }

    void RendererVK::FreeMemory(VkDeviceMemory inMemory, VkDeviceSize inSize)
    {
        vkFreeMemory(mDevice, inMemory, nullptr);

        // Track free
        --mNumAllocations;
        mTotalAllocated -= inSize;
    }

    void RendererVK::CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties,
                                  BufferVK& outBuffer)
    {
        // Check the cache
        BufferCache::iterator i = mBufferCache.find({inSize, inUsage, inProperties});
        if (i != mBufferCache.end() && !i->second.empty())
        {
            outBuffer = i->second.back();
            i->second.pop_back();
            return;
        }

        // Create a new buffer
        outBuffer.mSize = inSize;
        outBuffer.mUsage = inUsage;
        outBuffer.mProperties = inProperties;

        VkBufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = inSize;
        create_info.usage = inUsage;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        FatalErrorIfFailed(vkCreateBuffer(mDevice, &create_info, nullptr, &outBuffer.mBuffer));

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(mDevice, outBuffer.mBuffer, &mem_requirements);

        if (mem_requirements.size > cMaxAllocSize)
        {
            // Allocate block directly
            AllocateMemory(mem_requirements.size, mem_requirements.memoryTypeBits, inProperties, outBuffer.mMemory);
            outBuffer.mAllocatedSize = mem_requirements.size;
            outBuffer.mOffset = 0;
        }
        else
        {
            // Round allocation to the next power of 2 so that we can use a simple block based allocator
            outBuffer.mAllocatedSize = max(VkDeviceSize(GetNextPowerOf2(uint32(mem_requirements.size))), cMinAllocSize);

            // Ensure that we have memory available from the right pool
            Array<Memory>& mem_array = mMemoryCache[{outBuffer.mAllocatedSize, outBuffer.mUsage, outBuffer.mProperties}];
            if (mem_array.empty())
            {
                // Allocate a bigger block
                VkDeviceMemory device_memory;
                AllocateMemory(cBlockSize, mem_requirements.memoryTypeBits, inProperties, device_memory);

                // Divide into sub blocks
                for (VkDeviceSize offset = 0; offset < cBlockSize; offset += outBuffer.mAllocatedSize)
                    mem_array.push_back({device_memory, offset});
            }

            // Claim memory from the pool
            Memory& memory = mem_array.back();
            outBuffer.mMemory = memory.mMemory;
            outBuffer.mOffset = memory.mOffset;
            mem_array.pop_back();
        }

        // Bind the memory to the buffer
        vkBindBufferMemory(mDevice, outBuffer.mBuffer, outBuffer.mMemory, outBuffer.mOffset);
    }

    void RendererVK::CopyBuffer(VkBuffer inSrc, VkBuffer inDst, VkDeviceSize inSize)
    {
        VK_Core::m_Device->CopyBuffer(inSrc, inDst, inSize);
    }

    void RendererVK::CreateDeviceLocalBuffer(const void* inData, VkDeviceSize inSize, VkBufferUsageFlags inUsage,
                                             BufferVK& outBuffer)
    {
        BufferVK staging_buffer;
        CreateBuffer(inSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer);

        void* data;
        vkMapMemory(mDevice, staging_buffer.mMemory, staging_buffer.mOffset, inSize, 0, &data);
        memcpy(data, inData, (size_t)inSize);
        vkUnmapMemory(mDevice, staging_buffer.mMemory);

        CreateBuffer(inSize, inUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuffer);

        CopyBuffer(staging_buffer.mBuffer, outBuffer.mBuffer, inSize);

        FreeBuffer(staging_buffer);
    }

    void RendererVK::FreeBuffer(BufferVK& ioBuffer)
    {
        if (ioBuffer.mBuffer != VK_NULL_HANDLE)
        {
            mFreedBuffers[mFrameIndex][{ioBuffer.mSize, ioBuffer.mUsage, ioBuffer.mProperties}].push_back(ioBuffer);
        }
    }

    void RendererVK::FreeBufferInternal(BufferVK& ioBuffer)
    {
        // Destroy the buffer
        vkDestroyBuffer(mDevice, ioBuffer.mBuffer, nullptr);
        ioBuffer.mBuffer = VK_NULL_HANDLE;

        if (ioBuffer.mAllocatedSize > cMaxAllocSize)
            FreeMemory(ioBuffer.mMemory, ioBuffer.mAllocatedSize);
        else
            mMemoryCache[{ioBuffer.mAllocatedSize, ioBuffer.mUsage, ioBuffer.mProperties}].push_back(
                {ioBuffer.mMemory, ioBuffer.mOffset});
        ioBuffer.mMemory = VK_NULL_HANDLE;
    }

    unique_ptr<ConstantBufferVK> RendererVK::CreateConstantBuffer(VkDeviceSize inBufferSize)
    {
        return make_unique<ConstantBufferVK>(this, inBufferSize);
    }

    VkImageView RendererVK::CreateImageView(VkImage inImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags)
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = inImage;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = inFormat;
        view_info.subresourceRange.aspectMask = inAspectFlags;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.layerCount = 1;

        VkImageView image_view;
        FatalErrorIfFailed(vkCreateImageView(mDevice, &view_info, nullptr, &image_view));

        return image_view;
    }

    void RendererVK::CreateImage(uint32 inWidth, uint32 inHeight, VkFormat inFormat, VkImageTiling inTiling,
                                 VkImageUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkImage& outImage,
                                 VkDeviceMemory& outMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = inWidth;
        imageInfo.extent.height = inHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = inFormat;
        imageInfo.tiling = inTiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = inUsage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        FatalErrorIfFailed(vkCreateImage(mDevice, &imageInfo, nullptr, &outImage));

        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(mDevice, outImage, &mem_requirements);

        AllocateMemory(mem_requirements.size, mem_requirements.memoryTypeBits, inProperties, outMemory);

        vkBindImageMemory(mDevice, outImage, outMemory, 0);
    }

    void RendererVK::DestroyImage(VkImage inImage, VkDeviceMemory inMemory)
    {
        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(mDevice, inImage, &mem_requirements);

        vkDestroyImage(mDevice, inImage, nullptr);

        FreeMemory(inMemory, mem_requirements.size);
    }

    void RendererVK::UpdateViewPortAndScissorRect(uint32 inWidth, uint32 inHeight)
    {
        VkCommandBuffer command_buffer = GetCommandBuffer();

        // Update the view port rect
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)inWidth;
        viewport.height = (float)inHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        // Update the scissor rect
        VkRect2D scissor = {};
        scissor.extent = {inWidth, inHeight};
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    VkCommandBuffer RendererVK::GetCommandBuffer()
    {
        JPH_ASSERT(mInFrame);
        return static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer())->GetCurrentCommandBuffer();
    }

    VkRenderPass RendererVK::GetRenderPass() const
    { //
        return static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer())->Get3DRenderPass();
    }

#ifdef JPH_ENABLE_VULKAN
    Renderer* Renderer::sCreate() { return new RendererVK; }
#endif
} // namespace JPH