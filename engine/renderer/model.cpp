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

#include <memory>
#include <iostream>
#include <unordered_map>

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "gtc/type_ptr.hpp"

#include "renderer/model.h"
#include "renderer/texture.h"
#include "auxiliary/hash.h"
#include "auxiliary/file.h"

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
    bool Vertex::operator==(const Vertex& other) const
    {
        return (m_Position    == other.m_Position) &&
               (m_Color       == other.m_Color) &&
               (m_Normal      == other.m_Normal) &&
               (m_UV          == other.m_UV) &&
               (m_DiffuseMapTextureSlot == other.m_DiffuseMapTextureSlot) &&
               (m_Amplification == other.m_Amplification) &&
               (m_Unlit       == other.m_Unlit) &&
               (m_NormalTextureSlot == other.m_NormalTextureSlot);
    }

    void Builder::LoadGLTF(const std::string& filepath, int diffuseMapTextureSlot, int fragAmplification)
    {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF gltfLoader;
        std::string warn, err;
        std::vector<std::shared_ptr<Texture>> images;

        if (!gltfLoader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath))
        {
            LOG_CORE_CRITICAL("LoadGLTF errors: {0}, warnings: {1}", err, warn);
        }

        std::string basepath = EngineCore::GetPathWithoutFilename(filepath);
        // handle images
        for (uint i = 0; i < gltfModel.images.size(); i++)
        {
            std::string imageFilepath = basepath + gltfModel.images[0].uri;
            tinygltf::Image& glTFImage = gltfModel.images[i];

            // glTFImage.component - the number of channels in each pixel
            // three channels per pixel need to be converted to four channels per pixel
            uchar* buffer = nullptr;
            uint64 bufferSize = 0;
            if (glTFImage.component == 3)
            {
                bufferSize = glTFImage.width * glTFImage.height * 4;
                std::vector<uchar> imageData(bufferSize, 0x00);

                buffer = (uchar*)imageData.data();
                uchar* rgba = buffer;
                uchar* rgb = &glTFImage.image[0];
                for (uint i = 0; i < glTFImage.width * glTFImage.height; ++i)
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
            //auto texture = Texture::Create();
            //texture->Init(glTFImage.width, glTFImage.height, buffer);
            //images.push_back(texture);
        }

        // handle vertex data
        m_Vertices.clear();
        m_Indices.clear();

        for (const auto& mesh : gltfModel.meshes)
        {
            for (const auto& glTFPrimitive : mesh.primitives)
            {
                uint32_t firstIndex  = 0;
                uint32_t vertexStart = 0;
                uint32_t indexCount  = 0;
                // Vertices
                {
                    const float* positionBuffer = nullptr;
                    const float* normalsBuffer = nullptr;
                    const float* texCoordsBuffer = nullptr;
                    size_t vertexCount = 0;

                    // Get buffer data for vertex normals
                    if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                        const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
                        positionBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        vertexCount = accessor.count;
                    }
                    // Get buffer data for vertex normals
                    if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                        const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
                        normalsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                    // Get buffer data for vertex texture coordinates
                    // glTF supports multiple sets, we only load the first one
                    if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                        const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
                        texCoordsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }

                    // Append data to model's vertex buffer
                    for (size_t v = 0; v < vertexCount; v++) {
                        Vertex vertex{};
                        vertex.m_DiffuseMapTextureSlot = diffuseMapTextureSlot;
                        vertex.m_Amplification      = fragAmplification;

                        vertex.m_Position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                        vertex.m_Normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                        vertex.m_UV = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                        vertex.m_Color = glm::vec3(1.0f);
                        m_Vertices.push_back(vertex);
                    }
                }
                // Indices
                {
                    const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.indices];
                    const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

                    indexCount += static_cast<uint32_t>(accessor.count);

                    // glTF supports different component types of indices
                    switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            m_Indices.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            m_Indices.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++) {
                            m_Indices.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    default:
                        std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                        return;
                    }
                }
            }
        }
    }

    void Builder::LoadModel(const std::string &filepath, int diffuseMapTextureSlot, int fragAmplification, int normalTextureSlot)
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
                vertex.m_DiffuseMapTextureSlot = diffuseMapTextureSlot;
                vertex.m_Amplification      = fragAmplification;
                vertex.m_NormalTextureSlot  = normalTextureSlot;

                if (index.vertex_index >= 0)
                {
                    vertex.m_Position =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
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
                        attrib.normals[3 * index.normal_index + 1],
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
            // calculate tangents and bitangents
            uint index = 0;
            for (const auto& shapeMeshIndex : shape.mesh.indices)
            {

                static uint cnt = 0;
                static glm::vec3 position1;
                static glm::vec3 position2;
                static glm::vec3 position3;
                static glm::vec2 uv1;
                static glm::vec2 uv2;
                static glm::vec2 uv3;

                auto& vertex = m_Vertices[m_Indices[index]];

                switch (cnt)
                {
                    case 0:
                        position1 = vertex.m_Position;
                        uv1  = vertex.m_UV;
                        break;
                    case 1:
                        position2 = vertex.m_Position;
                        uv2  = vertex.m_UV;
                        break;
                    case 2:
                        position3 = vertex.m_Position;
                        uv3  = vertex.m_UV;

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

                        float factor = 1.0f / (dU1 * dV2 - dU2 * dV1);
                        
                        glm::vec3 tangent;
                        
                        tangent.x = factor * (dV2 * E1x - dV1 * E2x);
                        tangent.y = factor * (dV2 * E1y - dV1 * E2y);
                        tangent.z = factor * (dV2 * E1z - dV1 * E2z);

                        uint vertexIndex1 = m_Indices[index];
                        uint vertexIndex2 = m_Indices[index-1];
                        uint vertexIndex3 = m_Indices[index-2];
                        m_Vertices[vertexIndex1].m_Tangent = tangent;
                        m_Vertices[vertexIndex2].m_Tangent = tangent;
                        m_Vertices[vertexIndex3].m_Tangent = tangent;

                        break;
                }
                cnt = (cnt + 1) % 3;
                index++;
            }
        }
        LOG_CORE_INFO("Vertex count: {0}, Index count: {1}", m_Vertices.size(), m_Indices.size());
    }

    void Builder::LoadSprite(Sprite* sprite, const glm::mat4& position, float amplification, int unlit, const glm::vec4& color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        // 0 - 1
        // | / |
        // 3 - 2
        int slot = sprite->GetTextureSlot();

        Vertex vertex[4]
        {
            // index 0, 0.0f,  1.0f
            {/*pos*/ {position[0]}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {sprite->m_Pos1X, 1.0f-sprite->m_Pos2Y}, slot, amplification, unlit},

            // index 1, 1.0f,  1.0f
            {/*pos*/ {position[1]}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {sprite->m_Pos2X, 1.0f-sprite->m_Pos2Y}, slot, amplification, unlit},

            // index 2, 1.0f,  0.0f
            {/*pos*/ {position[2]}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {sprite->m_Pos2X, 1.0f-sprite->m_Pos1Y}, slot, amplification, unlit},

            // index 3, 0.0f,  0.0f
            {/*pos*/ {position[3]}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {sprite->m_Pos1X, 1.0f-sprite->m_Pos1Y}, slot, amplification, unlit}
        };
        for (int i = 0; i < 4; i++) m_Vertices.push_back(vertex[i]);
        slot++;

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
            {/*pos*/ {-1.0f,  1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 1.0f-1.0f}, /*slot*/0, 1.0f /*amplification*/, 0 /*unlit*/},

            // index 1, 1.0f,  1.0f
            {/*pos*/ { 1.0f,  1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 1.0f-1.0f}, /*slot*/0, 1.0f /*amplification*/, 0 /*unlit*/},

            // index 2, 1.0f,  0.0f
            {/*pos*/ { 1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 1.0f-0.0f}, /*slot*/0, 1.0f /*amplification*/, 0 /*unlit*/},

            // index 3, 0.0f,  0.0f
            {/*pos*/ {-1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 1.0f-0.0f}, /*slot*/0, 1.0f /*amplification*/, 0 /*unlit*/}
        };
        for (int i = 0; i < 4; i++) m_Vertices.push_back(vertex[i]);

        m_Indices.push_back(0);
        m_Indices.push_back(1);
        m_Indices.push_back(3);
        m_Indices.push_back(1);
        m_Indices.push_back(2);
        m_Indices.push_back(3);
    }
}
