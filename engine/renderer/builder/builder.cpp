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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "core.h"
#include "renderer/builder/builder.h"
#include "auxiliary/hash.h"
#include "scene/scene.h"

#include "VKmodel.h"

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

    entt::entity Builder::LoadTerrainHeightMap(const std::string &filepath, Scene &scene)
    {

        m_Vertices.clear();
        m_Indices.clear();
        m_Cubemaps.clear();
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        size_t floatCount = fileSize / sizeof(float);

        std::vector<float> buffer(floatCount);
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
        file.close();

        terrainSize = static_cast<uint32_t>(sqrt(floatCount));
        terrainData.resize(terrainSize, std::vector<float>(terrainSize));
        for (size_t i = 0; i < terrainSize; ++i)
        {
            for (size_t j = 0; j < terrainSize; ++j)
            {
                terrainData[i][j] = buffer[i * terrainSize + j];
            }
        }

        float scale = 0.1f;       // Scale for the grid spacing
        float heightScale = .01f; // Scale for the height values
        size_t rows = terrainData.size();
        size_t cols = terrainData.empty() ? 0 : terrainData[0].size();
        // size_t rows = 50;
        // size_t cols = 50;
        Vertex vertex{};
        for (size_t z = 0; z < rows; ++z)
        {
            for (size_t x = 0; x < cols; ++x)
            {
                vertex.m_Position = glm::vec3(x * scale, terrainData[z][x] * heightScale, z * scale);
                std::cout << vertex.m_Position.x << " " << vertex.m_Position.y << " " << vertex.m_Position.z << std::endl;
                vertex.m_Color = glm::vec3(1.f, 0.f, 1.f);
                vertex.m_UV = glm::vec2(0.f, 0.f);
                vertex.m_Amplification = 0.f;
                vertex.m_Unlit = 0;
                vertex.m_Tangent = glm::vec3(1.0F);
                vertex.m_JointIds = glm::ivec4(0.f);
                vertex.m_Weights = glm::vec4(0.0f);

                //--------

                glm::vec3 sumNormals(0.0f);
                // Neighbors
                glm::vec3 left = x > 0 ? glm::vec3(-1.0f, terrainData[z][x - 1] - terrainData[z][x], 0.0f) : glm::vec3(0.0f);
                glm::vec3 right = x < rows - 1 ? glm::vec3(1.0f, terrainData[z][x + 1] - terrainData[z][x], 0.0f) : glm::vec3(0.0f);
                glm::vec3 down = z > 0 ? glm::vec3(0.0f, terrainData[z - 1][x] - terrainData[z][x], -1.0f) : glm::vec3(0.0f);
                glm::vec3 up = z < rows - 1 ? glm::vec3(0.0f, terrainData[z + 1][x] - terrainData[z][x], 1.0f) : glm::vec3(0.0f);

                // Cross products to compute normals
                if (x > 0 && z > 0)
                    sumNormals += glm::cross(left, down);
                if (x < rows - 1 && z > 0)
                    sumNormals += glm::cross(down, right);
                if (x < rows - 1 && z < rows - 1)
                    sumNormals += glm::cross(right, up);
                if (x > 0 && z < rows - 1)
                    sumNormals += glm::cross(up, left);

                vertex.m_Normal = glm::normalize(sumNormals);
                // vertex.m_Normal = glm::vec3(0.0f, 1.0f, 0.0f);

                m_Vertices.push_back(vertex);

                //--------
            }
        }
        for (size_t z = 0; z < rows - 1; ++z)
        {
            for (size_t x = 0; x < cols - 1; ++x)
            {

                uint32_t topLeft = z * cols + x;

                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = (z + 1) * cols + x;
                uint32_t bottomRight = bottomLeft + 1;

                m_Indices.push_back(topLeft);
                m_Indices.push_back(bottomLeft);
                m_Indices.push_back(topRight);
                m_Indices.push_back(topRight);
                m_Indices.push_back(bottomLeft);
                m_Indices.push_back(bottomRight);
            }
        }

        ModelSubmesh submesh{};
        submesh.m_FirstIndex = 0;
        submesh.m_FirstVertex = 0;
        submesh.m_IndexCount = m_Indices.size();
        submesh.m_VertexCount = m_Vertices.size();

        { // create material descriptor
            auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMap);
            submesh.m_MaterialDescriptors.push_back(materialDescriptor);
        }
        submesh.m_MaterialProperties.m_Roughness = 0.5f;
        submesh.m_MaterialProperties.m_Metallic = 0.1f;
        submesh.m_MaterialProperties.m_NormalMapIntensity = 1.0f;

        m_Submeshes.push_back(submesh);

        // create game object
        auto model = Engine::m_Engine->LoadModel(*this);
        entt::entity entity;
        {
            auto &registry = scene.GetRegistry();
            auto &sceneGraph = scene.GetSceneGraph();
            auto &dictionary = scene.GetDictionary();

            entity = registry.create();
            MeshComponent mesh{"terrain", model};
            registry.emplace<MeshComponent>(entity, mesh);
            TransformComponent transform{};
            registry.emplace<TransformComponent>(entity, transform);
            PbrNoMapTag pbrNoMapTag{};
            registry.emplace<PbrNoMapTag>(entity, pbrNoMapTag);

            uint groupNode = sceneGraph.CreateNode(entity, "terrain", "terrain", dictionary);
            sceneGraph.GetRoot().AddChild(groupNode);
        }
        return entity;
    }
    void Builder::LoadSprite(Sprite const &sprite, float const amplification, int const unlit, glm::vec4 const &color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        // 0 - 1
        // | / |
        // 3 - 2

        Vertex vertex[4];

        // index 0, 0.0f,  1.0f
        vertex[0] = {/*pos*/ {-1.0f, 1.0f, 0.0f}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos1X, sprite.m_Pos1Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 1, 1.0f,  1.0f
        vertex[1] = {/*pos*/ {1.0f, 1.0f, 0.0f}, /*col*/ {0.0f, 0.1f, 0.9f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos2X, sprite.m_Pos1Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 2, 1.0f,  0.0f
        vertex[2] = {/*pos*/ {1.0f, -1.0f, 0.0f}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos2X, sprite.m_Pos2Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        // index 3, 0.0f,  0.0f
        vertex[3] = {/*pos*/ {-1.0f, -1.0f, 0.0f}, /*col*/ {0.0f, 0.9f, 0.1f}, /*norm*/ {0.0f, 0.0f, 1.0f}, /*uv*/ {sprite.m_Pos1X, sprite.m_Pos2Y}, amplification, unlit, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};

        for (int i = 0; i < 4; i++)
            m_Vertices.push_back(vertex[i]);

        m_Indices.push_back(0);
        m_Indices.push_back(1);
        m_Indices.push_back(3);
        m_Indices.push_back(1);
        m_Indices.push_back(2);
        m_Indices.push_back(3);
    }

    void Builder::LoadParticle(const glm::vec4 &color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        // 0 - 1
        // | / |
        // 3 - 2

        Vertex vertex[4]{
            // index 0, 0.0f,  1.0f
            {/*pos*/ {-1.0f, 1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 1.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 1, 1.0f,  1.0f
            {/*pos*/ {1.0f, 1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 1.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 2, 1.0f,  0.0f
            {/*pos*/ {1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {1.0f, 0.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)},

            // index 3, 0.0f,  0.0f
            {/*pos*/ {-1.0f, -1.0f, 0.0f}, {color.x, color.y, color.z}, /*norm*/ {0.0f, 0.0f, -1.0f}, /*uv*/ {0.0f, 0.0f}, 1.0f /*amplification*/, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)}};
        for (int i = 0; i < 4; i++)
            m_Vertices.push_back(vertex[i]);

        m_Indices.push_back(0);
        m_Indices.push_back(1);
        m_Indices.push_back(3);
        m_Indices.push_back(1);
        m_Indices.push_back(2);
        m_Indices.push_back(3);
    }

    entt::entity Builder::LoadCubemap(const std::vector<std::string> &faces, entt::registry &registry)
    {
        entt::entity entity;
        static constexpr uint VERTEX_COUNT = 36;

        m_Vertices.clear();
        m_Indices.clear();
        m_Cubemaps.clear();

        glm::vec3 cubemapVertices[VERTEX_COUNT] =
            {
                // positions
                {-1.0f, 1.0f, -1.0f},
                {-1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {-1.0f, 1.0f, -1.0f},

                {-1.0f, -1.0f, 1.0f},
                {-1.0f, -1.0f, -1.0f},
                {-1.0f, 1.0f, -1.0f},
                {-1.0f, 1.0f, -1.0f},
                {-1.0f, 1.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f},

                {1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},

                {-1.0f, -1.0f, 1.0f},
                {-1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f},

                {-1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, -1.0f},

                {-1.0f, -1.0f, -1.0f},
                {-1.0f, -1.0f, 1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {-1.0f, -1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f}};

        // create vertices
        {
            for (uint i = 0; i < VERTEX_COUNT; i++)
            {
                Vertex vertex = {/*pos*/ cubemapVertices[i], /*col*/ {0.0f, 0.0f, 0.0f}, /*norm*/ {0.0f, 0.0f, 0.0f}, /*uv*/ {0.0f, 0.0f}, /* amplification */ 0.0f, 0 /*unlit*/, /*tangent*/ glm::vec3(0.0), glm::ivec4(0.0), glm::vec4(0.0)};
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

        {
            ModelSubmesh submesh{};
            submesh.m_FirstVertex = 0;
            submesh.m_VertexCount = VERTEX_COUNT;

            { // create material descriptor
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtCubemap, m_Cubemaps[0]);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_Submeshes.push_back(submesh);
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

    void Builder::LoadModelObjWavefront(const std::string &filepath, int fragAmplification)
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

        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.m_Amplification = fragAmplification;

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

        {
            ModelSubmesh submesh{};
            submesh.m_FirstIndex = 0;
            submesh.m_FirstVertex = 0;
            submesh.m_IndexCount = m_Indices.size();
            submesh.m_VertexCount = m_Vertices.size();

            { // create material descriptor
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMap);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            submesh.m_MaterialProperties.m_Roughness = 0.5f;
            submesh.m_MaterialProperties.m_Metallic = 0.1f;

            m_Submeshes.push_back(submesh);
        }

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

    void Builder::CalculateTangentsFromIndexBuffer(const std::vector<uint> &indices)
    {
        uint cnt = 0;
        uint vertexIndex1 = 0;
        uint vertexIndex2 = 0;
        uint vertexIndex3 = 0;
        glm::vec3 position1 = glm::vec3{0.0f};
        glm::vec3 position2 = glm::vec3{0.0f};
        glm::vec3 position3 = glm::vec3{0.0f};
        glm::vec2 uv1 = glm::vec2{0.0f};
        glm::vec2 uv2 = glm::vec2{0.0f};
        glm::vec2 uv3 = glm::vec2{0.0f};

        for (uint index : indices)
        {
            auto &vertex = m_Vertices[index];

            switch (cnt)
            {
            case 0:
                position1 = vertex.m_Position;
                uv1 = vertex.m_UV;
                vertexIndex1 = index;
                break;
            case 1:
                position2 = vertex.m_Position;
                uv2 = vertex.m_UV;
                vertexIndex2 = index;
                break;
            case 2:
                position3 = vertex.m_Position;
                uv3 = vertex.m_UV;
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
                if ((dU1 * dV2 - dU2 * dV1) > std::numeric_limits<float>::epsilon())
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
                if (tangent.x == 0.0f && tangent.y == 0.0f && tangent.z == 0.0f)
                    tangent = glm::vec3(1.0f, 0.0f, 0.0f);

                m_Vertices[vertexIndex1].m_Tangent = tangent;
                m_Vertices[vertexIndex2].m_Tangent = tangent;
                m_Vertices[vertexIndex3].m_Tangent = tangent;

                break;
            }
            cnt = (cnt + 1) % 3;
        }
    }
}
