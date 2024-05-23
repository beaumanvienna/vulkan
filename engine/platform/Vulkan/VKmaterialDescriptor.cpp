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

#include "VKmaterialDescriptor.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"
#include "VKtexture.h"
#include "VKcubemap.h"
#include "VKbuffer.h"

namespace GfxRenderEngine
{

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialType materialType) : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialType::MtPbrNoMap:
            case MaterialType::MtPbrEmissive:
            {
                // nothing to be done
                break;
            }
            default:
            {
                CORE_ASSERT(false, "unsupported material type");
                break;
            }
        }
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialType materialType, std::vector<std::shared_ptr<Buffer>>& buffers)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialType::MtPbrNoMapInstanced:
            case MaterialType::MtPbrEmissiveInstanced:
            {
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            default:
            {
                CORE_ASSERT(false, "unsupported material type");
                break;
            }
        }
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialType materialType, std::vector<std::shared_ptr<Texture>>& textures)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialType::MtPbrDiffuseMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];

                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                const auto& imageInfo = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, imageInfo)
                    .Build(m_DescriptorSet);
                break;
            }
            case MaterialType::MtPbrDiffuseNormalMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];

                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, imageInfo0)
                    .WriteImage(1, imageInfo1)
                    .Build(m_DescriptorSet);
                break;
            }
            case MaterialType::MtPbrEmissiveTexture:
            {
                std::shared_ptr<Texture>& emissiveMap = textures[0];

                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                const auto& imageInfo = static_cast<VK_Texture*>(emissiveMap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, imageInfo)
                    .Build(m_DescriptorSet);
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallicMap: // gltf files have 3 textures
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMetallicMap = textures[2];

                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, imageInfo0)
                    .WriteImage(1, imageInfo1)
                    .WriteImage(2, imageInfo2)
                    .Build(m_DescriptorSet);
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallic2Map: // fbx files have 4 textures (grey scale images for
                                                                        // metallic and roughness textures)
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMap = textures[2];
                std::shared_ptr<Texture>& metallicMap = textures[3];

                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMap.get())->GetDescriptorImageInfo();
                auto& imageInfo3 = static_cast<VK_Texture*>(metallicMap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, imageInfo0)
                    .WriteImage(1, imageInfo1)
                    .WriteImage(2, imageInfo2)
                    .WriteImage(3, imageInfo3)
                    .Build(m_DescriptorSet);
                break;
            }
            case MaterialType::MtPbrNoMap:    // use the other constructor without textures
            case MaterialType::MtPbrEmissive: // use the other constructor without textures
            default:
            {
                CORE_ASSERT(false, "unsupported material type");
                break;
            }
        }
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialType materialType, std::vector<std::shared_ptr<Texture>>& textures,
                                                 std::vector<std::shared_ptr<Buffer>>& buffers)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialType::MtPbrDiffuseSAMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    const auto& imageInfo = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo)
                        .WriteBuffer(1, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }

                break;
            }
            case MaterialType::MtPbrDiffuseSAMapInstanced:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];
                std::shared_ptr<Buffer>& ubo = buffers[1];

                auto buffer = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                auto bufferInstanced = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfoInstanced = bufferInstanced->DescriptorInfo();

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    const auto& imageInfo = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo)
                        .WriteBuffer(1, bufferInfo)
                        .WriteBuffer(2, bufferInfoInstanced)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .WriteBuffer(1, bufferInfoInstanced)
                        .Build(m_ShadowDescriptorSet);
                }

                break;
            }
            case MaterialType::MtPbrDiffuseNormalSAMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteBuffer(2, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalSAMapInstanced:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];
                std::shared_ptr<Buffer>& ubo = buffers[1];

                auto buffer = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                auto bufferInstanced = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfoInstanced = bufferInstanced->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteBuffer(2, bufferInfo)
                        .WriteBuffer(3, bufferInfoInstanced)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .WriteBuffer(1, bufferInfoInstanced)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallicSAMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMetallicMap = textures[2];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];
                std::shared_ptr<Buffer>& uboInstanced = buffers[1];

                auto bufferSA = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                auto bufferInstanced = static_cast<VK_Buffer*>(uboInstanced.get());
                VkDescriptorBufferInfo bufferInfoInstanced = bufferInstanced->DescriptorInfo();
                VkDescriptorBufferInfo bufferInfoSA = bufferSA->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfoDiffuse = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfoNormal = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfoRoughnessMetallic =
                        static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfoDiffuse)
                        .WriteImage(1, imageInfoNormal)
                        .WriteImage(2, imageInfoRoughnessMetallic)
                        .WriteBuffer(3, bufferInfoSA)
                        .WriteBuffer(4, bufferInfoInstanced)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfoSA)
                        .WriteBuffer(1, bufferInfoInstanced)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallicSA2Map:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMap = textures[2];
                std::shared_ptr<Texture>& metallicMap = textures[3];
                std::shared_ptr<Buffer>& skeletalAnimationUBO = buffers[0];
                std::shared_ptr<Buffer>& uboInstanced = buffers[1];

                auto bufferSA = static_cast<VK_Buffer*>(skeletalAnimationUBO.get());
                auto bufferInstanced = static_cast<VK_Buffer*>(uboInstanced.get());
                VkDescriptorBufferInfo bufferInfoInstanced = bufferInstanced->DescriptorInfo();
                VkDescriptorBufferInfo bufferInfoSA = bufferSA->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfoDiffuse = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfoNormal = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfoRoughness = static_cast<VK_Texture*>(roughnessMap.get())->GetDescriptorImageInfo();
                    auto& imageInfoMetallic = static_cast<VK_Texture*>(metallicMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfoDiffuse)
                        .WriteImage(1, imageInfoNormal)
                        .WriteImage(2, imageInfoRoughness)
                        .WriteImage(3, imageInfoMetallic)
                        .WriteBuffer(4, bufferInfoSA)
                        .WriteBuffer(5, bufferInfoInstanced)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfoSA)
                        .WriteBuffer(1, bufferInfoInstanced)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrEmissiveTextureInstanced:
            {
                std::shared_ptr<Texture>& emissiveMap = textures[0];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    const auto& imageInfo = static_cast<VK_Texture*>(emissiveMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo)
                        .WriteBuffer(1, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseMapInstanced:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteBuffer(1, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalMapInstanced:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteBuffer(2, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallicMapInstanced:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMetallicMap = textures[2];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteImage(2, imageInfo2)
                        .WriteBuffer(3, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrMap:
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMetallicMap = textures[2];
                std::shared_ptr<Texture>& emissiveMap = textures[3];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .AddBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo3 = static_cast<VK_Texture*>(emissiveMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteImage(2, imageInfo2)
                        .WriteImage(3, imageInfo3)
                        .WriteBuffer(4, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            case MaterialType::MtPbrDiffuseNormalRoughnessMetallic2MapInstanced: // fbx files have 4 textures (grey scale
                                                                                 // images for metallic and roughness
                                                                                 // textures)
            {
                std::shared_ptr<Texture>& diffuseMap = textures[0];
                std::shared_ptr<Texture>& normalMap = textures[1];
                std::shared_ptr<Texture>& roughnessMap = textures[2];
                std::shared_ptr<Texture>& metallicMap = textures[3];
                std::shared_ptr<Buffer>& ubo = buffers[0];

                auto buffer = static_cast<VK_Buffer*>(ubo.get());
                VkDescriptorBufferInfo bufferInfo = buffer->DescriptorInfo();
                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .AddBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo3 = static_cast<VK_Texture*>(metallicMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteImage(2, imageInfo2)
                        .WriteImage(3, imageInfo3)
                        .WriteBuffer(4, bufferInfo)
                        .Build(m_DescriptorSet);
                }

                {
                    std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                        VK_DescriptorSetLayout::Builder()
                            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                            .Build();

                    VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                        .WriteBuffer(0, bufferInfo)
                        .Build(m_ShadowDescriptorSet);
                }
                break;
            }
            default:
            {
                CORE_ASSERT(false, "unsupported material type");
                break;
            }
        }
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialType materialType, std::shared_ptr<Cubemap> const& cubemap)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialType::MtCubemap:
            {
                std::unique_ptr<VK_DescriptorSetLayout> localDescriptorSetLayout =
                    VK_DescriptorSetLayout::Builder()
                        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                        .Build();

                VkDescriptorImageInfo cubemapInfo = static_cast<VK_Cubemap*>(cubemap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(*localDescriptorSetLayout, *VK_Renderer::m_DescriptorPool)
                    .WriteImage(0, cubemapInfo)
                    .Build(m_DescriptorSet);
                break;
            }
            default:
            {
                CORE_ASSERT(false, "unsupported material type");
                break;
            }
        }
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(VK_MaterialDescriptor const& other)
    {
        m_MaterialType = other.m_MaterialType;
        m_DescriptorSet = other.m_DescriptorSet;
        m_ShadowDescriptorSet = other.m_ShadowDescriptorSet;
    }

    VK_MaterialDescriptor::~VK_MaterialDescriptor() {}

    MaterialDescriptor::MaterialType VK_MaterialDescriptor::GetMaterialType() const { return m_MaterialType; }

    const VkDescriptorSet& VK_MaterialDescriptor::GetDescriptorSet() const { return m_DescriptorSet; }

    const VkDescriptorSet& VK_MaterialDescriptor::GetShadowDescriptorSet() const { return m_ShadowDescriptorSet; }
} // namespace GfxRenderEngine
