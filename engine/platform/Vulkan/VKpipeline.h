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

#include <string>
#include <memory>

#include "engine.h"

#include "VKdevice.h"

namespace GfxRenderEngine
{
    struct PipelineConfigInfo
    {
        // input assembly stage
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint subpass = 0;

        std::vector<VkVertexInputBindingDescription> m_BindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions{};
    };

    class VK_Pipeline
    {

    public:
        VK_Pipeline(VK_Device* device, const std::string& filePathVertexShader_SPV,
                    const std::string& filePathFragmentShader_SPV, const PipelineConfigInfo& spec);
        ~VK_Pipeline();

        VK_Pipeline(const VK_Pipeline&) = delete;
        VK_Pipeline& operator=(const VK_Pipeline&) = delete;

        void Bind(VkCommandBuffer commandBuffer);

        static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        static void SetColorBlendState(PipelineConfigInfo& configInfo, int attachmentCount,
                                       const VkPipelineColorBlendAttachmentState* blendAttachments);

    private:
        static std::vector<char> readFile(const std::string& filepath);
        void CreateGraphicsPipeline(const std::string& filePathVertexShader_SPV,
                                    const std::string& filePathFragmentShader_SPV, const PipelineConfigInfo& configInfo);
        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    private:
        VK_Device* m_Device;
        VkPipeline m_GraphicsPipeline{nullptr};
        VkShaderModule m_VertShaderModule{nullptr};
        VkShaderModule m_FragShaderModule{nullptr};
    };
} // namespace GfxRenderEngine
