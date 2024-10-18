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

#pragma once

#include <memory>
#include <vector>

#include "engine.h"
#include "renderer/model.h"
#include "renderer/buffer.h"
#include "renderer/builder/builder.h"
#include "renderer/builder/gltfBuilder.h"
#include "renderer/builder/terrainBuilder.h"
#include "renderer/builder/fastgltfBuilder.h"
#include "renderer/builder/ufbxBuilder.h"
#include "renderer/builder/fbxBuilder.h"
#include "scene/material.h"
#include "scene/scene.h"

#include "VKdevice.h"
#include "VKbuffer.h"
#include "VKswapChain.h"
#include "VKframeInfo.h"
#include "VKtexture.h"
#include "VKcubemap.h"
#include "VKmaterialDescriptor.h"
#include "VKresourceDescriptor.h"

namespace GfxRenderEngine
{

    struct VK_Submesh : public Submesh
    {
        VK_Submesh(Submesh const& submesh);
        VK_MaterialDescriptor m_MaterialDescriptor;
        VK_ResourceDescriptor m_ResourceDescriptor;
    };

    class VK_Model : public Model
    {
    private:
        template <typename T> void CreateVertexBuffer(std::vector<T> const& vertices)
        {
            m_VertexCount = static_cast<uint>(vertices.size());
            CORE_ASSERT(m_VertexCount >= 3, "CreateVertexBuffer: at least one triangle required");
            uint vertexSize = sizeof(T);
            VkDeviceSize bufferSize = vertexSize * m_VertexCount;

            VK_Buffer stagingBuffer{vertexSize, m_VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
            stagingBuffer.Map();
            stagingBuffer.WriteToBuffer((void*)vertices.data());

            m_VertexBuffer = std::make_unique<VK_Buffer>(
                vertexSize, m_VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            m_Device->CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
        }

    public:
        struct VK_Vertex : public Vertex
        {
            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        };

    public:
        VK_Model(VK_Device* device, const Builder& builder);
        VK_Model(VK_Device* device, const GltfBuilder& builder);
        VK_Model(const Model::ModelData&);
        VK_Model(VK_Device* device, const FbxBuilder& builder);
        VK_Model(VK_Device* device, const UFbxBuilder& builder);
        VK_Model(VK_Device* device, const TerrainBuilder& builder);
        virtual ~VK_Model() override;

        VK_Model(const VK_Model&) = delete;
        VK_Model& operator=(const VK_Model&) = delete;

        virtual void CreateVertexBuffer(const std::vector<Vertex>& vertices) override;
        virtual void CreateIndexBuffer(const std::vector<uint>& indices) override;

        void Bind(VkCommandBuffer commandBuffer);
        void UpdateAnimation(const Timestep& timestep, uint frameCounter);

        void PushConstantsPbr(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                              VK_Submesh const& submesh);
        void BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                             VK_Submesh const& submesh);
        void BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                             VK_Submesh const& submesh, bool bindResources);

        void Draw(VkCommandBuffer commandBuffer);
        void DrawSubmesh(VkCommandBuffer commandBuffer, Submesh const& submesh);

        // draw pbr materials
        void DrawPbr(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawGrass(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, int instanceCount);

        // draw shadow
        void DrawShadowInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                 VkDescriptorSet const& shadowDescriptorSet);
        void DrawShadowInstancedInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout,
                                         VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet);
        // cube map
        void DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);

    private:
        void CopySubmeshes(std::vector<Submesh> const& submeshes);

    private:
        VK_Device* m_Device;
        std::unique_ptr<VK_Buffer> m_VertexBuffer;

        uint m_VertexCount{0};
        uint m_IndexCount{0};

        bool m_HasIndexBuffer{false};
        std::unique_ptr<VK_Buffer> m_IndexBuffer;

        std::vector<VK_Submesh> m_SubmeshesPbrMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrSAMap{};
        std::vector<VK_Submesh> m_SubmeshesCubemap{};
    };
} // namespace GfxRenderEngine
