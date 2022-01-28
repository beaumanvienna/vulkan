/* Engine Copyright (c) 2022 Engine Development Team
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "engine.h"

#include "VKdevice.h"

namespace GfxRenderEngine
{
    class VK_DescriptorSetLayout
    {

    public:

        class Builder
        {

        public:

            Builder(VK_Device& device) : m_Device{device} {}

            Builder& AddBinding
                (
                    uint binding,
                    VkDescriptorType descriptorType,
                    VkShaderStageFlags stageFlags,
                    uint count = 1
                );

            std::unique_ptr<VK_DescriptorSetLayout> Build() const;

        private:

            VK_Device& m_Device;
            std::unordered_map<uint, VkDescriptorSetLayoutBinding> m_Bindings;

        };

    public:

        VK_DescriptorSetLayout(VK_Device& device, std::unordered_map<uint, VkDescriptorSetLayoutBinding> bindings);
        ~VK_DescriptorSetLayout();

        VK_DescriptorSetLayout(const VK_DescriptorSetLayout&) = delete;
        VK_DescriptorSetLayout &operator=(const VK_DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

    private:

        VK_Device& m_Device;

        // VkDescriptorSetLayout: opaque handle to a descriptor set layout object;
        // a descriptor set layout object is defined by an array of zero or more descriptor bindings
        VkDescriptorSetLayout m_DescriptorSetLayout;

        // VkDescriptorSetLayoutBinding: structure specifying a descriptor set layout binding
        // 
        //typedef struct VkDescriptorSetLayoutBinding
        //{
        //    uint32_t              binding; // binding number
        //    VkDescriptorType      descriptorType; // buffer or texture
        //    uint32_t              descriptorCount;
        //    VkShaderStageFlags    stageFlags;
        //    const VkSampler*      pImmutableSamplers;
        //} VkDescriptorSetLayoutBinding;
        std::unordered_map<uint, VkDescriptorSetLayoutBinding> m_Bindings;

        friend class VK_DescriptorWriter;
    };

    class VK_DescriptorPool
    {

    public:

        class Builder
        {

        public:
            Builder(VK_Device& m_Device) : m_Device{m_Device} {}

            Builder& AddPoolSize(VkDescriptorType descriptorType, uint count);
            Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& SetMaxSets(uint count);
            std::unique_ptr<VK_DescriptorPool> Build() const;

        private:

            VK_Device& m_Device;

            // VkDescriptorPoolSize: structure specifying descriptor pool size
            //typedef struct VkDescriptorPoolSize {
            //    VkDescriptorType    type;
            //    uint32_t            descriptorCount;
            //} VkDescriptorPoolSize;
            std::vector<VkDescriptorPoolSize> m_PoolSizes;

            uint m_MaxSets = 1000;
            VkDescriptorPoolCreateFlags m_PoolFlags = 0;
        };

    public:

        VK_DescriptorPool(VK_Device& m_Device, uint maxSets, VkDescriptorPoolCreateFlags poolFlags,
                        const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VK_DescriptorPool();
    
        VK_DescriptorPool(const VK_DescriptorPool&) = delete;
        VK_DescriptorPool& operator=(const VK_DescriptorPool&) = delete;
    
        bool AllocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout,
                                VkDescriptorSet& descriptor) const;
        void FreeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        void ResetPool();

    private:

        VK_Device& m_Device;
        VkDescriptorPool m_DescriptorPool;

        friend class VK_DescriptorWriter;
    };

    class VK_DescriptorWriter
    {

    public:

        VK_DescriptorWriter(VK_DescriptorSetLayout& setLayout, VK_DescriptorPool& pool);

        VK_DescriptorWriter& WriteBuffer(uint binding, VkDescriptorBufferInfo* bufferInfo);
        VK_DescriptorWriter& WriteImage(uint binding, VkDescriptorImageInfo* imageInfo);

        bool Build(VkDescriptorSet& set);
        void Overwrite(VkDescriptorSet& set);

    private:

        VK_DescriptorSetLayout& m_SetLayout;
        VK_DescriptorPool& m_Pool;
        std::vector<VkWriteDescriptorSet> m_Writes;
    };
}
