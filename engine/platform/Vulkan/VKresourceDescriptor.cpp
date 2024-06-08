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

#include "VKresourceDescriptor.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"
#include "VKcubemap.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{
    VK_ResourceDescriptor::VK_ResourceDescriptor(Resources::ResourceBuffers& buffers)
    {

        // instance buffer
        std::shared_ptr<Buffer>& instanceUbo = buffers[Resources::INSTANCE_BUFFER_INDEX];
        VK_Buffer* instanceBuffer = nullptr;
        VkDescriptorBufferInfo instanceBufferInfo;
        if (instanceUbo)
        {
            instanceBuffer = static_cast<VK_Buffer*>(instanceUbo.get());
            instanceBufferInfo = instanceBuffer->DescriptorInfo();
        }

        // joint/bone matrices
        std::shared_ptr<Buffer>& skeletalAnimationUbo = buffers[Resources::SKELETAL_ANIMATION_BUFFER_INDEX];
        VK_Buffer* skeletalAnimationBuffer = nullptr;
        VkDescriptorBufferInfo skeletalAnimationBufferInfo;
        if (skeletalAnimationUbo)
        {
            skeletalAnimationBuffer = static_cast<VK_Buffer*>(skeletalAnimationUbo.get());
            skeletalAnimationBufferInfo = skeletalAnimationBuffer->DescriptorInfo();
        }

        {
            VK_DescriptorSetLayout::Builder builder{};
            if (instanceUbo)
            {
                builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
            }
            if (skeletalAnimationUbo)
            {
                builder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
            }

            if (builder.Size())
            {
                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = builder.Build();

                VK_DescriptorWriter descriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool);
                descriptorWriter.WriteBuffer(0, instanceBufferInfo);
                if (skeletalAnimationBuffer)
                {
                    descriptorWriter.WriteBuffer(1, skeletalAnimationBufferInfo);
                }
                descriptorWriter.Build(m_DescriptorSet);
            }
        }

        // shadows
        {
            VK_DescriptorSetLayout::Builder builder{};
            if (instanceUbo)
            {
                builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
            }
            if (skeletalAnimationUbo)
            {
                builder.AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
            }
            if(builder.Size())
            {
                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = builder.Build();

                VK_DescriptorWriter descriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool);
                descriptorWriter.WriteBuffer(0, instanceBufferInfo);
                if (skeletalAnimationBuffer)
                {
                    descriptorWriter.WriteBuffer(1, skeletalAnimationBufferInfo);
                }
                descriptorWriter.Build(m_ShadowDescriptorSet);
            }
        }
    }

    VK_ResourceDescriptor::VK_ResourceDescriptor(VK_ResourceDescriptor const& other)
    {
        m_DescriptorSet = other.m_DescriptorSet;
        m_ShadowDescriptorSet = other.m_ShadowDescriptorSet;
    }

    VK_ResourceDescriptor::VK_ResourceDescriptor(std::shared_ptr<ResourceDescriptor> const& resourceDescriptor)
    {
        if (resourceDescriptor)
        {
            VK_ResourceDescriptor* other = static_cast<VK_ResourceDescriptor*>(resourceDescriptor.get());

            m_DescriptorSet = other->m_DescriptorSet;
            m_ShadowDescriptorSet = other->m_ShadowDescriptorSet;
        }
    }

    VK_ResourceDescriptor::~VK_ResourceDescriptor() {}

    const VkDescriptorSet& VK_ResourceDescriptor::GetDescriptorSet() const { return m_DescriptorSet; }

    const VkDescriptorSet& VK_ResourceDescriptor::GetShadowDescriptorSet() const { return m_ShadowDescriptorSet; }
} // namespace GfxRenderEngine
