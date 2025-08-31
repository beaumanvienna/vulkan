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

#include <vector>
#include <bitset>

#include "engine.h"
#include "scene/scene.h"

#include "VKframeInfo.h"
#include "VKpipeline.h"

namespace GfxRenderEngine
{

    class VK_RenderSystemDeferredShading
    {
    public:
        struct VK_PushConstantsIBL
        {
            // x: uMaxPrefilterMip, number of mips - 1
            // y: float exposure
            // z: reserve
            // w: reserve
            glm::vec4 m_Values0;
            // x: shader settings 0 (see shader.h)
            // y: reserve
            // z: reserve
            // w: reserve
            glm::ivec4 m_Values1;
        };

    public:
        VK_RenderSystemDeferredShading(VkRenderPass renderPass,
                                       std::vector<VkDescriptorSetLayout>& lightingDescriptorSetLayouts,
                                       const VkDescriptorSet* lightingDescriptorSet,
                                       const VkDescriptorSet* shadowMapDescriptorSet);
        ~VK_RenderSystemDeferredShading();

        VK_RenderSystemDeferredShading(const VK_RenderSystemDeferredShading&) = delete;
        VK_RenderSystemDeferredShading& operator=(const VK_RenderSystemDeferredShading&) = delete;

        void LightingPass(const VK_FrameInfo& frameInfo, VkDescriptorSet* lightingDescriptorSet = nullptr);
        void LightingPassIBL(const VK_FrameInfo& frameInfo, float uMaxPrefilterMip,
                             std::shared_ptr<ResourceDescriptor> const& resourceDescriptorIBL,
                             VkDescriptorSet* lightingDescriptorSet = nullptr);
        float& Exposure() { return m_Exposure; }
        std::bitset<32>& ShaderSettings0() { return m_ShaderSettings0; }

    private:
        void CreateLightingPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void CreateLightingPipeline(VkRenderPass renderPass);

        void CreateLightingPipelineLayoutIBL(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
        void CreateLightingPipelineIBL(VkRenderPass renderPass);

    private:
        // constant ambient light
        VkPipelineLayout m_LightingPipelineLayout;
        std::unique_ptr<VK_Pipeline> m_LightingPipeline;

        // IBL as ambient light
        VkPipelineLayout m_LightingPipelineLayoutIBL;
        std::unique_ptr<VK_Pipeline> m_LightingPipelineIBL;

        const VkDescriptorSet* m_LightingDescriptorSets;
        const VkDescriptorSet* m_ShadowMapDescriptorSets;

        float m_Exposure{1.0f};
        std::bitset<32> m_ShaderSettings0; // 32-bit mask, default constructor initializes all bits to zero
    };
} // namespace GfxRenderEngine
