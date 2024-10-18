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
#include "VKmodel.h"
#include "VKdescriptor.h"
#include "VKmaterialDescriptor.h"
#include "VKrenderer.h"

#include "systems/pushConstantData.h"

namespace GfxRenderEngine
{
    // Vertex
    std::vector<VkVertexInputBindingDescription> VK_Model::VK_Vertex::GetBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VK_Model::VK_Vertex::GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, m_Color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, m_UV)});
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, m_Tangent)});
        attributeDescriptions.push_back({5, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(Vertex, m_JointIds)});
        attributeDescriptions.push_back({6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, m_Weights)});

        return attributeDescriptions;
    }

#define INIT_MODEL()                                   \
    CopySubmeshes(builder.m_Submeshes);                \
    CreateVertexBuffer(std::move(builder.m_Vertices)); \
    CreateIndexBuffer(std::move(builder.m_Indices));

#define INIT_GLTF_AND_FBX_MODEL()                      \
    CopySubmeshes(builder.m_Submeshes);                \
    CreateVertexBuffer(std::move(builder.m_Vertices)); \
    CreateIndexBuffer(std::move(builder.m_Indices));   \
    m_Skeleton = std::move(builder.m_Skeleton);        \
    m_Animations = std::move(builder.m_Animations);    \
    m_ShaderDataUbo = builder.m_ShaderData;

    VK_Model::VK_Model(const Model::ModelData& modelData) : m_Device(VK_Core::m_Device)
    {
        ZoneScopedNC("VK_Model(FastgltfBuilder)", 0x00ffff);

        CopySubmeshes(modelData.m_Submeshes);
        CreateVertexBuffer(std::move(modelData.m_Vertices));
        CreateIndexBuffer(std::move(modelData.m_Indices));
        m_Skeleton = std::move(modelData.m_Skeleton);
        m_Animations = std::move(modelData.m_Animations);
        m_ShaderDataUbo = std::move(modelData.m_ShaderData);
    }
    VK_Model::VK_Model(VK_Device* device, const UFbxBuilder& builder) : m_Device(device) { INIT_GLTF_AND_FBX_MODEL(); }
    VK_Model::VK_Model(VK_Device* device, const GltfBuilder& builder) : m_Device(device) { INIT_GLTF_AND_FBX_MODEL(); }
    VK_Model::VK_Model(VK_Device* device, const FbxBuilder& builder) : m_Device(device) { INIT_GLTF_AND_FBX_MODEL(); }
    VK_Model::VK_Model(VK_Device* device, const Builder& builder) : m_Device(device)
    {
        INIT_MODEL();
        m_Cubemaps = std::move(builder.m_Cubemaps); // used to manage lifetime
    }
    VK_Model::VK_Model(VK_Device* device, const TerrainBuilder& builder) : m_Device(device) { INIT_MODEL(); }

    VK_Model::~VK_Model() {}

    VK_Submesh::VK_Submesh(Submesh const& submesh)
        : Submesh{submesh.m_FirstIndex,    submesh.m_FirstVertex, submesh.m_IndexCount, submesh.m_VertexCount,
                  submesh.m_InstanceCount, submesh.m_Material,    submesh.m_Resources},
          m_MaterialDescriptor(submesh.m_Material.m_MaterialDescriptor),
          m_ResourceDescriptor(submesh.m_Resources.m_ResourceDescriptor)
    {
    }

    void VK_Model::CopySubmeshes(std::vector<Submesh> const& submeshes)
    {
        for (auto& submesh : submeshes)
        {
            VK_Submesh vkSubmesh(submesh);

            MaterialDescriptor::MaterialType materialType = vkSubmesh.m_MaterialDescriptor.GetMaterialType();

            switch (materialType)
            {
                case MaterialDescriptor::MaterialType::MtPbr:
                {
                    m_SubmeshesPbrMap.push_back(vkSubmesh);
                    break;
                }
                case MaterialDescriptor::MaterialType::MtCubemap:
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

    void VK_Model::CreateVertexBuffer(const std::vector<Vertex>& vertices) { CreateVertexBuffer<Vertex>(vertices); }

    void VK_Model::CreateIndexBuffer(const std::vector<uint>& indices)
    {
        m_IndexCount = static_cast<uint>(indices.size());
        m_HasIndexBuffer = (m_IndexCount > 0);
        if (!m_HasIndexBuffer)
        {
            return;
        }

        VkDeviceSize bufferSize = sizeof(uint) * m_IndexCount;
        uint indexSize = sizeof(indices[0]);

        VK_Buffer stagingBuffer{indexSize, m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)indices.data());

        m_IndexBuffer = std::make_unique<VK_Buffer>(indexSize, m_IndexCount,
                                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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

    void VK_Model::BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                   VK_Submesh const& submesh)
    {
        const VkDescriptorSet& materialDescriptorSet = submesh.m_MaterialDescriptor.GetDescriptorSet();
        std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, materialDescriptorSet};
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer,       // VkCommandBuffer        commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint    pipelineBindPoint,
                                pipelineLayout,                  // VkPipelineLayout       layout,
                                0,                               // uint32_t               firstSet,
                                descriptorSets.size(),           // uint32_t               descriptorSetCount,
                                descriptorSets.data(),           // const VkDescriptorSet* pDescriptorSets,
                                0,                               // uint32_t               dynamicOffsetCount,
                                nullptr                          // const uint32_t*        pDynamicOffsets);
        );
    }

    void VK_Model::BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                   VK_Submesh const& submesh, bool bindResources)
    {
        const VkDescriptorSet& materialDescriptorSet = submesh.m_MaterialDescriptor.GetDescriptorSet();
        const VkDescriptorSet& resourceDescriptorSet = submesh.m_ResourceDescriptor.GetDescriptorSet();
        std::vector<VkDescriptorSet> descriptorSets = {frameInfo.m_GlobalDescriptorSet, materialDescriptorSet,
                                                       resourceDescriptorSet};
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer,       // VkCommandBuffer        commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS, // VkPipelineBindPoint    pipelineBindPoint,
                                pipelineLayout,                  // VkPipelineLayout       layout,
                                0,                               // uint32_t               firstSet,
                                descriptorSets.size(),           // uint32_t               descriptorSetCount,
                                descriptorSets.data(),           // const VkDescriptorSet* pDescriptorSets,
                                0,                               // uint32_t               dynamicOffsetCount,
                                nullptr                          // const uint32_t*        pDynamicOffsets);
        );
    }

    // multi-instance
    void VK_Model::PushConstantsPbr(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                    VK_Submesh const& submesh)
    {
        vkCmdPushConstants(frameInfo.m_CommandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(Material::PbrMaterial), &submesh.m_Material.m_PbrMaterial);
    }

    void VK_Model::Draw(VkCommandBuffer commandBuffer)
    {
        if (m_HasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer, // VkCommandBuffer commandBuffer
                             m_IndexCount,  // uint32_t        indexCount
                             1,             // uint32_t        instanceCount
                             0,             // uint32_t        firstIndex
                             0,             // int32_t         vertexOffset
                             0              // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw(commandBuffer, // VkCommandBuffer commandBuffer
                      m_VertexCount, // uint32_t        vertexCount
                      1,             // uint32_t        instanceCount
                      0,             // uint32_t        firstVertex
                      0              // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawSubmesh(VkCommandBuffer commandBuffer, Submesh const& submesh)
    {
        if (m_HasIndexBuffer)
        {
            vkCmdDrawIndexed(commandBuffer,           // VkCommandBuffer commandBuffer
                             submesh.m_IndexCount,    // uint32_t        indexCount
                             submesh.m_InstanceCount, // uint32_t        instanceCount
                             submesh.m_FirstIndex,    // uint32_t        firstIndex
                             submesh.m_FirstVertex,   // int32_t         vertexOffset
                             0                        // uint32_t        firstInstance
            );
        }
        else
        {
            vkCmdDraw(commandBuffer,           // VkCommandBuffer commandBuffer
                      submesh.m_VertexCount,   // uint32_t        vertexCount
                      submesh.m_InstanceCount, // uint32_t        instanceCount
                      submesh.m_FirstVertex,   // uint32_t        firstVertex
                      0                        // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawPbr(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
    {
        for (auto& submesh : m_SubmeshesPbrMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh, true /*bind resources*/);
            PushConstantsPbr(frameInfo, pipelineLayout, submesh);
            DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
        }
    }

    void VK_Model::DrawGrass(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, int instanceCount)
    {
        for (auto& submesh : m_SubmeshesPbrMap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh, true /*bind resources*/);
            PushConstantsPbr(frameInfo, pipelineLayout, submesh);
            vkCmdDrawIndexed(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                             submesh.m_IndexCount,      // uint32_t        indexCount
                             instanceCount,             // uint32_t        instanceCount
                             submesh.m_FirstIndex,      // uint32_t        firstIndex
                             submesh.m_FirstVertex,     // int32_t         vertexOffset
                             0                          // uint32_t        firstInstance
            );
        }
    }

    void VK_Model::DrawShadowInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                       const VkDescriptorSet& shadowDescriptorSet)
    {
        for (auto& submesh : m_SubmeshesPbrMap)
        {
            DrawShadowInstancedInternal(frameInfo, pipelineLayout, submesh, shadowDescriptorSet);
        }
    }

    void VK_Model::DrawShadowInstancedInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout,
                                               VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet)
    {
        VkDescriptorSet localDescriptorSet = submesh.m_ResourceDescriptor.GetDescriptorSet();
        std::vector<VkDescriptorSet> descriptorSets = {shadowDescriptorSet, localDescriptorSet};
        CORE_ASSERT(localDescriptorSet, "resource descriptor set empty");
        vkCmdBindDescriptorSets(frameInfo.m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2,
                                descriptorSets.data(), 0, nullptr);

        DrawSubmesh(frameInfo.m_CommandBuffer, submesh);
    }

    void VK_Model::DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout)
    {
        for (auto& submesh : m_SubmeshesCubemap)
        {
            BindDescriptors(frameInfo, pipelineLayout, submesh);
            vkCmdDraw(frameInfo.m_CommandBuffer, // VkCommandBuffer commandBuffer
                      submesh.m_VertexCount,     // uint32_t        vertexCount
                      1,                         // uint32_t        instanceCount
                      submesh.m_FirstVertex,     // uint32_t        firstVertex
                      0                          // uint32_t        firstInstance
            );
        }
    }
} // namespace GfxRenderEngine
