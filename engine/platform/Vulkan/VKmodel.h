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
#include "scene/material.h"
#include "scene/scene.h"

#include "VKdevice.h"
#include "VKbuffer.h"
#include "VKswapChain.h"
#include "VKframeInfo.h"
#include "VKtexture.h"

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
        ~VK_Model() override {}

        VK_Model(const VK_Model&) = delete;
        VK_Model& operator=(const VK_Model&) = delete;

        void CreateVertexBuffers(const std::vector<Vertex>& vertices) override;
        void CreateIndexBuffers(const std::vector<uint>& indices) override;

        void Bind(VkCommandBuffer commandBuffer);

        void Draw(VkCommandBuffer commandBuffer);
        void DrawNoMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);
        void DrawDiffuseNormalRoughnessMetallicMap(const VK_FrameInfo& frameInfo, TransformComponent& transform, const VkPipelineLayout& pipelineLayout);

    public:

        static void CreateDescriptorSet
        (
            PbrDiffuseMaterial& pbrDiffuseMaterial,
            const std::shared_ptr<VK_Texture>& colorMap
        );
        static void CreateDescriptorSet
        (
            PbrDiffuseNormalMaterial& pbrDiffuseNormalMaterial,
            const std::shared_ptr<VK_Texture>& colorMap,
            const std::shared_ptr<VK_Texture>& normalMap
        );
        static void CreateDescriptorSet
        (
            PbrDiffuseNormalRoughnessMetallicMaterial& pbrDiffuseNormalRoughnessMetallicMaterial,
            const std::shared_ptr<VK_Texture>& colorMap,
            const std::shared_ptr<VK_Texture>& normalMap, 
            const std::shared_ptr<VK_Texture>& roughnessMetallicMap
        );

    public:

        static std::vector<std::shared_ptr<VK_Texture>> m_Images;

    private:

        std::vector<std::shared_ptr<VK_Texture>> m_ImagesInternal;
        std::shared_ptr<VK_Device> m_Device;

        std::unique_ptr<VK_Buffer> m_VertexBuffer;
        uint m_VertexCount;

        bool m_HasIndexBuffer;
        std::unique_ptr<VK_Buffer> m_IndexBuffer;
        uint m_IndexCount;

        std::vector<PrimitiveNoMap> m_PrimitivesNoMap{};
        std::vector<PrimitiveDiffuseMap> m_PrimitivesDiffuseMap{};
        std::vector<PrimitiveDiffuseNormalMap> m_PrimitivesDiffuseNormalMap{};
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicMap> m_PrimitivesDiffuseNormalRoughnessMetallicMap{};

    };
}
