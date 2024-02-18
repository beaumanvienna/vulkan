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
#include "VKtypes.h"
#include <vulkan/vulkan.h>
#include <vma.h>

namespace GfxRenderEngine
{
    static inline constexpr uint CONSTANT_BUFFER_BINDING_COUNT = 8;

    enum struct ImageViewType {
        REGULAR_1D = 0,
        REGULAR_2D = 1,
        REGULAR_3D = 2,
        CUBE = 3,
        REGULAR_1D_ARRAY = 4,
        REGULAR_2D_ARRAY = 5,
        CUBE_ARRAY = 6,
        MAX_ENUM = 0x7fffffff,
    };

    struct GPUResourceId {
        uint index : 24 = {};
        uint version : 8 = {};

        auto isEmpty() const -> bool;

        constexpr auto operator<=>(const GPUResourceId& other) const {
            return std::bit_cast<uint>(*this) <=> std::bit_cast<uint>(other);
        }
        constexpr bool operator==(const GPUResourceId& other) const = default;
        constexpr bool operator!=(const GPUResourceId& other) const = default;
        constexpr bool operator<(const GPUResourceId& other) const = default;
        constexpr bool operator>(const GPUResourceId& other) const = default;
        constexpr bool operator<=(const GPUResourceId& other) const = default;
        constexpr bool operator>=(const GPUResourceId& other) const = default;
    };

    struct BufferId : public GPUResourceId {};

    struct ImageViewId : public GPUResourceId {};

    template <ImageViewType VIEW_TYPE>
    struct TypedImageViewId : public ImageViewId {
        static constexpr inline auto viewType() -> ImageViewType { return VIEW_TYPE; }
    };

    struct ImageId : public GPUResourceId {
        auto defaultView() const->ImageViewId;
    };

    struct SamplerId : public GPUResourceId {};

    struct BufferInfo {
        uint size = {};
        MemoryFlags memoryFlags = {};
        std::string name = {};
    };

    using ImageCreateFlags = uint;
    struct ImageCreateFlagBits {
        static inline constexpr ImageCreateFlags NONE = { 0x00000000 };
        static inline constexpr ImageCreateFlags ALLOW_MUTABLE_FORMAT = { 0x00000008 };
        static inline constexpr ImageCreateFlags COMPATIBLE_CUBE = { 0x00000010 };
        static inline constexpr ImageCreateFlags COMPATIBLE_2D_ARRAY = { 0x00000020 };
        static inline constexpr ImageCreateFlags ALLOW_ALIAS = { 0x00000400 };
    };

    struct ImageInfo {
        ImageCreateFlags flags = ImageCreateFlagBits::NONE;
        uint dimensions = 2;
        Format format = Format::R8G8B8A8_UNORM;
        Extent3D size = { 0, 0, 0 };
        uint mipLevelCount = 1;
        uint arrayLayerCount = 1;
        uint sampleCount = 1;
        ImageUsageFlags usage = {};
        MemoryFlags memoryFlags = MemoryFlagBits::DEDICATED_MEMORY;
        std::string name = {};
    };

    struct ImageViewInfo {
        ImageViewType type = ImageViewType::REGULAR_2D;
        Format format = Format::R8G8B8A8_UNORM;
        ImageId image = {};
        ImageMipArraySlice slice = {};
        std::string name = {};
    };

    struct SamplerInfo {
        Filter magnificationFilter = Filter::LINEAR;
        Filter minificationFilter = Filter::LINEAR;
        Filter mipmapFilter = Filter::LINEAR;
        ReductionMode reductionMode = ReductionMode::WEIGHTED_AVERAGE;
        SamplerAddressMode addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
        SamplerAddressMode addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
        float mipLodBias = 0.5f;
        bool enableAnisotropy = false;
        float maxAnisotropy = 0.0f;
        bool enableCompare = false;
        CompareOp compareOp = CompareOp::ALWAYS;
        float minLod = 0.0f;
        float maxLod = 1000.0f;
        BorderColor borderColor = BorderColor::FLOAT_TRANSPARENT_BLACK;
        bool enableUnnormalizedCoordinates = false;
        std::string name = {};
    };

    static inline constexpr uint STORAGE_IMAGE_BINDING = 0;
    static inline constexpr uint SAMPLED_IMAGE_BINDING = 1;
    static inline constexpr uint SAMPLER_BINDING = 2;

    struct ImplBufferSlot {
        BufferInfo info = {};
        VkBuffer vkBuffer = {};
        VmaAllocation vmaAllocation = {};
        VkDeviceAddress deviceAddress = {};
        void* hostAddress = {};
        bool zombie = {};
    };

    static inline constexpr int NOT_OWNED_BY_SWAPCHAIN = -1;

    struct ImplImageViewSlot {
        ImageViewInfo info = {};
        VkImageView vkImageView = {};
    };

    struct ImplImageSlot {
        ImplImageViewSlot viewSlot = {};
        ImageInfo info = {};
        VkImage vkImage = {};
        VmaAllocation vmaAllocation = {};
        int swapchainImageIndex = NOT_OWNED_BY_SWAPCHAIN;
        VkImageAspectFlags aspectFlags = {};
        bool zombie = {};
    };

    struct ImplSamplerSlot {
        SamplerInfo info = {};
        VkSampler vkSampler = {};
        bool zombie = {};
    };

    template <typename ResourceT, uint MAX_RESOURCE_COUNT = 1u << 20u>
    struct GpuShaderResourcePool {
        static constexpr inline uint PAGE_BITS = 12u;
        static constexpr inline uint PAGE_SIZE = 1u << PAGE_BITS;
        static constexpr inline uint PAGE_MASK = PAGE_SIZE - 1u;
        static constexpr inline uint PAGE_COUNT = MAX_RESOURCE_COUNT / PAGE_SIZE;

        using PageT = std::array<std::pair<ResourceT, uchar>, PAGE_SIZE>;

        std::vector<uint> m_FreeIndexStack = {};
        uint m_NextIndex = {};
        uint m_MaxResources = {};

        std::array<std::unique_ptr<PageT>, PAGE_COUNT> m_Pages = {};

        auto NewSlot() -> std::pair<GPUResourceId, ResourceT&> {
            uint index;
            if (m_FreeIndexStack.empty()) {
                index = m_NextIndex++;
            }
            else {
                index = m_FreeIndexStack.back();
                m_FreeIndexStack.pop_back();
            }

            uint page = index >> PAGE_BITS;
            uint offset = index & PAGE_MASK;

            if (!m_Pages[page]) {
                m_Pages[page] = std::make_unique<PageT>();
                for (uint i = 0; i < PAGE_SIZE; ++i) {
                    m_Pages[page]->at(i).second = 0;
                }
            }

            m_Pages[page]->at(offset).second = std::max<uchar>(m_Pages[page]->at(offset).second, 1);

            uchar version = m_Pages[page]->at(offset).second;

            return { GPUResourceId{.index = index, .version = version}, m_Pages[page]->at(offset).first };
        }

        auto ReturnSlot(GPUResourceId id) {
            uint page = id.index >> PAGE_BITS;
            uint offset = id.index & PAGE_MASK;

            m_Pages[page]->at(offset).second = std::max<uchar>(m_Pages[page]->at(offset).second + 1, 1);

            m_FreeIndexStack.push_back(id.index);
        }

        auto IsIdValid(GPUResourceId id) const -> bool {
            uint page = id.index >> PAGE_BITS;
            uint offset = id.index & PAGE_MASK;

            if (!(page < m_Pages.size()) || !(m_Pages[page] != nullptr) || !(id.version != 0)) {
                return false;
            }
            uchar version = m_Pages[page]->at(offset).second;
            if (!(version == id.version) || m_Pages[page]->at(offset).first.zombie) {
                return false;
            }
            return true;
        }

        auto DereferenceId(GPUResourceId id) -> ResourceT& {
            uint page = id.index >> PAGE_BITS;
            uint offset = id.index & PAGE_MASK;

            return m_Pages[page]->at(offset).first;
        }

        auto DereferenceId(GPUResourceId id) const -> const ResourceT& {
            uint page = id.index >> PAGE_BITS;
            uint offset = id.index & PAGE_MASK;

            return m_Pages[page]->at(offset).first;
        }
    };

    struct GPUShaderResourceTable {
        GpuShaderResourcePool<ImplBufferSlot> m_BufferSlots = {};
        GpuShaderResourcePool<ImplImageSlot> m_ImageSlots = {};
        GpuShaderResourcePool<ImplSamplerSlot> m_SamplerSlots = {};

        VkDescriptorSetLayout m_VkDescriptorSetLayout = {};
        VkDescriptorSetLayout m_UniformBufferDescriptorSetLayout = {};
        VkDescriptorSet m_VkDescriptorSet = {};
        VkDescriptorPool m_VkDescriptorPool = {};
        VkDevice m_VkDevice = {};

        GPUShaderResourceTable(uint maxBuffers, uint maxImages, uint maxSamplers, VkDevice device, bool enableDebugNames);
        ~GPUShaderResourceTable();

        void WriteDescriptorSetSampler(VkSampler vkSampler, uint index);
        void WriteDescriptorSetImage(VkImageView vkImageView, ImageUsageFlags usage, uint index);
    };

}
