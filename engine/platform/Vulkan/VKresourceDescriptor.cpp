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

#include "core.h"

#include "VKrenderer.h"
#include "VKresourceDescriptor.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"
#include "VKcubemap.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{
    VK_ResourceDescriptor::VK_ResourceDescriptor(Resources::ResourceBuffers& buffers)
    {
        auto renderer = static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer());
        auto gDummyBuffer = renderer->gDummyBuffer;

        auto& instBuffer = buffers[Resources::INSTANCE_BUFFER_INDEX];
        auto& skelBuffer = buffers[Resources::SKELETAL_ANIMATION_BUFFER_INDEX];
        auto& hBuffer = buffers[Resources::HEIGHTMAP];
        auto& mPurposeBuffer = buffers[Resources::MULTI_PURPOSE_BUFFER];

        // instance buffer
        std::shared_ptr<Buffer>& instanceUbo = instBuffer ? instBuffer : gDummyBuffer;
        VK_Buffer* instanceBuffer = static_cast<VK_Buffer*>(instanceUbo.get());
        VkDescriptorBufferInfo instanceBufferInfo = instanceBuffer->DescriptorInfo();

        // joint/bone matrices
        std::shared_ptr<Buffer>& skeletalAnimationUbo = skelBuffer ? skelBuffer : gDummyBuffer;
        VK_Buffer* skeletalAnimationBuffer = static_cast<VK_Buffer*>(skeletalAnimationUbo.get());
        VkDescriptorBufferInfo skeletalAnimationBufferInfo = skeletalAnimationBuffer->DescriptorInfo();

        // height map
        std::shared_ptr<Buffer>& heightmapUbo = hBuffer ? hBuffer : gDummyBuffer;
        VK_Buffer* heightmapBuffer = static_cast<VK_Buffer*>(heightmapUbo.get());
        VkDescriptorBufferInfo heightmapBufferInfo = heightmapBuffer->DescriptorInfo();

        // multi purpose
        std::shared_ptr<Buffer>& multiPurposeUbo = mPurposeBuffer ? mPurposeBuffer : gDummyBuffer;
        VK_Buffer* multiPurposeBuffer = static_cast<VK_Buffer*>(multiPurposeUbo.get());
        VkDescriptorBufferInfo multiPurposeBufferInfo = multiPurposeBuffer->DescriptorInfo();

        {
            ResourceDescriptor::ResourceType resourceType{};
            if (buffers[Resources::HEIGHTMAP])
            {
                resourceType = ResourceDescriptor::ResourceType::RtGrass;
            }
            else if (buffers[Resources::SKELETAL_ANIMATION_BUFFER_INDEX])
            {
                resourceType = ResourceDescriptor::ResourceType::RtInstanceSA;
            }
            else if (buffers[Resources::INSTANCE_BUFFER_INDEX])
            {
                resourceType = ResourceDescriptor::ResourceType::RtInstance;
            }
            else
            {
                CORE_ASSERT(false, "resource type not supported");
                CORE_HARD_STOP("resource type was not found");
            }
            VK_DescriptorWriter descriptorWriter(GetResourceDescriptorSetLayout(resourceType));
            if (instBuffer || skelBuffer || hBuffer || mPurposeBuffer)
            {
                descriptorWriter.WriteBuffer(0, instanceBufferInfo);
            }
            if (skelBuffer || hBuffer || mPurposeBuffer)
            {
                descriptorWriter.WriteBuffer(1, skeletalAnimationBufferInfo);
            }
            if (hBuffer || mPurposeBuffer)
            {
                descriptorWriter.WriteBuffer(2, heightmapBufferInfo);
            }
            if (mPurposeBuffer)
            {
                descriptorWriter.WriteBuffer(3, multiPurposeBufferInfo);
            }
            {
                bool success = descriptorWriter.Build(m_DescriptorSet);
                CORE_ASSERT(success, "descriptor writer failed");
            }
        }
    }

    VK_ResourceDescriptor::VK_ResourceDescriptor(VK_ResourceDescriptor const& other)
    {
        m_DescriptorSet = other.m_DescriptorSet;
    }

    VK_ResourceDescriptor::VK_ResourceDescriptor(std::shared_ptr<ResourceDescriptor> const& resourceDescriptor)
    {
        if (resourceDescriptor)
        {
            VK_ResourceDescriptor* other = static_cast<VK_ResourceDescriptor*>(resourceDescriptor.get());

            m_DescriptorSet = other->m_DescriptorSet;
        }
    }

    VK_ResourceDescriptor::~VK_ResourceDescriptor() {}

    const VkDescriptorSet& VK_ResourceDescriptor::GetDescriptorSet() const { return m_DescriptorSet; }

    VK_DescriptorSetLayout&
    VK_ResourceDescriptor::GetResourceDescriptorSetLayout(ResourceDescriptor::ResourceType resourcelType)
    {
        auto renderer = static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer());
        return renderer->GetResourceDescriptorSetLayout(resourcelType);
    }

} // namespace GfxRenderEngine
