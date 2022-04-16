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

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/renderer.h"
#include "platform/Vulkan/imguiEngine/imgui.h"

#include "systems/VKdefaultDiffuseMapSys.h"
#include "systems/VKpointLightSys.h"

#include "systems/VKpbrNoMapSys.h"
#include "systems/VKpbrDiffuseSys.h"
#include "systems/VKpbrDiffuseNormalSys.h"
#include "systems/VKpbrDiffuseNormalRoughnessMetallicSys.h"

#include "VKdevice.h"
#include "VKswapChain.h"
#include "VKdescriptor.h"
#include "VKtexture.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{
    class VK_Window;
    class VK_Renderer : public Renderer
    {

    public:

        VK_Renderer(VK_Window* window, std::shared_ptr<VK_Device> device);
        ~VK_Renderer() override;

        VK_Renderer(const VK_Renderer&) = delete;
        VK_Renderer& operator=(const VK_Renderer&) = delete;

        VkRenderPass GetSwapChainRenderPass() const { return m_SwapChain->GetRenderPass(); }
        float GetAspectRatio() const { return m_SwapChain->ExtentAspectRatio(); }
        float GetContextWidth() const { return m_SwapChain->Width(); }
        float GetContextHeight() const { return m_SwapChain->Height(); }
        bool FrameInProgress() const { return m_FrameInProgress; }

        VkCommandBuffer GetCurrentCommandBuffer() const;

        VkCommandBuffer BeginFrame();
        void EndFrame();
        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
        int GetFrameIndex() const;

        virtual void BeginFrame(Camera* camera, entt::registry& registry) override;
        virtual void Submit(entt::registry& registry) override;
        virtual void Submit(std::shared_ptr<ParticleSystem>& particleSystem) override;
        virtual void EndScene() override;

        void ToggleDebugWindow(const GenericCallback& callback = nullptr) { m_Imgui = Imgui::ToggleDebugWindow(callback); }

    public:

        static std::unique_ptr<VK_DescriptorPool> m_DescriptorPool;

    private:

        void CreateCommandBuffers();
        void FreeCommandBuffers();
        void RecreateSwapChain();
        void CompileShaders();

    private:

        VK_Window* m_Window;
        std::shared_ptr<VK_Device> m_Device;

        std::unique_ptr<VK_RenderSystemPbrNoMap>                          m_RenderSystemPbrNoMap;
        std::unique_ptr<VK_RenderSystemPbrDiffuse>                        m_RenderSystemPbrDiffuse;
        std::unique_ptr<VK_RenderSystemPbrDiffuseNormal>                  m_RenderSystemPbrDiffuseNormal;
        std::unique_ptr<VK_RenderSystemPbrDiffuseNormalRoughnessMetallic> m_RenderSystemPbrDiffuseNormalRoughnessMetallic;

        std::unique_ptr<VK_RenderSystemDefaultDiffuseMap> m_RenderSystemDefaultDiffuseMap;
        std::unique_ptr<VK_PointLightSystem> m_PointLightSystem;
        std::shared_ptr<Imgui> m_Imgui;
        Camera* m_Camera;

        std::unique_ptr<VK_SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;
        VkCommandBuffer m_CurrentCommandBuffer;

        uint m_CurrentImageIndex;
        int m_CurrentFrameIndex;
        bool m_FrameInProgress;
        VK_FrameInfo m_FrameInfo;

        std::vector<VkDescriptorSet> m_GlobalDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> m_LocalDescriptorSets{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};
        std::vector<std::unique_ptr<VK_Buffer>> m_UniformBuffers{VK_SwapChain::MAX_FRAMES_IN_FLIGHT};

    };
}
