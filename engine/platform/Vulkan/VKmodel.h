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

#pragma once

#include <memory>
#include <vector>

#include "engine.h"
#include "renderer/model.h"
#include "renderer/buffer.h"
#include "renderer/builder/builder.h"
#include "renderer/builder/gltfBuilder.h"
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

namespace GfxRenderEngine
{

    struct VK_Submesh : public Submesh
    {
        VK_Submesh(ModelSubmesh const& modelSubmesh, uint materialDescriptorIndex);
        VK_MaterialDescriptor m_MaterialDescriptor;
    };

    class VK_Model : public Model
    {

    public:
        struct VK_Vertex : public Vertex
        {
            static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
        };

    public:
        VK_Model(std::shared_ptr<VK_Device> device, const Builder& builder);
        VK_Model(std::shared_ptr<VK_Device> device, const GltfBuilder& builder);
        VK_Model(std::shared_ptr<VK_Device> device, const FastgltfBuilder& builder);
        VK_Model(std::shared_ptr<VK_Device> device, const FbxBuilder& builder);
        VK_Model(std::shared_ptr<VK_Device> device, const UFbxBuilder& builder);
        virtual ~VK_Model() override;

        VK_Model(const VK_Model&) = delete;
        VK_Model& operator=(const VK_Model&) = delete;

        virtual void CreateVertexBuffers(const std::vector<Vertex>& vertices) override;
        virtual void CreateIndexBuffers(const std::vector<uint>& indices) override;

        void Bind(VkCommandBuffer commandBuffer);
        void UpdateAnimation(const Timestep& timestep, uint frameCounter);

        void PushConstants(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, VK_Submesh const& submesh);
        void PushConstants(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                           const VkPipelineLayout& pipelineLayout, VK_Submesh const& submesh);
        void BindDescriptors(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                             VK_Submesh const& submesh);

        void Draw(VkCommandBuffer commandBuffer);
        void DrawSubmesh(VkCommandBuffer commandBuffer, Submesh const& submesh);

        // draw pbr materials
        void DrawNoMapInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseMapInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseSAMapInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalMapInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalSAMapInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallicMapInstanced(const VK_FrameInfo& frameInfo,
                                                            const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallic2MapInstanced(const VK_FrameInfo& frameInfo,
                                                             const VkPipelineLayout& pipelineLayout);
        void DrawEmissiveInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                   float emissiveStrength = 0.f);
        void DrawEmissiveTextureInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                          float emissiveStrength = 0.f);

        void DrawNoMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                            const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                              const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                  const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                    const VkPipelineLayout& pipelineLayout);
        void DrawEmissive(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                          const VkPipelineLayout& pipelineLayout, float emissiveStrength = 0.f);
        void DrawEmissiveTexture(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                 const VkPipelineLayout& pipelineLayout, float emissiveStrength = 0.f);
        void DrawDiffuseNormalRoughnessMetallicSA2Map(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                                      const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallicSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                                     const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallic2Map(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                                    const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallicMap(const VK_FrameInfo& frameInfo, TransformComponent& transform,
                                                   const VkPipelineLayout& pipelineLayout);

        void DrawShadow(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
        void DrawShadowAnimated(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                const VkDescriptorSet& descriptorSet);
        void DrawShadowInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                 VkDescriptorSet const& shadowDescriptorSet);
        void DrawShadowAnimatedInstanced(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout,
                                         const VkDescriptorSet& shadowDescriptorSet);
        void DrawAnimatedShadowInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout,
                                        VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet);
        void DrawShadowInstancedInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout,
                                         VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet);
        void DrawAnimatedShadowInstancedInternal(VK_FrameInfo const& frameInfo, VkPipelineLayout const& pipelineLayout,
                                                 VK_Submesh const& submesh, VkDescriptorSet const& shadowDescriptorSet);
        void DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);

    private:
        void CopySubmeshes(std::vector<ModelSubmesh> const& modelSubmeshes);

    private:
        std::shared_ptr<VK_Device> m_Device;
        std::unique_ptr<VK_Buffer> m_VertexBuffer;

        uint m_VertexCount;

        bool m_HasIndexBuffer;
        std::unique_ptr<VK_Buffer> m_IndexBuffer;
        uint m_IndexCount;

        std::vector<VK_Submesh> m_SubmeshesPbrNoMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrEmissive{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseSAMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrNoMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrEmissiveTexture{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrEmissiveInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalSAMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseSAMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrEmissiveTextureInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalSAMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallicMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallic2Map{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallicSAMap{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallicSA2Map{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallicMapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesPbrDiffuseNormalRoughnessMetallic2MapInstanced{};
        std::vector<VK_Submesh> m_SubmeshesCubemap{};
    };
} // namespace GfxRenderEngine
