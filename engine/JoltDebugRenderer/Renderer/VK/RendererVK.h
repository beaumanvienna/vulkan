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

#include <JoltDebugRenderer/Renderer/Renderer.h>
#include <JoltDebugRenderer/Renderer/VK/ConstantBufferVK.h>
#include <JoltDebugRenderer/Renderer/VK/TextureVK.h>
#include <Jolt/Core/UnorderedMap.h>

#include <vulkan/vulkan.h>
#include "engine/platform/Vulkan/VKswapChain.h"

namespace JPH
{

    class RendererVK : public Renderer
    {
    public:
        struct PushConstants
        {
            glm::mat4 m_Projection;
            glm::mat4 m_View;
        };

    public:
        /// Destructor
        virtual ~RendererVK() override;

        // See: Renderer
        virtual void Initialize() override;
        virtual void BeginFrame(const CameraState& inCamera, float inWorldScale,
                                GfxRenderEngine::Camera const& cam0) override;
        virtual void EndFrame() override;
        virtual void SetProjectionMode() override;
        virtual void SetOrthoMode() override;
        virtual Ref<Texture> CreateTexture(const Surface* inSurface) override;
        virtual Ref<VertexShader> CreateVertexShader(const char* inName) override;
        virtual Ref<PixelShader> CreatePixelShader(const char* inName) override;
        virtual unique_ptr<PipelineState>
        CreatePipelineState(const VertexShader* inVertexShader, const PipelineState::EInputDescription* inInputDescription,
                            uint inInputDescriptionCount, const PixelShader* inPixelShader,
                            PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode,
                            PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest,
                            PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode,
                            std::string const& debugName) override;
        virtual RenderPrimitive* CreateRenderPrimitive(PipelineState::ETopology inType) override;
        virtual RenderInstances* CreateRenderInstances() override;
        virtual Texture* GetShadowMap() const override { return mShadowMap.GetPtr(); }
        VkDevice GetDevice() const { return mDevice; }
        VkDescriptorPool GetDescriptorPool() const { return mDescriptorPool; }
        VkDescriptorSetLayout GetDescriptorSetLayoutTexture() const { return mDescriptorSetLayoutTexture; }
        VkSampler GetTextureSamplerRepeat() const { return mTextureSamplerRepeat; }
        VkRenderPass GetRenderPass() const;
        VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
        VkCommandBuffer GetCommandBuffer();

        void AllocateMemory(VkDeviceSize inSize, uint32 inMemoryTypeBits, VkMemoryPropertyFlags inProperties,
                            VkDeviceMemory& outMemory);
        void FreeMemory(VkDeviceMemory inMemory, VkDeviceSize inSize);
        void CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties,
                          BufferVK& outBuffer);
        void CopyBuffer(VkBuffer inSrc, VkBuffer inDst, VkDeviceSize inSize);
        void CreateDeviceLocalBuffer(const void* inData, VkDeviceSize inSize, VkBufferUsageFlags inUsage,
                                     BufferVK& outBuffer);
        void FreeBuffer(BufferVK& ioBuffer);
        unique_ptr<ConstantBufferVK> CreateConstantBuffer(VkDeviceSize inBufferSize);
        void CreateImage(uint32 inWidth, uint32 inHeight, VkFormat inFormat, VkImageTiling inTiling,
                         VkImageUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkImage& outImage,
                         VkDeviceMemory& outMemory);
        void DestroyImage(VkImage inImage, VkDeviceMemory inMemory);
        VkImageView CreateImageView(VkImage inImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags);

    private:
        uint32 FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties);
        void FreeBufferInternal(BufferVK& ioBuffer);
        void UpdateViewPortAndScissorRect(uint32 inWidth, uint32 inHeight);

        VkInstance mInstance = VK_NULL_HANDLE;
#ifdef JPH_DEBUG
        VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
#endif

        VkDevice mDevice = VK_NULL_HANDLE;

        VkImage mDepthImage = VK_NULL_HANDLE;
        VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
        VkImageView mDepthImageView = VK_NULL_HANDLE;
        VkDescriptorSetLayout mDescriptorSetLayoutUBO = VK_NULL_HANDLE;
        VkDescriptorSetLayout mDescriptorSetLayoutTexture = VK_NULL_HANDLE;
        VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mDescriptorSets[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        VkDescriptorSet mDescriptorSetsOrtho[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        VkSampler mTextureSamplerRepeat = VK_NULL_HANDLE;
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
        uint32 mImageIndex = 0;
        Ref<TextureVK> mShadowMap;
        unique_ptr<ConstantBufferVK> mVertexShaderConstantBufferProjection[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        unique_ptr<ConstantBufferVK> mVertexShaderConstantBufferOrtho[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        unique_ptr<ConstantBufferVK> mPixelShaderConstantBuffer[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];

        struct Key
        {
            bool operator==(const Key& inRHS) const
            {
                return mSize == inRHS.mSize && mUsage == inRHS.mUsage && mProperties == inRHS.mProperties;
            }

            VkDeviceSize mSize;
            VkBufferUsageFlags mUsage;
            VkMemoryPropertyFlags mProperties;
        };

        JPH_MAKE_HASH_STRUCT(Key, KeyHasher, t.mSize, t.mUsage, t.mProperties)

        // We try to recycle buffers from frame to frame
        using BufferCache = UnorderedMap<Key, Array<BufferVK>, KeyHasher>;

        BufferCache mFreedBuffers[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];
        BufferCache mBufferCache;

        // Smaller allocations (from cMinAllocSize to cMaxAllocSize) will be done in blocks of cBlockSize bytes.
        // We do this because there is a limit to the number of allocations that we can make in Vulkan.
        static constexpr VkDeviceSize cMinAllocSize = 512;
        static constexpr VkDeviceSize cMaxAllocSize = 65536;
        static constexpr VkDeviceSize cBlockSize = 524288;

        JPH_MAKE_HASH_STRUCT(Key, MemKeyHasher, t.mUsage, t.mProperties, t.mSize)

        struct Memory
        {
            VkDeviceMemory mMemory;
            VkDeviceSize mOffset;
        };

        using MemoryCache = UnorderedMap<Key, Array<Memory>, KeyHasher>;

        MemoryCache mMemoryCache;
        uint32 mNumAllocations = 0;
        uint32 mMaxNumAllocations = 0;
        VkDeviceSize mTotalAllocated = 0;
        VkDeviceSize mMaxTotalAllocated = 0;
    };
} // namespace JPH