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
#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/renderer.h"
#include "platform/Vulkan/imguiEngine/imgui.h"

#include "systems/VKshadowRenderSys.h"
#include "systems/VKshadowAnimatedRenderSys.h"
#include "systems/VKspriteRenderSys.h"
#include "systems/VKcubemapRenderSys.h"
#include "systems/VKspriteRenderSys2D.h"
#include "systems/VKdebugRenderSys.h"
#include "systems/VKlightSys.h"
#include "systems/VKguiRenderSys.h"

#include "systems/VKpbrNoMapSys.h"
#include "systems/VKpbrDiffuseSys.h"
#include "systems/VKpbrEmissiveSys.h"
#include "systems/VKpbrDiffuseSASys.h"
#include "systems/VKpbrDiffuseNormalSys.h"
#include "systems/VKpbrEmissiveTextureSys.h"
#include "systems/VKpbrDiffuseNormalRoughnessMetallicSys.h"
#include "systems/bloom/VKbloomRenderSystem.h"
#include "systems/VKpostprocessingSys.h"
#include "systems/VKdeferredShading.h"

#include "VKdevice.h"
#include "VKswapChain.h"
#include "VKrenderPass.h"
#include "VKshadowMap.h"
#include "VKdescriptor.h"
#include "VKtexture.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{
    class VK_Window;
    class VK_Renderer : public Renderer
    {

    public:

        VK_Renderer(VK_Window* window);
        ~VK_Renderer() override;

        VK_Renderer(const VK_Renderer&) = delete;
        VK_Renderer& operator=(const VK_Renderer&) = delete;

        float GetAspectRatio() const { return m_SwapChain->ExtentAspectRatio(); }
        uint GetContextWidth() const { return m_SwapChain->Width(); }
        uint GetContextHeight() const { return m_SwapChain->Height(); }
        bool FrameInProgress() const { return m_FrameInProgress; }

        VkCommandBuffer GetCurrentCommandBuffer() const;

        VkCommandBuffer BeginFrame();
        void EndFrame();
        void BeginShadowRenderPass0(VkCommandBuffer commandBuffer);
        void BeginShadowRenderPass1(VkCommandBuffer commandBuffer);
        void Begin3DRenderPass(VkCommandBuffer commandBuffer);
        void BeginPostProcessingRenderPass(VkCommandBuffer commandBuffer);
        void BeginGUIRenderPass(VkCommandBuffer commandBuffer);
        void EndRenderPass(VkCommandBuffer commandBuffer);
        int GetFrameIndex() const;

        virtual bool Init() override;
        virtual void BeginFrame(Camera* camera) override;
        virtual void Renderpass3D(entt::registry& registry) override;
        virtual void SubmitShadows(entt::registry& registry, const std::vector<DirectionalLightComponent*>& directionalLights = {}) override;
        virtual void Submit(entt::registry& registry, TreeNode& sceneHierarchy) override;
        virtual void NextSubpass() override;
        virtual void LightingPass() override;
        virtual void PostProcessingRenderpass() override;
        virtual void TransparencyPass(entt::registry& registry, ParticleSystem* particleSystem) override;
        virtual void Submit2D(Camera* camera, entt::registry& registry) override;
        virtual void GUIRenderpass(Camera* camera) override;
        virtual void EndScene() override;
        virtual uint GetFrameCounter() override { return m_FrameCounter; }
        virtual void SetAmbientLightIntensity(float ambientLightIntensity) override { m_AmbientLightIntensity = ambientLightIntensity; }
        virtual float GetAmbientLightIntensity() override { return m_AmbientLightIntensity; }
        virtual void DrawWithTransform(const Sprite& sprite, const glm::mat4& transform) override;
        virtual void Draw(const Sprite& sprite, const glm::mat4& position, const glm::vec4& color, const float textureID = 1.0f) override;
        virtual void ShowDebugShadowMap(bool showDebugShadowMap) override { m_ShowDebugShadowMap = showDebugShadowMap; }

        virtual void UpdateAnimations(entt::registry& registry) override { m_RenderSystemPbrDiffuseSA->UpdateAnimations(registry); };

        void ToggleDebugWindow(const GenericCallback& callback = nullptr) { m_Imgui = Imgui::ToggleDebugWindow(callback); }

    public:

        static std::unique_ptr<VK_DescriptorPool> m_DescriptorPool;

    private:

        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void RecreateSwapChain();
        void RecreateRenderpass();
        void RecreateShadowMaps();
        void CompileShaders();
        void UpdateTransformCache(entt::registry& registry, TreeNode& node, const glm::mat4& parentMat4, bool parentDirtyFlag);
        void CreateShadowMapDescriptorSets();
        void CreateLightingDescriptorSets();
        void CreatePostProcessingDescriptorSets();
        void CreateRenderSystemBloom();
        void Recreate();

    private:

        bool m_ShadersCompiled;
        VK_Window* m_Window;
        std::shared_ptr<VK_Device> m_Device;
        std::unique_ptr<VK_SwapChain> m_SwapChain;

        enum ShadowMaps
        {
            HIGH_RES = 0,
            LOW_RES,
            NUMBER_OF_SHADOW_MAPS
        };
        std::unique_ptr<VK_RenderPass> m_RenderPass;
        std::unique_ptr<VK_ShadowMap> m_ShadowMap[NUMBER_OF_SHADOW_MAPS];

        std::unique_ptr<VK_RenderSystemShadow>                            m_RenderSystemShadow;
        std::unique_ptr<VK_RenderSystemPbrNoMap>                          m_RenderSystemPbrNoMap;
        std::unique_ptr<VK_RenderSystemPbrDiffuse>                        m_RenderSystemPbrDiffuse;
        std::unique_ptr<VK_RenderSystemPbrEmissive>                       m_RenderSystemPbrEmissive;
        std::unique_ptr<VK_RenderSystemPbrDiffuseSA>                      m_RenderSystemPbrDiffuseSA;
        std::unique_ptr<VK_RenderSystemShadowAnimated>                    m_RenderSystemShadowAnimated;
        std::unique_ptr<VK_RenderSystemPbrDiffuseNormal>                  m_RenderSystemPbrDiffuseNormal;
        std::unique_ptr<VK_RenderSystemPbrEmissiveTexture>                m_RenderSystemPbrEmissiveTexture;
        std::unique_ptr<VK_RenderSystemPbrDiffuseNormalRoughnessMetallic> m_RenderSystemPbrDiffuseNormalRoughnessMetallic;
        std::unique_ptr<VK_RenderSystemDeferredShading>                   m_RenderSystemDeferredShading;
        std::unique_ptr<VK_RenderSystemPostProcessing>                    m_RenderSystemPostProcessing;
        std::unique_ptr<VK_RenderSystemBloom>                             m_RenderSystemBloom;
        std::unique_ptr<VK_RenderSystemCubemap>                           m_RenderSystemCubemap;
        std::unique_ptr<VK_RenderSystemSpriteRenderer>                    m_RenderSystemSpriteRenderer;
        std::unique_ptr<VK_RenderSystemSpriteRenderer2D>                  m_RenderSystemSpriteRenderer2D;
        std::unique_ptr<VK_RenderSystemGUIRenderer>                       m_RenderSystemGUIRenderer;
        std::unique_ptr<VK_RenderSystemDebug>                             m_RenderSystemDebug;
        std::unique_ptr<VK_LightSystem>                                   m_LightSystem;

        std::shared_ptr<Imgui> m_Imgui;

        std::vector<VkCommandBuffer> m_CommandBuffers;
        VkCommandBuffer m_CurrentCommandBuffer;
        VkDescriptorSetLayout m_GlobalDescriptorSetLayout;

        uint m_CurrentImageIndex;
        int m_CurrentFrameIndex;
        uint m_FrameCounter;
        bool m_FrameInProgress;
        VK_FrameInfo m_FrameInfo;

        std::unique_ptr<VK_DescriptorSetLayout> m_ShadowMapDescriptorSetLayout;
        std::unique_ptr<VK_DescriptorSetLayout> m_LightingDescriptorSetLayout;
        std::unique_ptr<VK_DescriptorSetLayout> m_PostProcessingDescriptorSetLayout;

        std::vector<VkDescriptorSet> m_ShadowDescriptorSets0{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_ShadowDescriptorSets1{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_GlobalDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_LocalDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VK_Buffer>> m_UniformBuffers{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VK_Buffer>> m_ShadowUniformBuffers0{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VK_Buffer>> m_ShadowUniformBuffers1{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_ShadowMapDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_LightingDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_PostProcessingDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};

        float m_AmbientLightIntensity;
        glm::mat4 m_GUIViewProjectionMatrix;

        bool m_ShowDebugShadowMap;
    };
}
