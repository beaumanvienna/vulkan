/* Engine Copyright (c) 2023 Engine Development Team
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
#include "VKmaterialDescriptor.h"
#include "VKrenderer.h"

#include "systems/VKpbrNoMapSys.h"
#include "systems/VKpbrDiffuseSys.h"
#include "systems/VKpbrEmissiveSys.h"
#include "systems/VKpbrDiffuseNormalSys.h"
#include "systems/VKpbrDiffuseNormalSASys.h"
#include "systems/VKpbrEmissiveTextureSys.h"
#include "systems/VKpbrDiffuseNormalRoughnessMetallicSys.h"
#include "systems/VKpbrDiffuseNormalRoughnessMetallicSASys.h"
#include "systems/VKshadowRenderSys.h"
#include "systems/pushConstantData.h"

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
    VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const GltfBuilder& builder)
        : m_Device(device), m_HasIndexBuffer{false}
    {
        CopySubmeshes(builder.m_Submeshes);
        m_Images = std::move(builder.m_Images);

        m_Skeleton = std::move(builder.m_Skeleton);
        m_Animations = std::move(builder.m_Animations);
        m_ShaderDataUbo = builder.m_ShaderData;

        CreateVertexBuffers(std::move(builder.m_Vertices));
        CreateIndexBuffers(std::move(builder.m_Indices));
    }

    VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const FbxBuilder& builder)
        : m_Device(device), m_HasIndexBuffer{false}
    {
        CopySubmeshes(builder.m_Submeshes);
        m_Images = std::move(builder.m_Images);

        m_Skeleton = std::move(builder.m_Skeleton);
        m_Animations = std::move(builder.m_Animations);
        m_ShaderDataUbo = builder.m_ShaderData;

        CreateVertexBuffers(std::move(builder.m_Vertices));
        CreateIndexBuffers(std::move(builder.m_Indices));
    }

    VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const Builder& builder)
        : m_Device(device), m_HasIndexBuffer{false}
    {
        CopySubmeshes(builder.m_Submeshes);
        m_Cubemaps = std::move(builder.m_Cubemaps);

        CreateVertexBuffers(std::move(builder.m_Vertices));
        CreateIndexBuffers(std::move(builder.m_Indices));
    }

    VK_Model::~VK_Model() {}

    VK_Submesh::VK_Submesh(ModelSubmesh const& modelSubmesh, uint materialDescriptorIndex)
        : Submesh{modelSubmesh.m_FirstIndex, modelSubmesh.m_FirstVertex, modelSubmesh.m_IndexCount, modelSubmesh.m_VertexCount, 
                  modelSubmesh.m_InstanceCount, modelSubmesh.m_MaterialProperties},
          m_MaterialDescriptor(*(static_cast<VK_MaterialDescriptor*>(modelSubmesh.m_MaterialDescriptors[materialDescriptorIndex].get())))
    {
    }

    void VK_Model::CopySubmeshes(std::vector<ModelSubmesh> const& modelSubmeshes)
    {
        for (auto& modelSubmesh : modelSubmeshes)
        {
            uint numMaterialDescriptors = modelSubmesh.m_MaterialDescriptors.size();
            for (uint materialDescriptorIndex = 0; materialDescriptorIndex < numMaterialDescriptors; ++materialDescriptorIndex)
            {
                VK_Submesh vkSubmesh(modelSubmesh, materialDescriptorIndex);
                
                MaterialDescriptor::MaterialType materialType = vkSubmesh.m_MaterialDescriptor.GetMaterialType();

                switch (materialType)
                {
                    case MaterialDescriptor::MtPbrNoMap:
                    {
                        m_SubmeshesPbrNoMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrNoMapInstanced:
                    {
                        m_SubmeshesPbrNoMapInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrEmissive:
                    {
                        m_SubmeshesPbrEmissive.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrEmissiveInstanced:
                    {
                        m_SubmeshesPbrEmissiveInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseMap:
                    {
                        m_SubmeshesPbrDiffuseMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseMapInstanced:
                    {
                        m_SubmeshesPbrDiffuseMapInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseSAMap:
                    {
                        m_SubmeshesPbrDiffuseSAMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrEmissiveTexture:
                    {
                        m_SubmeshesPbrEmissiveTexture.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrEmissiveTextureInstanced:
                    {
                        m_SubmeshesPbrEmissiveTextureInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalMap:
                    {
                        m_SubmeshesPbrDiffuseNormalMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalMapInstanced:
                    {
                        m_SubmeshesPbrDiffuseNormalMapInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalSAMap:
                    {
                        m_SubmeshesPbrDiffuseNormalSAMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMap:
                    {
                        m_SubmeshesPbrDiffuseNormalRoughnessMetallicMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMapInstanced:
                    {
                        m_SubmeshesPbrDiffuseNormalRoughnessMetallicMapInstanced.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2Map:
                    {
                        m_SubmeshesPbrDiffuseNormalRoughnessMetallic2Map.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSAMap:
                    {
                        m_SubmeshesPbrDiffuseNormalRoughnessMetallicSAMap.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSA2Map:
                    {
                        m_SubmeshesPbrDiffuseNormalRoughnessMetallicSA2Map.push_back(vkSubmesh);
                        break;
                    }
                    case MaterialDescriptor::MtCubemap:
                    {
                        m_SubmeshesCubemap.push_back(vkSubmesh);
                        break;
                    }
                    default:
                    {
                        CORE_ASSERT(false, "unkown material type");
                        break;
                    }
                }
            }
        }
    }

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

    void VK_Model::UpdateAnimation(const Timestep& timestep, uint frameCounter)
    {

        m_Animations->Update(timestep, *m_Skeleton, frameCounter);
        m_Skeleton->Update();

        // update ubo
        static_cast<VK_Buffer*>(m_ShaderDataUbo.get())->WriteToBuffer(m_Skeleton->m_ShaderData.m_FinalJointsMatrices.data());
        static_cast<VK_Buffer*>(m_ShaderDataUbo.get())->Flush();
    }

    void VK_Model::BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, VK_Submesh const& submesh)
    {
        auto& localDescriptorSet = submesh.m_MaterialDescriptor.GetDescriptorSet();
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
    }

    // single-instance
    void VK_Model::PushConstants(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, VK_Submesh const& submesh)
    {
        VK_PushConstantDataGeneric push{};
        push.m_ModelMatrix  = transform.GetMat4Global();
        push.m_NormalMatrix = transform.GetNormalMatrix();

        push.m_NormalMatrix[3].x = submesh.m_MaterialProperties.m_Roughness;
        push.m_NormalMatrix[3].y = submesh.m_MaterialProperties.m_Metallic;
        push.m_NormalMatrix[3].z = submesh.m_MaterialProperties.m_NormalMapIntensity;
        push.m_NormalMatrix[3].w = submesh.m_MaterialProperties.m_EmissiveStrength;

        vkCmdPushConstants(
            frameInfo.m_CommandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(VK_PushConstantDataGeneric),
            &push);
    }

    // multi-instance
    void VK_Model::PushConstants(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, VK_Submesh const& submesh)
    {
        VK_PushConstantDataGenericInstanced push{};

        push.m_Roughness = submesh.m_MaterialProperties.m_Roughness;
        push.m_Metallic = submesh.m_MaterialProperties.m_Metallic;
        push.m_NormalMapIntensity = submesh.m_MaterialProperties.m_NormalMapIntensity;
        push.m_EmissiveStrength = submesh.m_MaterialProperties.m_EmissiveStrength;

        vkCmdPushConstants(
            frameInfo.m_CommandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(VK_PushConstantDataGenericInstanced),
            &push);
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

    void VK_Model::DrawSubmesh(VkCommandBuffer commandBuffer, Submesh const& submesh, uint instanceCount/* = 1*/)
    {
        if(m_HasIndexBuffer)
        {
            vkCmdDrawIndexed
            (
                commandBuffer,              // VkCommandBuffer commandBuffer
                submesh.m_IndexCount,       // uint32_t        indexCount
                instanceCount,              // uint32_t        instanceCount
                submesh.m_FirstIndex,       // uint32_t        firstIndex
                submesh.m_FirstVertex,      // int32_t         vertexOffset
                0                           // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw
            (
                commandBuffer,              // VkCommandBuffer commandBuffer
                submesh.m_VertexCount,      // uint32_t        vertexCount
                instanceCount,              // uint32_t        instanceCount
                submesh.m_FirstVertex,      // uint32_t        firstVertex
                0                           // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawNoMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrNoMap)
        {
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawNoMapInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrNoMapInstanced)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawEmissive(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& submesh : m_SubmeshesPbrEmissive)
        {

            if (emissiveStrength)
            {
                submesh.m_MaterialProperties.m_EmissiveStrength = emissiveStrength;
            }

            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawEmissiveInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& submesh : m_SubmeshesPbrEmissiveInstanced)
        {
            if (emissiveStrength)
            {
                submesh.m_MaterialProperties.m_EmissiveStrength = emissiveStrength;
            }

            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawEmissiveTexture(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& submesh : m_SubmeshesPbrEmissiveTexture)
        {
            if (emissiveStrength)
            {
                submesh.m_MaterialProperties.m_EmissiveStrength = emissiveStrength;
            }

            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawEmissiveTextureInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout, float emissiveStrength)
    {
        for(auto& submesh : m_SubmeshesPbrEmissiveTextureInstanced)
        {
            if (emissiveStrength)
            {
                submesh.m_MaterialProperties.m_EmissiveStrength = emissiveStrength;
            }

            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawDiffuseMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseMapInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseMapInstanced)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawDiffuseSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseSAMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalMapInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalMapInstanced)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawDiffuseNormalSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalSAMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallicMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallicMapInstanced(const VK_FrameInfo& frameInfo, uint instanceCount, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicMapInstanced)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, instanceCount);
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallicSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicSAMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallic2Map(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallic2Map)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawDiffuseNormalRoughnessMetallicSA2Map(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicSA2Map)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            PushConstants(frameInfo, transform, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawShadow(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
    {
        for (auto& submesh : m_SubmeshesPbrNoMap)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrNoMapInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
        for (auto& submesh : m_SubmeshesPbrEmissive)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrEmissiveInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
        for (auto& submesh : m_SubmeshesPbrEmissiveTexture)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrEmissiveTextureInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseMap)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseMapInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseNormalMap)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseNormalMapInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicMap)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
        for (auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicMapInstanced)
        {
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh, submesh.m_InstanceCount);
        }
    }

    void VK_Model::DrawShadowAnimated(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, const VkDescriptorSet& shadowDescriptorSet)
    {
        for(auto& submesh : m_SubmeshesPbrDiffuseSAMap)
        {
            DrawAnimatedShadowInternal(frameInfo, pipelineLayout, submesh, shadowDescriptorSet);
        }

        for(auto& submesh : m_SubmeshesPbrDiffuseNormalRoughnessMetallicSAMap)
        {
            DrawAnimatedShadowInternal(frameInfo, pipelineLayout, submesh, shadowDescriptorSet);
        }

        for(auto& submesh : m_SubmeshesPbrDiffuseNormalSAMap)
        {
            DrawAnimatedShadowInternal(frameInfo, pipelineLayout, submesh, shadowDescriptorSet);
        }
    }

    void VK_Model::DrawAnimatedShadowInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout, VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet)
    {
        VkDescriptorSet localDescriptorSet = submesh.m_MaterialDescriptor.GetShadowDescriptorSet();
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

        DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
    }

    void VK_Model::DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
    {
        for(auto& submesh : m_SubmeshesCubemap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            vkCmdDraw
            (
                frameInfo.m_CommandBuffer,  // VkCommandBuffer commandBuffer
                submesh.m_VertexCount,      // uint32_t        vertexCount
                1,                          // uint32_t        instanceCount
                submesh.m_FirstVertex,      // uint32_t        firstVertex
                0                           // uint32_t        firstInstance
            );
        }
    }
}
