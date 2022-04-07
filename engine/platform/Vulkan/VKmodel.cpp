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

#include "VKmodel.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"

namespace GfxRenderEngine
{

    std::vector<std::shared_ptr<VK_Texture>> VK_Model::m_Images;

    // Vertex
    std::vector<VkVertexInputBindingDescription> VK_Model::VK_Vertex::GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride  = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VK_Model::VK_Vertex::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_UV)});
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32_SINT, offsetof(Vertex, m_DiffuseMapTextureSlot)});
        attributeDescriptions.push_back({5, 0, VK_FORMAT_R32_SFLOAT, offsetof(Vertex, m_Amplification)});
        attributeDescriptions.push_back({6, 0, VK_FORMAT_R32_SINT, offsetof(Vertex, m_Unlit)});
        attributeDescriptions.push_back({7, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Tangent)});

        return attributeDescriptions;
    }

    // VK_Model
    VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const Builder& builder)
        : m_Device(device), m_HasIndexBuffer{false}
    {
        m_ImagesInternal = m_Images;
        CreateVertexBuffers(builder.m_Vertices);
        CreateIndexBuffers(builder.m_Indices);
    }

    void VK_Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<uint>(vertices.size());
        ASSERT(m_VertexCount >= 3); // at least one triangle
        VkDeviceSize bufferSize = sizeof(Vertex) * m_VertexCount;
        uint vertexSize = sizeof(vertices[0]);

        VK_Buffer stagingBuffer
        {
            *m_Device, vertexSize, m_VertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*) vertices.data());

        m_VertexBuffer = std::make_unique<VK_Buffer>
        (
            *m_Device, vertexSize, m_VertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_Device->CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
    }

    void VK_Model::CreateIndexBuffers(const std::vector<uint>& indices)
    {
        m_IndexCount = static_cast<uint>(indices.size());
        VkDeviceSize bufferSize = sizeof(uint) * m_IndexCount;
        m_HasIndexBuffer = ( m_IndexCount > 0);
        uint indexSize = sizeof(indices[0]);

        if (!m_HasIndexBuffer)
        {
            return;
        }

        VK_Buffer stagingBuffer
        {
            *m_Device, indexSize, m_IndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*) indices.data());

        m_IndexBuffer = std::make_unique<VK_Buffer>
        (
            *m_Device, indexSize, m_IndexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        m_Device->CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);

    }

    void VK_Model::Bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {m_VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_HasIndexBuffer)
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void VK_Model::Draw(VkCommandBuffer commandBuffer)
    {
        if (m_HasIndexBuffer)
        {
            vkCmdDrawIndexed
            (
                commandBuffer,      // VkCommandBuffer commandBuffer
                m_IndexCount,       // uint32_t        indexCount
                1,                  // uint32_t        instanceCount
                0,                  // uint32_t        firstIndex
                0,                  // int32_t         vertexOffset
                0                   // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw
            (
                commandBuffer,      // VkCommandBuffer commandBuffer
                m_VertexCount,      // uint32_t        vertexCount
                1,                  // uint32_t        instanceCount
                0,                  // uint32_t        firstVertex
                0                   // uint32_t        firstInstance
            );
        }
    }

    PbrDiffuseComponent VK_Model::CreateDescriptorSet(const std::shared_ptr<VK_Texture>& colorMap)
    {
        PbrDiffuseComponent pbrDiffuseComponent{};
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        auto texture = colorMap.get();
        VkDescriptorImageInfo imageInfo {};
        imageInfo.sampler     = texture->m_Sampler;
        imageInfo.imageView   = texture->m_TextureView;
        imageInfo.imageLayout = texture->m_ImageLayout;

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                .WriteImage(0, &imageInfo)
                .Build(pbrDiffuseComponent.m_DescriptorSet[i]);
        }
        return pbrDiffuseComponent;
    }

    PbrDiffuseNormalComponent VK_Model::CreateDescriptorSet(const std::shared_ptr<VK_Texture>& colorMap, const std::shared_ptr<VK_Texture>& normalMap)
    {
        PbrDiffuseNormalComponent pbrDiffuseNormalComponent{};
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        VkDescriptorImageInfo imageInfo0 {};
        {
            auto texture = colorMap.get();
            imageInfo0.sampler     = texture->m_Sampler;
            imageInfo0.imageView   = texture->m_TextureView;
            imageInfo0.imageLayout = texture->m_ImageLayout;
        }

        VkDescriptorImageInfo imageInfo1 {};
        {
            auto texture = normalMap.get();
            imageInfo1.sampler     = texture->m_Sampler;
            imageInfo1.imageView   = texture->m_TextureView;
            imageInfo1.imageLayout = texture->m_ImageLayout;
        }

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                .WriteImage(0, &imageInfo0)
                .WriteImage(1, &imageInfo1)
                .Build(pbrDiffuseNormalComponent.m_DescriptorSet[i]);
        }
        return pbrDiffuseNormalComponent;
    }

    PbrDiffuseNormalRoughnessMetallicComponent VK_Model::CreateDescriptorSet
    (
        const std::shared_ptr<VK_Texture>& colorMap,
        const std::shared_ptr<VK_Texture>& normalMap, 
        const std::shared_ptr<VK_Texture>& roughnessMetallicMap)
    {
        PbrDiffuseNormalRoughnessMetallicComponent pbrDiffuseNormalRoughnessMetallicComponent{};
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        VkDescriptorImageInfo imageInfo0 {};
        {
            auto texture = colorMap.get();
            imageInfo0.sampler     = texture->m_Sampler;
            imageInfo0.imageView   = texture->m_TextureView;
            imageInfo0.imageLayout = texture->m_ImageLayout;
        }

        VkDescriptorImageInfo imageInfo1 {};
        {
            auto texture = normalMap.get();
            imageInfo1.sampler     = texture->m_Sampler;
            imageInfo1.imageView   = texture->m_TextureView;
            imageInfo1.imageLayout = texture->m_ImageLayout;
        }

        VkDescriptorImageInfo imageInfo2 {};
        {
            auto texture = roughnessMetallicMap.get();
            imageInfo2.sampler     = texture->m_Sampler;
            imageInfo2.imageView   = texture->m_TextureView;
            imageInfo2.imageLayout = texture->m_ImageLayout;
        }

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                .WriteImage(0, &imageInfo0)
                .WriteImage(1, &imageInfo1)
                .WriteImage(2, &imageInfo2)
                .Build(pbrDiffuseNormalRoughnessMetallicComponent.m_DescriptorSet[i]);
        }
        return pbrDiffuseNormalRoughnessMetallicComponent;
    }


    void VK_Model::CreateDescriptorSet
    (
        const std::shared_ptr<VK_Texture>& colorMap,
        const std::shared_ptr<VK_Texture>& roughnessMetallicMap,
        PbrDiffuseRoughnessMetallicComponent& pbrDiffuseRoughnessMetallicComponent)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        VkDescriptorImageInfo imageInfo0 {};
        {
            auto texture = colorMap.get();
            imageInfo0.sampler     = texture->m_Sampler;
            imageInfo0.imageView   = texture->m_TextureView;
            imageInfo0.imageLayout = texture->m_ImageLayout;
        }

        VkDescriptorImageInfo imageInfo1 {};
        {
            auto texture = roughnessMetallicMap.get();
            imageInfo1.sampler     = texture->m_Sampler;
            imageInfo1.imageView   = texture->m_TextureView;
            imageInfo1.imageLayout = texture->m_ImageLayout;
        }

        for (uint i = 0; i < VK_SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                .WriteImage(0, &imageInfo0)
                .WriteImage(1, &imageInfo1)
                .Build(pbrDiffuseRoughnessMetallicComponent.m_DescriptorSet[i]);
        }
    }
}
