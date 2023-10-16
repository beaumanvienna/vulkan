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

#include <memory>
#include <vector>

#include "engine.h"
#include "renderer/model.h"
#include "renderer/buffer.h"
#include "scene/material.h"
#include "scene/scene.h"

#include "VKdevice.h"
#include "VKbuffer.h"
#include "VKswapChain.h"
#include "VKframeInfo.h"
#include "VKtexture.h"
#include "VKcubemap.h"

namespace GfxRenderEngine
{

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
        ~VK_Model() override;

        VK_Model(const VK_Model&) = delete;
        VK_Model& operator=(const VK_Model&) = delete;

        void CreateVertexBuffers(const std::vector<Vertex>& vertices) override;
        void CreateIndexBuffers(const std::vector<uint>& indices) override;

        void Bind(VkCommandBuffer commandBuffer);

        void Draw(VkCommandBuffer commandBuffer);
        void DrawNoMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseSAMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawEmissive(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength = 0.f);
        void DrawEmissiveTexture(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, float emissiveStrength = 0.f);
        void DrawDiffuseNormalRoughnessMetallicMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawShadow(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawCubemap(const VK_FrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);

    public:

        static void CreateDescriptorSet
        (
            PbrDiffuseMaterial& pbrDiffuseMaterial,
            const std::shared_ptr<Texture>& colorMap
        );
        static void CreateDescriptorSet
        (
            PbrDiffuseSAMaterial& pbrDiffuseSAMaterial,
            const std::shared_ptr<Texture>& colorMap,
            const std::shared_ptr<Buffer>& skeletalAnimationUBO
        );
        static void CreateDescriptorSet
        (
            PbrEmissiveTextureMaterial& pbrEmissiveTextureMaterial,
            const std::shared_ptr<Texture>& emissiveMap
        );
        static void CreateDescriptorSet
        (
            PbrDiffuseNormalMaterial& pbrDiffuseNormalMaterial,
            const std::shared_ptr<Texture>& colorMap,
            const std::shared_ptr<Texture>& normalMap
        );
        static void CreateDescriptorSet
        (
            PbrDiffuseNormalRoughnessMetallicMaterial& pbrDiffuseNormalRoughnessMetallicMaterial,
            const std::shared_ptr<Texture>& colorMap,
            const std::shared_ptr<Texture>& normalMap, 
            const std::shared_ptr<Texture>& roughnessMetallicMap
        );

        static void CreateDescriptorSet(CubemapMaterial& cubemapMaterial, const std::shared_ptr<Cubemap>& cubemap);

    private:

        void DrawShadowInternal(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout, const PrimitiveTmp& primitive);

    private:

        std::shared_ptr<VK_Device> m_Device;
        std::unique_ptr<VK_Buffer> m_VertexBuffer;

        uint m_VertexCount;

        bool m_HasIndexBuffer;
        std::unique_ptr<VK_Buffer> m_IndexBuffer;
        uint m_IndexCount;

        std::vector<PrimitiveNoMap> m_PrimitivesNoMap{};
        std::vector<PrimitiveEmissive> m_PrimitivesEmissive{};
        std::vector<PrimitiveDiffuseMap> m_PrimitivesDiffuseMap{};
        std::vector<PrimitiveDiffuseSAMap> m_PrimitivesDiffuseSAMap{};
        std::vector<PrimitiveEmissiveTexture> m_PrimitivesEmissiveTexture{};
        std::vector<PrimitiveDiffuseNormalMap> m_PrimitivesDiffuseNormalMap{};
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicMap> m_PrimitivesDiffuseNormalRoughnessMetallicMap{};
        std::vector<PrimitiveCubemap> m_PrimitivesCubemap{};

        // skeletal animation
        std::vector<Armature::Skeleton> m_Skeletons;
        std::shared_ptr<Buffer> m_ShaderDataUbo;

    };
}
