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

#include "core.h"
#include "VKmodel.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"

#include "systems/VKpbrNoMapSys.h"
#include "systems/VKpbrDiffuseSys.h"
#include "systems/VKpbrEmissiveSys.h"
#include "systems/VKpbrDiffuseNormalSys.h"
#include "systems/VKpbrEmissiveTextureSys.h"
#include "systems/VKpbrDiffuseNormalRoughnessMetallicSys.h"
#include "systems/VKshadowRenderSys.h"

namespace GfxRenderEngine
{
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
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32_SFLOAT, offsetof(Vertex, m_Amplification)});
        attributeDescriptions.push_back({5, 0, VK_FORMAT_R32_SINT, offsetof(Vertex, m_Unlit)});
        attributeDescriptions.push_back({6, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Tangent)});
        attributeDescriptions.push_back({7, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, m_JointIds)});
        attributeDescriptions.push_back({8, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, m_Weights)});

        return attributeDescriptions;
    }

    // VK_Model
    VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const Builder& builder)
        : m_Device(device), m_HasIndexBuffer{false}
    {
        m_Images = std::move(builder.m_Images);
        m_Cubemaps = std::move(builder.m_Cubemaps);

        m_PrimitivesNoMap = std::move(builder.m_PrimitivesNoMap);
        m_PrimitivesEmissive = std::move(builder.m_PrimitivesEmissive);
        m_PrimitivesDiffuseMap = std::move(builder.m_PrimitivesDiffuseMap);
        m_PrimitivesDiffuseSAMap = std::move(builder.m_PrimitivesDiffuseSAMap);
        m_PrimitivesEmissiveTexture = std::move(builder.m_PrimitivesEmissiveTexture);
        m_PrimitivesDiffuseNormalMap = std::move(builder.m_PrimitivesDiffuseNormalMap);
        m_PrimitivesDiffuseNormalRoughnessMetallicMap = std::move(builder.m_PrimitivesDiffuseNormalRoughnessMetallicMap);
        m_PrimitivesCubemap = std::move(builder.m_PrimitivesCubemap);

        m_Skeletons  = std::move(builder.m_Skeletons);
        m_Animations = std::move(builder.m_Animations);
        m_ShaderDataUbo = builder.m_ShaderData;

        CreateVertexBuffers(std::move(builder.m_Vertices));
        CreateIndexBuffers(std::move(builder.m_Indices));
    }

    VK_Model::~VK_Model() {}


    void VK_Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = static_cast<uint>(vertices.size());
        ASSERT(m_VertexCount >= 3); // at least one triangle
        VkDeviceSize bufferSize = sizeof(Vertex) * m_VertexCount;
        uint vertexSize = sizeof(vertices[0]);

        VK_Buffer stagingBuffer
        {
            vertexSize, m_VertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*) vertices.data());

        m_VertexBuffer = std::make_unique<VK_Buffer>
        (
            vertexSize, m_VertexCount,
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
            indexSize, m_IndexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*) indices.data());

        m_IndexBuffer = std::make_unique<VK_Buffer>
        (
            indexSize, m_IndexCount,
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

    void VK_Model::UpdateAnimation()
    {
        size_t lastAnimation = m_Animations.size() - 1;

        m_Animations[lastAnimation]->Update(m_Skeletons[0]);
        m_Skeletons[0].Update();

        // update ubo
        static_cast<VK_Buffer*>(m_ShaderDataUbo.get())->WriteToBuffer(m_Skeletons[0].m_ShaderData.m_FinalJointsMatrices.data());
        static_cast<VK_Buffer*>(m_ShaderDataUbo.get())->Flush();
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

    void VK_Model::DrawNoMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesNoMap)
        {

            VK_PushConstantDataPbrNoMap push{};

            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrNoMapMaterial.m_Roughness;
            push.m_NormalMatrix[3].y = primitive.m_PbrNoMapMaterial.m_Metallic;

            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrNoMap),
                &push);

            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawEmissive(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& primitive : m_PrimitivesEmissive)
        {

            VK_PushConstantDataPbrEmissive push{};

            if (emissiveStrength)
            {
                primitive.m_PbrEmissiveMaterial.m_EmissiveStrength = emissiveStrength;
            }

            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrEmissiveMaterial.m_EmissiveStrength;
            push.m_NormalMatrix[3].y = 0;

            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrEmissive),
                &push);

            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawEmissiveTexture(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& primitive : m_PrimitivesEmissiveTexture)
        {
            VkDescriptorSet localDescriptorSet = primitive.m_PbrEmissiveTextureMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );

            if (emissiveStrength)
            {
                primitive.m_PbrEmissiveTextureMaterial.m_EmissiveStrength = emissiveStrength;
            }

            VK_PushConstantDataPbrEmissiveTexture push{};
            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrEmissiveTextureMaterial.m_EmissiveStrength;
            push.m_NormalMatrix[3].y = 0;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrEmissiveTexture),
                &push);
            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawDiffuseMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesDiffuseMap)
        {
            VkDescriptorSet localDescriptorSet = primitive.m_PbrDiffuseMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );
            VK_PushConstantDataPbrDiffuse push{};
            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrDiffuseMaterial.m_Roughness;
            push.m_NormalMatrix[3].y = primitive.m_PbrDiffuseMaterial.m_Metallic;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrDiffuse),
                &push);
            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawDiffuseSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesDiffuseSAMap)
        {
            VkDescriptorSet localDescriptorSet = primitive.m_PbrDiffuseSAMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );
            VK_PushConstantDataPbrDiffuseSA push{};
            push.m_ModelMatrix  = transform.GetMat4();

            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrDiffuseSAMaterial.m_Roughness;
            push.m_NormalMatrix[3].y = primitive.m_PbrDiffuseSAMaterial.m_Metallic;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrDiffuse),
                &push);

            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawDiffuseNormalMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesDiffuseNormalMap)
        {
            VkDescriptorSet localDescriptorSet = primitive.m_PbrDiffuseNormalMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );
            VK_PushConstantDataPbrDiffuseNormal push{};
            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].x = primitive.m_PbrDiffuseNormalMaterial.m_Roughness;
            push.m_NormalMatrix[3].y = primitive.m_PbrDiffuseNormalMaterial.m_Metallic;
            push.m_NormalMatrix[3].z = m_NormalMapIntensity;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrDiffuseNormal),
                &push);

            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallicMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesDiffuseNormalRoughnessMetallicMap)
        {
            VK_PushConstantDataPbrDiffuseNormalRoughnessMetallic push{};
            push.m_ModelMatrix  = transform.GetMat4();
            push.m_NormalMatrix = transform.GetNormalMatrix();
            push.m_NormalMatrix[3].z = m_NormalMapIntensity;
            vkCmdPushConstants(
                frameInfo.m_CommandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(VK_PushConstantDataPbrDiffuseNormalRoughnessMetallic),
                &push);

            VkDescriptorSet localDescriptorSet = primitive.m_PbrDiffuseNormalRoughnessMetallicMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );
            if(m_HasIndexBuffer)
            {
                vkCmdDrawIndexed
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_IndexCount,     // uint32_t        indexCount
                    1,                          // uint32_t        instanceCount
                    primitive.m_FirstIndex,     // uint32_t        firstIndex
                    primitive.m_FirstVertex,    // int32_t         vertexOffset
                    0                           // uint32_t        firstInstance
                );
            }
            else
            {
                vkCmdDraw
                (
                    frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                    primitive.m_VertexCount,    // uint32_t        vertexCount
                    1,                          // uint32_t        instanceCount
                    0,                          // uint32_t        firstVertex
                    0                           // uint32_t        firstInstance
                );
            }
        }
    }

    void VK_Model::DrawShadowInternal(const VK_FrameInfo& frameInfo, TransformComponent& transform, const PrimitiveTmp& primitive)
    {
        if(m_HasIndexBuffer)
        {
            vkCmdDrawIndexed
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                primitive.m_IndexCount,     // uint32_t        indexCount
                1,                          // uint32_t        instanceCount
                primitive.m_FirstIndex,     // uint32_t        firstIndex
                primitive.m_FirstVertex,    // int32_t         vertexOffset
                0                           // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                primitive.m_VertexCount,    // uint32_t        vertexCount
                1,                          // uint32_t        instanceCount
                0,                          // uint32_t        firstVertex
                0                           // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawAnimatedShadowInternal(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, const PrimitiveDiffuseSAMap& primitive, VkDescriptorSet shadowDescriptorSet)
    {
        VkDescriptorSet localDescriptorSet = primitive.m_PbrDiffuseSAMaterial.m_DescriptorSet;
        std::vector<VkDescriptorSet> descriptorSets = {shadowDescriptorSet, localDescriptorSet};
        vkCmdBindDescriptorSets
        (
            frameInfo.m_CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            2,
            descriptorSets.data(),
            0,
            nullptr
        );
        if(m_HasIndexBuffer)
        {
            vkCmdDrawIndexed
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                primitive.m_IndexCount,     // uint32_t        indexCount
                1,                          // uint32_t        instanceCount
                primitive.m_FirstIndex,     // uint32_t        firstIndex
                primitive.m_FirstVertex,    // int32_t         vertexOffset
                0                           // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                primitive.m_VertexCount,    // uint32_t        vertexCount
                1,                          // uint32_t        instanceCount
                0,                          // uint32_t        firstVertex
                0                           // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawShadow(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesNoMap)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
        for(auto& primitive : m_PrimitivesEmissive)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
        for(auto& primitive : m_PrimitivesEmissiveTexture)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
        for(auto& primitive : m_PrimitivesDiffuseMap)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
        for(auto& primitive : m_PrimitivesDiffuseNormalMap)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
        for(auto& primitive : m_PrimitivesDiffuseNormalRoughnessMetallicMap)
        {
            PrimitiveTmp primitiveTmp {primitive.m_FirstIndex, primitive.m_FirstVertex, primitive.m_IndexCount, primitive.m_VertexCount};
            DrawShadowInternal(frameInfo, transform, primitiveTmp);
        }
    }

    void VK_Model::DrawShadowAnimated(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, VkDescriptorSet shadowDescriptorSet)
    {
        for(auto& primitive : m_PrimitivesDiffuseSAMap)
        {
            DrawAnimatedShadowInternal(frameInfo, transform, pipelineLayout, primitive, shadowDescriptorSet);
        }
    }

    void VK_Model::DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& primitive : m_PrimitivesCubemap)
        {
            VkDescriptorSet localDescriptorSet = primitive.m_CubemapMaterial.m_DescriptorSet;
            std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, localDescriptorSet};
            vkCmdBindDescriptorSets
            (
                frameInfo.m_CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                2,
                descriptorSets.data(),
                0,
                nullptr
            );

            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                primitive.m_VertexCount,    // uint32_t        vertexCount
                1,                          // uint32_t        instanceCount
                primitive.m_FirstVertex,    // uint32_t        firstVertex
                0                           // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::CreateDescriptorSet(PbrDiffuseMaterial& pbrDiffuseMaterial,
                                       const std::shared_ptr<Texture>& colorMap)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        const auto& imageInfo = static_cast<VK_Texture*>(colorMap.get())->GetDescriptorImageInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, imageInfo)
            .Build(pbrDiffuseMaterial.m_DescriptorSet);
    }

    void VK_Model::CreateDescriptorSet(PbrDiffuseSAMaterial& pbrDiffuseSAMaterial,
                                       const std::shared_ptr<Texture>& colorMap,
                                       const std::shared_ptr<Buffer>& skeletalAnimationUBO)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        const auto& imageInfo = static_cast<VK_Texture*>(colorMap.get())->GetDescriptorImageInfo();

        auto buffer = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
        buffer->Map();
        VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, imageInfo)
            .WriteBuffer(1, bufferInfo)
            .Build(pbrDiffuseSAMaterial.m_DescriptorSet);

    }

    void VK_Model::CreateDescriptorSet(PbrEmissiveTextureMaterial& pbrEmissiveTextureMaterial,
                                       const std::shared_ptr<Texture>& emissiveMap)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        const auto& imageInfo = static_cast<VK_Texture*>(emissiveMap.get())->GetDescriptorImageInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, imageInfo)
            .Build(pbrEmissiveTextureMaterial.m_DescriptorSet);
    }

    void VK_Model::CreateDescriptorSet(PbrDiffuseNormalMaterial& pbrDiffuseNormalMaterial,
                                       const std::shared_ptr<Texture>& colorMap,
                                       const std::shared_ptr<Texture>& normalMap)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        auto& imageInfo0 = static_cast<VK_Texture*>(colorMap.get())->GetDescriptorImageInfo();
        auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, imageInfo0)
            .WriteImage(1, imageInfo1)
            .Build(pbrDiffuseNormalMaterial.m_DescriptorSet);
    }

    void VK_Model::CreateDescriptorSet(PbrDiffuseNormalRoughnessMetallicMaterial& pbrDiffuseNormalRoughnessMetallicMaterial,
                                       const std::shared_ptr<Texture>& colorMap,
                                       const std::shared_ptr<Texture>& normalMap, 
                                       const std::shared_ptr<Texture>& roughnessMetallicMap)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        auto& imageInfo0 = static_cast<VK_Texture*>(colorMap.get())->GetDescriptorImageInfo();
        auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
        auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, imageInfo0)
            .WriteImage(1, imageInfo1)
            .WriteImage(2, imageInfo2)
            .Build(pbrDiffuseNormalRoughnessMetallicMaterial.m_DescriptorSet);
    }

    void VK_Model::CreateDescriptorSet(CubemapMaterial& cubemapMaterial, const std::shared_ptr<Cubemap>& cubemap)
    {
        std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout = VK_DescriptorSetLayout::Builder()
                    .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                    .Build();

        VkDescriptorImageInfo cubemapInfo = static_cast<VK_Cubemap*>(cubemap.get())->GetDescriptorImageInfo();

        VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
            .WriteImage(0, cubemapInfo)
            .Build(cubemapMaterial.m_DescriptorSet);
    }
}
