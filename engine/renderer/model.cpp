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

#include <memory>
#include <iostream>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "gtc/type_ptr.hpp"
#include "stb_image.h"

#include "core.h"
#include "VKmodel.h"
#include "renderer/model.h"
#include "auxiliary/hash.h"
#include "auxiliary/file.h"
#include "auxiliary/math.h"
#include "scene/scene.h"

namespace std
{
    template <>
    struct hash<GfxRenderEngine::Vertex>
    {
        size_t operator()(GfxRenderEngine::Vertex const &vertex) const
        {
            size_t seed = 0;
            GfxRenderEngine::HashCombine(seed, vertex.m_Position, vertex.m_Color, vertex.m_Normal, vertex.m_UV);
            return seed;
        }
    };
}

namespace GfxRenderEngine
{
    Model::~Model() {}

    PrimitiveNoMap::~PrimitiveNoMap() {}

    PrimitiveEmissive::~PrimitiveEmissive() {}

    PrimitiveDiffuseMap::~PrimitiveDiffuseMap() {}

    PrimitiveEmissiveTexture::~PrimitiveEmissiveTexture() {}

    PrimitiveDiffuseNormalMap::~PrimitiveDiffuseNormalMap() {}

    PrimitiveDiffuseNormalRoughnessMetallicMap::~PrimitiveDiffuseNormalRoughnessMetallicMap() {}
    
    PrimitiveCubemap::~PrimitiveCubemap() {}

    float Model::m_NormalMapIntensity = 1.0f;

    bool Vertex::operator==(const Vertex& other) const
    {
        return (m_Position    == other.m_Position) &&
               (m_Color       == other.m_Color) &&
               (m_Normal      == other.m_Normal) &&
               (m_UV          == other.m_UV) &&
               (m_Amplification == other.m_Amplification) &&
               (m_Unlit       == other.m_Unlit);
    }

    Builder::Builder(const std::string& filepath)
        : m_Filepath(filepath), m_Transform(nullptr)
    {
        m_Basepath = EngineCore::GetPathWithoutFilename(filepath);
    }

    void Builder::LoadImagesGLTF()
    {
        m_ImageOffset = m_Images.size();
        // retrieve all images from the glTF file
        for (uint i = 0; i < m_GltfModel.images.size(); i++)
        {
            std::string imageFilepath = m_Basepath + m_GltfModel.images[i].uri;
            tinygltf::Image& glTFImage = m_GltfModel.images[i];

            // glTFImage.component - the number of channels in each pixel
            // three channels per pixel need to be converted to four channels per pixel
            uchar* buffer;
            uint64 bufferSize;
            if (glTFImage.component == 3)
            {
                bufferSize = glTFImage.width * glTFImage.height * 4;
                std::vector<uchar> imageData(bufferSize, 0x00);

                buffer = (uchar*)imageData.data();
                uchar* rgba = buffer;
                uchar* rgb = &glTFImage.image[0];
                for (int j = 0; j < glTFImage.width * glTFImage.height; ++j)
                {
                    memcpy(rgba, rgb, sizeof(uchar) * 3);
                    rgba += 4;
                    rgb += 3;
                }
            }
            else
            {
                buffer = &glTFImage.image[0];
                bufferSize = glTFImage.image.size();
            }
            auto texture = Texture::Create();
            texture->Init(glTFImage.width, glTFImage.height, GetImageFormatGLTF(i), buffer);
            m_Images.push_back(texture);
        }
    }

    bool Builder::GetImageFormatGLTF(uint imageIndex)
    {
        for (uint i = 0; i < m_GltfModel.materials.size(); i++)
        {
            tinygltf::Material glTFMaterial = m_GltfModel.materials[i];

            if (static_cast<uint>(glTFMaterial.pbrMetallicRoughness.baseColorTexture.index) == imageIndex)
            {
                return Texture::USE_SRGB;
            }
            else if (static_cast<uint>(glTFMaterial.emissiveTexture.index) == imageIndex)
            {
                return Texture::USE_SRGB;
            }
            else if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end())
            {
                int diffuseTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
                tinygltf::Texture& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                if (static_cast<uint>(diffuseTexture.source) == imageIndex)
                {
                    return Texture::USE_SRGB;
                }
            }
        }
        return Texture::USE_UNORM;
    }

    void Builder::LoadMaterialsGLTF()
    {
        m_Materials.clear();
        for (uint i = 0; i < m_GltfModel.materials.size(); i++)
        {
            tinygltf::Material glTFMaterial = m_GltfModel.materials[i];

            Material material{};
            material.m_DiffuseColor = glm::vec3(0.5f, 0.5f, 1.0f);
            material.m_Roughness = glTFMaterial.pbrMetallicRoughness.roughnessFactor;
            material.m_Metallic  = glTFMaterial.pbrMetallicRoughness.metallicFactor;
            material.m_NormalMapIntensity = glTFMaterial.normalTexture.scale;
            material.m_EmissiveStrength = 0;
            if (glTFMaterial.emissiveFactor.size() == 3)
            {
                glm::vec3 emissiveFactor = glm::make_vec3(glTFMaterial.emissiveFactor.data());
                if (emissiveFactor != glm::vec3(0,0,0))
                {
                    material.m_EmissiveFactor = emissiveFactor;
                    material.m_EmissiveStrength = 1;
                }
            }
            if (glTFMaterial.emissiveTexture.index != -1)
            {
                int emissiveTextureIndex = glTFMaterial.emissiveTexture.index;
                tinygltf::Texture& emissiveTexture = m_GltfModel.textures[emissiveTextureIndex];
                material.m_EmissiveMapIndex = emissiveTexture.source;
                material.m_Features |= Material::HAS_EMISSIVE_MAP;
                material.m_EmissiveStrength = 1;
            }
            {
                auto it = glTFMaterial.extensions.find("KHR_materials_emissive_strength");
                if (it != glTFMaterial.extensions.end())
                {
                    auto extension = it->second;
                    if (extension.IsObject())
                    {
                        auto emissiveStrength = extension.Get("emissiveStrength");
                        if (emissiveStrength.IsReal())
                        {
                            material.m_EmissiveStrength = emissiveStrength.GetNumberAsDouble();
                        }
                    }
                }
            }

            if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end())
            {
                material.m_DiffuseColor = glm::make_vec3(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
            }
            if (glTFMaterial.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                int diffuseTextureIndex = glTFMaterial.pbrMetallicRoughness.baseColorTexture.index;
                tinygltf::Texture& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                material.m_DiffuseMapIndex = diffuseTexture.source;
                material.m_Features |= Material::HAS_DIFFUSE_MAP;
            }
            else if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end())
            {
                LOG_CORE_WARN("using legacy field values/baseColorTexture");
                int diffuseTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
                tinygltf::Texture& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                material.m_DiffuseMapIndex = diffuseTexture.source;
                material.m_Features |= Material::HAS_DIFFUSE_MAP;
            }
            if (glTFMaterial.normalTexture.index != -1)
            {
                int normalTextureIndex = glTFMaterial.normalTexture.index;
                tinygltf::Texture& normalTexture = m_GltfModel.textures[normalTextureIndex];
                material.m_NormalMapIndex = normalTexture.source;
                material.m_Features |= Material::HAS_NORMAL_MAP;
            }
            if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                int mettalicRoughnessTextureIndex = glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
                tinygltf::Texture& mettalicRoughnessTexture = m_GltfModel.textures[mettalicRoughnessTextureIndex];
                material.m_RoughnessMettalicMapIndex = mettalicRoughnessTexture.source;
                material.m_Features |= Material::HAS_ROUGHNESS_METALLIC_MAP;
            }

            m_Materials.push_back(material);
        }
    }

    void Builder::LoadVertexDataGLTF(uint meshIndex)
    {
        // handle vertex data
        m_Vertices.clear();
        m_Indices.clear();

        m_PrimitivesNoMap.clear();
        m_PrimitivesEmissive.clear();
        m_PrimitivesDiffuseMap.clear();
        m_PrimitivesEmissiveTexture.clear();
        m_PrimitivesDiffuseNormalMap.clear();
        m_PrimitivesDiffuseNormalRoughnessMetallicMap.clear();

        for (const auto& glTFPrimitive : m_GltfModel.meshes[meshIndex].primitives)
        {

            PrimitiveTmp primitiveTmp;
            primitiveTmp.m_FirstVertex = static_cast<uint32_t>(m_Vertices.size());
            primitiveTmp.m_FirstIndex  = static_cast<uint32_t>(m_Indices.size());

            uint vertexCount = 0;
            uint indexCount  = 0;

            glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 1.0f);
            if (glTFPrimitive.material != -1)
            {
                if (!(static_cast<size_t>(glTFPrimitive.material) < m_Materials.size()))
                {
                    LOG_CORE_CRITICAL("LoadVertexDataGLTF: glTFPrimitive.material must be less than m_Materials.size()");
                }
                diffuseColor = m_Materials[glTFPrimitive.material].m_DiffuseColor;
            }
            // Vertices
            {
                const float* positionBuffer  = nullptr;
                const float* normalsBuffer   = nullptr;
                const float* tangentsBuffer  = nullptr;
                const float* texCoordsBuffer = nullptr;

                // Get buffer data for vertex positions
                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = m_GltfModel.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = m_GltfModel.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                #define USE_TINYGLTF_TANGENTS
                #ifdef USE_TINYGLTF_TANGENTS
                    // Get buffer data for vertex tangents
                    if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor = m_GltfModel.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
                        const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
                        tangentsBuffer = reinterpret_cast<const float*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                #endif
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = m_GltfModel.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // Append data to model's vertex buffer
                for (size_t v = 0; v < vertexCount; v++)
                {
                    Vertex vertex{};
                    vertex.m_Amplification      = 1.0f;
                    auto position = glm::make_vec3(&positionBuffer[v * 3]);
                    vertex.m_Position = glm::vec4(position.x, position.y, position.z, 1.0f);
                    vertex.m_Normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));

                    glm::vec4 t = glm::vec4(tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f));
                    vertex.m_Tangent = glm::vec3(t.x, t.y, t.z) * t.w;

                    vertex.m_UV = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                    vertex.m_Color = diffuseColor;
                    m_Vertices.push_back(vertex);
                }

                // calculate tangents
                if (!tangentsBuffer)
                {
                    CalculateTangents();
                }

            }
            // Indices
            {
                const tinygltf::Accessor& accessor = m_GltfModel.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView = m_GltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = m_GltfModel.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_Indices.push_back(buf[index]);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_Indices.push_back(buf[index]);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++) {
                        m_Indices.push_back(buf[index]);
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
                }
            }

            primitiveTmp.m_VertexCount = vertexCount;
            primitiveTmp.m_IndexCount  = indexCount;

            AssignMaterial(primitiveTmp, glTFPrimitive.material);
        }

    }

    void Builder::LoadTransformationMatrix(TransformComponent& transform, int nodeIndex)
    {
        auto& node = m_GltfModel.nodes[nodeIndex];

        if (node.matrix.size() == 16)
        {
            transform.SetMat4(glm::make_mat4x4(node.matrix.data()));
        }
        else
        {
            if (node.rotation.size() == 4)
            {
                float x = node.rotation[0];
                float y = node.rotation[1];
                float z = node.rotation[2];
                float w = node.rotation[3];

                transform.SetRotation({w, x, y, z});

            }
            if (node.scale.size() == 3)
            {
                transform.SetScale({node.scale[0], node.scale[1], node.scale[2]});
            }
            if (node.translation.size() == 3)
            {
                transform.SetTranslation({node.translation[0], node.translation[1], node.translation[2]});
            }
        }
    }

    void Builder::AssignMaterial(const PrimitiveTmp& primitiveTmp, int materialIndex)
    {
        if (materialIndex == -1)
        {
            PrimitiveNoMap primitiveNoMap{};
            primitiveNoMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveNoMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveNoMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveNoMap.m_VertexCount = primitiveTmp.m_VertexCount;

            primitiveNoMap.m_PbrNoMapMaterial.m_Roughness = 0.5f;
            primitiveNoMap.m_PbrNoMapMaterial.m_Metallic  = 0.1f;
            primitiveNoMap.m_PbrNoMapMaterial.m_Color     = glm::vec3(0.5f, 0.5f, 1.0f);

            m_PrimitivesNoMap.push_back(primitiveNoMap);

            return;
        }
        if (!(static_cast<size_t>(materialIndex) < m_Materials.size()))
        {
            LOG_CORE_CRITICAL("LoadVertexDataGLTF: materialIndex must be less than m_Materials.size()");
        }
        auto& material = m_Materials[materialIndex];

        uint pbrFeatures = material.m_Features & (
                Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP);
        if (pbrFeatures == Material::HAS_DIFFUSE_MAP)
        {
            PrimitiveDiffuseMap primitiveDiffuseMap{};
            primitiveDiffuseMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveDiffuseMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveDiffuseMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveDiffuseMap.m_VertexCount = primitiveTmp.m_VertexCount;

            uint diffuseMapIndex = m_ImageOffset + material.m_DiffuseMapIndex;
            ASSERT(diffuseMapIndex < m_Images.size());

            VK_Model::CreateDescriptorSet(primitiveDiffuseMap.m_PbrDiffuseMaterial, m_Images[diffuseMapIndex]);
            primitiveDiffuseMap.m_PbrDiffuseMaterial.m_Roughness = material.m_Roughness;
            primitiveDiffuseMap.m_PbrDiffuseMaterial.m_Metallic  = material.m_Metallic;

            m_PrimitivesDiffuseMap.push_back(primitiveDiffuseMap);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP))
        {
            PrimitiveDiffuseNormalMap primitiveDiffuseNormalMap{};
            primitiveDiffuseNormalMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveDiffuseNormalMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveDiffuseNormalMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveDiffuseNormalMap.m_VertexCount = primitiveTmp.m_VertexCount;

            uint diffuseMapIndex = m_ImageOffset + material.m_DiffuseMapIndex;
            uint normalMapIndex  = m_ImageOffset + material.m_NormalMapIndex;
            ASSERT(diffuseMapIndex < m_Images.size());
            ASSERT(normalMapIndex < m_Images.size());

            VK_Model::CreateDescriptorSet(primitiveDiffuseNormalMap.m_PbrDiffuseNormalMaterial, m_Images[diffuseMapIndex], m_Images[normalMapIndex]);
            primitiveDiffuseNormalMap.m_PbrDiffuseNormalMaterial.m_Roughness          = material.m_Roughness;
            primitiveDiffuseNormalMap.m_PbrDiffuseNormalMaterial.m_Metallic           = material.m_Metallic;
            primitiveDiffuseNormalMap.m_PbrDiffuseNormalMaterial.m_NormalMapIntensity = material.m_NormalMapIntensity;

            m_PrimitivesDiffuseNormalMap.push_back(primitiveDiffuseNormalMap);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            PrimitiveDiffuseNormalRoughnessMetallicMap primitiveDiffuseNormalRoughnessMetallicMap{};
            primitiveDiffuseNormalRoughnessMetallicMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveDiffuseNormalRoughnessMetallicMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveDiffuseNormalRoughnessMetallicMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveDiffuseNormalRoughnessMetallicMap.m_VertexCount = primitiveTmp.m_VertexCount;

            uint diffuseMapIndex           = m_ImageOffset + material.m_DiffuseMapIndex;
            uint normalMapIndex            = m_ImageOffset + material.m_NormalMapIndex;
            uint roughnessMettalicMapIndex = m_ImageOffset + material.m_RoughnessMettalicMapIndex;

            ASSERT(diffuseMapIndex            < m_Images.size());
            ASSERT(normalMapIndex             < m_Images.size());
            ASSERT(roughnessMettalicMapIndex  < m_Images.size());

            VK_Model::CreateDescriptorSet(primitiveDiffuseNormalRoughnessMetallicMap.m_PbrDiffuseNormalRoughnessMetallicMaterial,
                                          m_Images[diffuseMapIndex], 
                                          m_Images[normalMapIndex], 
                                          m_Images[roughnessMettalicMapIndex]);
            primitiveDiffuseNormalRoughnessMetallicMap.m_PbrDiffuseNormalRoughnessMetallicMaterial.m_NormalMapIntensity = material.m_NormalMapIntensity;

            m_PrimitivesDiffuseNormalRoughnessMetallicMap.push_back(primitiveDiffuseNormalRoughnessMetallicMap);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            LOG_CORE_CRITICAL("material diffuseRoughnessMetallic not supported");
        }
        else if (pbrFeatures & (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            PrimitiveDiffuseNormalRoughnessMetallicMap primitiveDiffuseNormalRoughnessMetallicMap{};
            primitiveDiffuseNormalRoughnessMetallicMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveDiffuseNormalRoughnessMetallicMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveDiffuseNormalRoughnessMetallicMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveDiffuseNormalRoughnessMetallicMap.m_VertexCount = primitiveTmp.m_VertexCount;

            uint diffuseMapIndex           = m_ImageOffset + material.m_DiffuseMapIndex;
            uint normalMapIndex            = m_ImageOffset + material.m_NormalMapIndex;
            uint roughnessMettalicMapIndex = m_ImageOffset + material.m_RoughnessMettalicMapIndex;
            ASSERT(diffuseMapIndex            < m_Images.size());
            ASSERT(normalMapIndex             < m_Images.size());
            ASSERT(roughnessMettalicMapIndex  < m_Images.size());

            VK_Model::CreateDescriptorSet(primitiveDiffuseNormalRoughnessMetallicMap.m_PbrDiffuseNormalRoughnessMetallicMaterial, 
                                            m_Images[diffuseMapIndex], m_Images[normalMapIndex], m_Images[roughnessMettalicMapIndex]);
            primitiveDiffuseNormalRoughnessMetallicMap.m_PbrDiffuseNormalRoughnessMetallicMaterial.m_NormalMapIntensity = material.m_NormalMapIntensity;

            m_PrimitivesDiffuseNormalRoughnessMetallicMap.push_back(primitiveDiffuseNormalRoughnessMetallicMap);
        }
        else if (pbrFeatures & Material::HAS_DIFFUSE_MAP)
        {
            PrimitiveDiffuseMap primitiveDiffuseMap{};
            primitiveDiffuseMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveDiffuseMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveDiffuseMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveDiffuseMap.m_VertexCount = primitiveTmp.m_VertexCount;

            uint diffuseMapIndex = m_ImageOffset + material.m_DiffuseMapIndex;
            ASSERT(diffuseMapIndex < m_Images.size());

            VK_Model::CreateDescriptorSet(primitiveDiffuseMap.m_PbrDiffuseMaterial, m_Images[diffuseMapIndex]);
            primitiveDiffuseMap.m_PbrDiffuseMaterial.m_Roughness = material.m_Roughness;
            primitiveDiffuseMap.m_PbrDiffuseMaterial.m_Metallic  = material.m_Metallic;

            m_PrimitivesDiffuseMap.push_back(primitiveDiffuseMap);
        }
        else
        {
            PrimitiveNoMap primitiveNoMap{};
            primitiveNoMap.m_FirstIndex  = primitiveTmp.m_FirstIndex;
            primitiveNoMap.m_FirstVertex = primitiveTmp.m_FirstVertex;
            primitiveNoMap.m_IndexCount  = primitiveTmp.m_IndexCount;
            primitiveNoMap.m_VertexCount = primitiveTmp.m_VertexCount;

            primitiveNoMap.m_PbrNoMapMaterial.m_Roughness = material.m_Roughness;
            primitiveNoMap.m_PbrNoMapMaterial.m_Metallic  = material.m_Metallic;
            primitiveNoMap.m_PbrNoMapMaterial.m_Color     = material.m_DiffuseColor;

            m_PrimitivesNoMap.push_back(primitiveNoMap);
        }

        // emissive materials
        if (material.m_EmissiveStrength != 0)
        {
            // emissive texture
            if (material.m_Features & Material::HAS_EMISSIVE_MAP)
            {
                PrimitiveEmissiveTexture primitiveEmissiveTexture{};
                primitiveEmissiveTexture.m_FirstIndex  = primitiveTmp.m_FirstIndex;
                primitiveEmissiveTexture.m_FirstVertex = primitiveTmp.m_FirstVertex;
                primitiveEmissiveTexture.m_IndexCount  = primitiveTmp.m_IndexCount;
                primitiveEmissiveTexture.m_VertexCount = primitiveTmp.m_VertexCount;

                uint emissiveMapIndex = m_ImageOffset + material.m_EmissiveMapIndex;
                ASSERT(emissiveMapIndex < m_Images.size());

                VK_Model::CreateDescriptorSet(primitiveEmissiveTexture.m_PbrEmissiveTextureMaterial, m_Images[emissiveMapIndex]);

                primitiveEmissiveTexture.m_PbrEmissiveTextureMaterial.m_Roughness = material.m_Roughness;
                primitiveEmissiveTexture.m_PbrEmissiveTextureMaterial.m_Metallic  = material.m_Metallic;
                primitiveEmissiveTexture.m_PbrEmissiveTextureMaterial.m_EmissiveStrength  = material.m_EmissiveStrength;

                m_PrimitivesEmissiveTexture.push_back(primitiveEmissiveTexture);
            }
            else // emissive vertex color
            {
                PrimitiveEmissive primitiveEmissive{};
                primitiveEmissive.m_FirstIndex  = primitiveTmp.m_FirstIndex;
                primitiveEmissive.m_FirstVertex = primitiveTmp.m_FirstVertex;
                primitiveEmissive.m_IndexCount  = primitiveTmp.m_IndexCount;
                primitiveEmissive.m_VertexCount = primitiveTmp.m_VertexCount;

                primitiveEmissive.m_PbrEmissiveMaterial.m_Roughness = material.m_Roughness;
                primitiveEmissive.m_PbrEmissiveMaterial.m_Metallic  = material.m_Metallic;
                primitiveEmissive.m_PbrEmissiveMaterial.m_EmissiveFactor = material.m_EmissiveFactor;
                primitiveEmissive.m_PbrEmissiveMaterial.m_EmissiveStrength  = material.m_EmissiveStrength;

                m_PrimitivesEmissive.push_back(primitiveEmissive);
            }
        }
    }

    entt::entity Builder::LoadGLTF(entt::registry& registry, TreeNode& sceneHierarchy, Dictionary& dictionary, TransformComponent* transform)
    {
        std::string warn, err;
        m_GameObject = entt::null;

        stbi_set_flip_vertically_on_load(false);

        if (!m_GltfLoader.LoadASCIIFromFile(&m_GltfModel, &err, &warn, m_Filepath))
        {
            LOG_CORE_CRITICAL("LoadGLTF errors: {0}, warnings: {1}", err, warn);
            return entt::null;
        }

        LoadImagesGLTF();
        LoadMaterialsGLTF();

        for (auto& scene : m_GltfModel.scenes)
        {
            TreeNode* currentNode = &sceneHierarchy;
            if (scene.nodes.size() > 1)
            {
                //this scene has multiple nodes -> create a group node
                auto entity = registry.create();
                if (m_GameObject == entt::null) m_GameObject = entity;
                TransformComponent l_transform{};
                registry.emplace<TransformComponent>(entity, l_transform);

                auto shortName = scene.name + "::root";
                auto longName = m_Filepath + std::string("::") + scene.name + std::string("::root");
                TreeNode sceneHierarchyNode{entity, shortName, longName};

                currentNode = currentNode->AddChild(sceneHierarchyNode, dictionary);
            }
            else if (scene.nodes.size() < 1)
            {
                LOG_CORE_WARN("Builder::LoadGLTF: empty scene in {0}", m_Filepath);
                return entt::null;
            }

            for (uint i = 0; i < scene.nodes.size(); i++)
            {
                ProcessNode(scene, scene.nodes[i], registry, dictionary, currentNode);
            }
        }
        return m_GameObject;
    }

    void Builder::ProcessNode(tinygltf::Scene& scene, uint nodeIndex, entt::registry& registry, Dictionary& dictionary, TreeNode* currentNode)
    {
        auto& node = m_GltfModel.nodes[nodeIndex];
        auto& nodeName = node.name;
        auto meshIndex = node.mesh;

        if (meshIndex == -1)
        {
            if (node.children.size())
            {
                auto entity = registry.create();
                if (m_GameObject == entt::null) m_GameObject = entity;
                TransformComponent transform{};
                LoadTransformationMatrix(transform, nodeIndex);
                registry.emplace<TransformComponent>(entity, transform);
                auto shortName = nodeName;
                auto longName = m_Filepath + std::string("::") + scene.name + std::string("::") + nodeName;
                TreeNode sceneHierarchyNode{entity, shortName, longName};

                TreeNode* groupNode = currentNode->AddChild(sceneHierarchyNode, dictionary);
                for (uint childNodeArrayIndex = 0; childNodeArrayIndex < node.children.size(); childNodeArrayIndex++)
                {
                    uint childNodeIndex = node.children[childNodeArrayIndex];
                    ProcessNode(scene, childNodeIndex, registry, dictionary, groupNode);
                }
            }
            else
            {
                LOG_CORE_WARN("No mesh and no children for node {0} in scene {1}, file {2}", nodeName, scene.name, m_Filepath);
            }
        }
        else
        {
            auto newNode = CreateGameObject(scene, nodeIndex, registry, dictionary, currentNode);
            if (node.children.size())
            {
                for (uint childNodeArrayIndex = 0; childNodeArrayIndex < node.children.size(); childNodeArrayIndex++)
                {
                    uint childNodeIndex = node.children[childNodeArrayIndex];
                    ProcessNode(scene, childNodeIndex, registry, dictionary, newNode);
                }
            }
        }
    }

    TreeNode* Builder::CreateGameObject(tinygltf::Scene& scene, uint nodeIndex, entt::registry& registry, Dictionary& dictionary, TreeNode* currentNode)
    {
        auto& node = m_GltfModel.nodes[nodeIndex];
        auto& nodeName = node.name;
        uint meshIndex = node.mesh;

        LoadVertexDataGLTF(meshIndex);
        LOG_CORE_INFO("Vertex count: {0}, Index count: {1} (file: {2}, node: {3})", m_Vertices.size(), m_Indices.size(), m_Filepath, nodeName);

        auto model = Engine::m_Engine->LoadModel(*this);
        auto entity = registry.create();

        auto longName = m_Filepath + std::string("::") + scene.name + std::string("::") + nodeName;

        if (m_GameObject == entt::null) m_GameObject = entity;

        TreeNode sceneHierarchyNode{entity, nodeName, longName};
        TreeNode* newNode = currentNode->AddChild(sceneHierarchyNode, dictionary);

        // mesh
        MeshComponent mesh{nodeName, model};
        registry.emplace<MeshComponent>(entity, mesh);

        // transform
        TransformComponent transform{};
        LoadTransformationMatrix(transform, nodeIndex);
        registry.emplace<TransformComponent>(entity, transform);

        // material tags (can have multiple tags)

        // vertex diffuse color, diffuse map, normal map, roughness/metallic map
        if (m_PrimitivesNoMap.size())
        {
            PbrNoMapTag pbrNoMapTag{};
            registry.emplace<PbrNoMapTag>(entity, pbrNoMapTag);
        }
        if (m_PrimitivesDiffuseMap.size())
        {
            PbrDiffuseTag pbrDiffuseTag{};
            registry.emplace<PbrDiffuseTag>(entity, pbrDiffuseTag);
        }
        if (m_PrimitivesDiffuseNormalMap.size())
        {
            PbrDiffuseNormalTag pbrDiffuseNormalTag;
            registry.emplace<PbrDiffuseNormalTag>(entity, pbrDiffuseNormalTag);
        }
        if (m_PrimitivesDiffuseNormalRoughnessMetallicMap.size())
        {
            PbrDiffuseNormalRoughnessMetallicTag pbrDiffuseNormalRoughnessMetallicTag;
            registry.emplace<PbrDiffuseNormalRoughnessMetallicTag>(entity, pbrDiffuseNormalRoughnessMetallicTag);
        }

        // emissive materials
        if (m_PrimitivesEmissive.size())
        {
            PbrEmissiveTag pbrEmissiveTag{};
            registry.emplace<PbrEmissiveTag>(entity, pbrEmissiveTag);
        }
        if (m_PrimitivesEmissiveTexture.size())
        {
            PbrEmissiveTextureTag pbrEmissiveTextureTag{};
            registry.emplace<PbrEmissiveTextureTag>(entity, pbrEmissiveTextureTag);
        }

        return newNode;
    }

    void Builder::LoadModel(const std::string &filepath, int fragAmplification)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
        {
            LOG_CORE_CRITICAL("LoadModel errors: {0}, warnings: {1}", err, warn);
        }

        m_Vertices.clear();
        m_Indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.m_Amplification      = fragAmplification;

                if (index.vertex_index >= 0)
                {
                    vertex.m_Position =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                       -attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };

                    vertex.m_Color =
                    {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0)
                {
                    vertex.m_Normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                       -attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.m_UV =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }
                m_Indices.push_back(uniqueVertices[vertex]);

            }
        }

        PrimitiveNoMap primitiveNoMap{};
        primitiveNoMap.m_FirstIndex  = 0;
        primitiveNoMap.m_FirstVertex = 0;
        primitiveNoMap.m_IndexCount  = m_Indices.size();
        primitiveNoMap.m_VertexCount = m_Vertices.size();

        primitiveNoMap.m_PbrNoMapMaterial.m_Roughness = 0.5f;
        primitiveNoMap.m_PbrNoMapMaterial.m_Metallic  = 0.1f;
        primitiveNoMap.m_PbrNoMapMaterial.m_Color     = glm::vec3(0.5f, 0.5f, 1.0f);

        m_PrimitivesNoMap.push_back(primitiveNoMap);

        // calculate tangents
        CalculateTangents();
        LOG_CORE_INFO("Vertex count: {0}, Index count: {1} ({2})", m_Vertices.size(), m_Indices.size(), filepath);
    }

    void Builder::CalculateTangents()
    {
        if (m_Indices.size())
        {
            CalculateTangentsFromIndexBuffer(m_Indices);
        }
        else
        {
            uint vertexCount = m_Vertices.size();
            if (vertexCount)
            {
                std::vector<uint> indices;
                indices.resize(vertexCount);
                for (uint i = 0; i < vertexCount; i++)
                {
                    indices[i] = i;
                }
                CalculateTangentsFromIndexBuffer(indices);
            }
        }
    }

    void Builder::CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices)
    {
        uint cnt = 0;
        uint vertexIndex1;
        uint vertexIndex2;
        uint vertexIndex3;
        glm::vec3 position1;
        glm::vec3 position2;
        glm::vec3 position3;
        glm::vec2 uv1;
        glm::vec2 uv2;
        glm::vec2 uv3;

        for (uint index : indices)
        {
            auto& vertex = m_Vertices[index];

            switch (cnt)
            {
                case 0:
                    position1 = vertex.m_Position;
                    uv1  = vertex.m_UV;
                    vertexIndex1 = index;
                    break;
                case 1:
                    position2 = vertex.m_Position;
                    uv2  = vertex.m_UV;
                    vertexIndex2 = index;
                    break;
                case 2:
                    position3 = vertex.m_Position;
                    uv3  = vertex.m_UV;
                    vertexIndex3 = index;

                    glm::vec3 edge1 = position2 - position1;
                    glm::vec3 edge2 = position3 - position1;
                    glm::vec2 deltaUV1 = uv2 - uv1;
                    glm::vec2 deltaUV2 = uv3 - uv1;

                    float dU1 = deltaUV1.x;
                    float dU2 = deltaUV2.x;
                    float dV1 = deltaUV1.y;
                    float dV2 = deltaUV2.y;
                    float E1x = edge1.x;
                    float E2x = edge2.x;
                    float E1y = edge1.y;
                    float E2y = edge2.y;
                    float E1z = edge1.z;
                    float E2z = edge2.z;

                    float factor;
                    if ((dU1 * dV2 - dU2 * dV1) != 0.0f)
                    {
                        factor = 1.0f / (dU1 * dV2 - dU2 * dV1);
                    }
                    else
                    {
                        factor = 100000.0f;
                    }

                    glm::vec3 tangent;

                    tangent.x = factor * (dV2 * E1x - dV1 * E2x);
                    tangent.y = factor * (dV2 * E1y - dV1 * E2y);
                    tangent.z = factor * (dV2 * E1z - dV1 * E2z);
                    if (tangent.x==0.0f && tangent.y==0.0f && tangent.z==0.0f) tangent = glm::vec3(1.0f, 0.0f, 0.0f);

                    m_Vertices[vertexIndex1].m_Tangent = tangent;
                    m_Vertices[vertexIndex2].m_Tangent = tangent;
                    m_Vertices[vertexIndex3].m_Tangent = tangent;

                    break;
            }
            cnt = (cnt + 1) % 3;
        }
    }

    void Builder::LoadSprite(const Sprite& sprite, float amplification, int unlit, const glm::vec4& color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        // 0 - 1
        // | / |
        // 3 - 2

        Vertex vertex[4];

        // index 0, 0.0f,  1.0f
        vertex[0] = {/*pos*/ {-1.0f,  1.0f, 0.0f}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos1X, sprite.m_Pos1Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 1, 1.0f,  1.0f
        vertex[1] = {/*pos*/ { 1.0f,  1.0f, 0.0f}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos2X, sprite.m_Pos1Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 2, 1.0f,  0.0f
        vertex[2] = {/*pos*/ { 1.0f, -1.0f, 0.0f}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos2X, sprite.m_Pos2Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 3, 0.0f,  0.0f
        vertex[3] = {/*pos*/ {-1.0f, -1.0f, 0.0f}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos1X, sprite.m_Pos2Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        for (int i = 0; i < 4; i++) m_Vertices.push_back(vertex[i]);

        m_Indices.push_back(0);
        m_Indices.push_back(1);
        m_Indices.push_back(3);
        m_Indices.push_back(1);
        m_Indices.push_back(2);
        m_Indices.push_back(3);
    }

    void Builder::LoadParticle(const glm::vec4& color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        // 0 - 1
        // | / |
        // 3 - 2

        Vertex vertex[4]
        {
            // index 0, 0.0f,  1.0f
            {/*pos*/ {-1.0f,  1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 1.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 1, 1.0f,  1.0f
            {/*pos*/ { 1.0f,  1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 1.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 2, 1.0f,  0.0f
            {/*pos*/ { 1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 0.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 3, 0.0f,  0.0f
            {/*pos*/ {-1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 0.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)}
        };
        for (int i = 0; i < 4; i++) m_Vertices.push_back(vertex[i]);

        m_Indices.push_back(0);
        m_Indices.push_back(1);
        m_Indices.push_back(3);
        m_Indices.push_back(1);
        m_Indices.push_back(2);
        m_Indices.push_back(3);
    }

    entt::entity Builder::LoadCubemap(const std::vector<std::string>& faces, entt::registry& registry)
    {
        entt::entity entity;
        static constexpr uint VERTEX_COUNT = 36;

        m_Vertices.clear();
        m_Indices.clear();
        m_Cubemaps.clear();
        
        glm::vec3 cubemapVertices[VERTEX_COUNT] =
        {
            // positions          
            {-1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},

            {-1.0f, -1.0f,  1.0f},
            {-1.0f, -1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f, -1.0f},
            {-1.0f,  1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f},

            {1.0f, -1.0f, -1.0f},
            {1.0f, -1.0f,  1.0f},
            {1.0f,  1.0f,  1.0f},
            {1.0f,  1.0f,  1.0f},
            {1.0f,  1.0f, -1.0f},
            {1.0f, -1.0f, -1.0f},

            {-1.0f, -1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f},
            {-1.0f, -1.0f,  1.0f},

            {-1.0f,  1.0f, -1.0f},
            { 1.0f,  1.0f, -1.0f},
            { 1.0f,  1.0f,  1.0f},
            { 1.0f,  1.0f,  1.0f},
            {-1.0f,  1.0f,  1.0f},
            {-1.0f,  1.0f, -1.0f},

            {-1.0f, -1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f, -1.0f},
            { 1.0f, -1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f},
            { 1.0f, -1.0f,  1.0f}
        };

        // create vertices
        {
            for (uint i = 0; i < VERTEX_COUNT; i++)
            {
                Vertex vertex = {/*pos*/ cubemapVertices[i], /*col*/ {0.0f, 0.0f, 0.0f}, /*norm*/ {0.0f, 0.0f, 0.0f}, /*uv*/ {0.0f, 0.0f}, /* amplification */0.0f, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};
                m_Vertices.push_back(vertex);
            }    
        }

        // create texture
        {
            auto cubemap = Cubemap::Create();
            if (cubemap->Init(faces, true))
            {
                m_Cubemaps.push_back(cubemap);
            }
            else
            {
                LOG_CORE_WARN("Builder::LoadCubemap: error loading skybox");
                return entt::null;
            }
        }

        // create descriptor set
        {
            PrimitiveCubemap primitiveCubemap{};
            primitiveCubemap.m_FirstVertex = 0;
            primitiveCubemap.m_VertexCount = VERTEX_COUNT;

            VK_Model::CreateDescriptorSet(primitiveCubemap.m_CubemapMaterial, m_Cubemaps[0]);
            m_PrimitivesCubemap.push_back(primitiveCubemap);
        }

        // create game object
        {
            auto model = Engine::m_Engine->LoadModel(*this);
            entity = registry.create();
            MeshComponent mesh{"cubemap", model};
            registry.emplace<MeshComponent>(entity, mesh);
            TransformComponent transform{};
            registry.emplace<TransformComponent>(entity, transform);
            CubemapComponent cubemapComponent{};
            registry.emplace<CubemapComponent>(entity, cubemapComponent);
        }

        return entity;
    }
}
