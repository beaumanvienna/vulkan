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
#include "VKrenderer.h"
#include "VKmaterialDescriptor.h"
#include "VKdescriptor.h"
#include "VKrenderer.h"
#include "VKtexture.h"
#include "VKcubemap.h"

namespace GfxRenderEngine
{

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialDescriptor::MaterialType materialType,
                                                 Material::MaterialTextures& textures)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialDescriptor::MaterialType::MtPbr:
            {
                auto renderer = static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer());
                auto textureAtlas = renderer->gTextureAtlas;
                // textures
                std::shared_ptr<Texture> diffuseMap;
                std::shared_ptr<Texture> normalMap;
                std::shared_ptr<Texture> roughnessMetallicMap;
                std::shared_ptr<Texture> emissiveMap;
                std::shared_ptr<Texture> roughnessMap;
                std::shared_ptr<Texture> metallicMap;
                std::shared_ptr<Texture>& dummy = textureAtlas;

                diffuseMap = textures[Material::DIFFUSE_MAP_INDEX] ? textures[Material::DIFFUSE_MAP_INDEX] : dummy;
                normalMap = textures[Material::NORMAL_MAP_INDEX] ? textures[Material::NORMAL_MAP_INDEX] : dummy;
                roughnessMetallicMap = textures[Material::ROUGHNESS_METALLIC_MAP_INDEX]
                                           ? textures[Material::ROUGHNESS_METALLIC_MAP_INDEX]
                                           : dummy;
                emissiveMap = textures[Material::EMISSIVE_MAP_INDEX] ? textures[Material::EMISSIVE_MAP_INDEX] : dummy;
                roughnessMap = textures[Material::ROUGHNESS_MAP_INDEX] ? textures[Material::ROUGHNESS_MAP_INDEX] : dummy;
                metallicMap = textures[Material::METALLIC_MAP_INDEX] ? textures[Material::METALLIC_MAP_INDEX] : dummy;

                {
                    auto& imageInfo0 = static_cast<VK_Texture*>(diffuseMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo1 = static_cast<VK_Texture*>(normalMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo2 = static_cast<VK_Texture*>(roughnessMetallicMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo3 = static_cast<VK_Texture*>(emissiveMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo4 = static_cast<VK_Texture*>(roughnessMap.get())->GetDescriptorImageInfo();
                    auto& imageInfo5 = static_cast<VK_Texture*>(metallicMap.get())->GetDescriptorImageInfo();

                    VK_DescriptorWriter(GetMaterialDescriptorSetLayout(materialType))
                        .WriteImage(0, imageInfo0)
                        .WriteImage(1, imageInfo1)
                        .WriteImage(2, imageInfo2)
                        .WriteImage(3, imageInfo3)
                        .WriteImage(4, imageInfo4)
                        .WriteImage(5, imageInfo5)
                        .Build(m_DescriptorSet);
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

    VK_MaterialDescriptor::VK_MaterialDescriptor(MaterialDescriptor::MaterialType materialType,
                                                 std::shared_ptr<Cubemap> const& cubemap)
        : m_MaterialType{materialType}
    {
        switch (materialType)
        {
            case MaterialDescriptor::MaterialType::MtCubemap:
            {
                VkDescriptorImageInfo cubemapInfo = static_cast<VK_Cubemap*>(cubemap.get())->GetDescriptorImageInfo();

                VK_DescriptorWriter(GetMaterialDescriptorSetLayout(materialType))
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
    }

    VK_MaterialDescriptor::VK_MaterialDescriptor(std::shared_ptr<MaterialDescriptor> const& materialDescriptor)
    {
        if (materialDescriptor)
        {
            VK_MaterialDescriptor* other = static_cast<VK_MaterialDescriptor*>(materialDescriptor.get());
            m_MaterialType = other->m_MaterialType;
            m_DescriptorSet = other->m_DescriptorSet;
        }
    }

    VK_MaterialDescriptor::~VK_MaterialDescriptor() {}

    MaterialDescriptor::MaterialType VK_MaterialDescriptor::GetMaterialType() const { return m_MaterialType; }

    const VkDescriptorSet& VK_MaterialDescriptor::GetDescriptorSet() const { return m_DescriptorSet; }

    VK_DescriptorSetLayout&
    VK_MaterialDescriptor::GetMaterialDescriptorSetLayout(MaterialDescriptor::MaterialType materialType)
    {
        auto renderer = static_cast<VK_Renderer*>(Engine::m_Engine->GetRenderer());
        return renderer->GetMaterialDescriptorSetLayout(materialType);
    }

} // namespace GfxRenderEngine
