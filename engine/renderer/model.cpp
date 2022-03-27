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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "renderer/model.h"
#include "auxiliary/hash.h"

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

                        float factor = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                        glm::vec3 tangent, bitangent;

                        tangent.x = factor * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                        tangent.y = factor * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                        tangent.z = factor * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                        bitangent.x = factor * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                        bitangent.y = factor * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                        bitangent.z = factor * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                        uint vertexIndex1 = m_Indices[index];
                        uint vertexIndex2 = m_Indices[index-1];
                        uint vertexIndex3 = m_Indices[index-2];
                        m_Vertices[vertexIndex1].m_Tangent = tangent;
                        m_Vertices[vertexIndex2].m_Tangent = tangent;
                        m_Vertices[vertexIndex3].m_Tangent = tangent;
                        m_Vertices[vertexIndex1].m_Bitangent = bitangent;
                        m_Vertices[vertexIndex2].m_Bitangent = bitangent;
                        m_Vertices[vertexIndex3].m_Bitangent = bitangent;

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
