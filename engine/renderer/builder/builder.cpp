
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

#include "stb_image.h"

#include "core.h"
#include "scene/scene.h"
#include "auxiliary/hash.h"
#include "renderer/builder/builder.h"
#include "renderer/model.h"

#include "VKmodel.h"

namespace std
{
    template <> struct hash<GfxRenderEngine::Vertex>
    {
        size_t operator()(GfxRenderEngine::Vertex const& vertex) const
        {
            size_t seed = 0;
            GfxRenderEngine::HashCombine(seed, vertex.m_Position, vertex.m_Color, vertex.m_Normal, vertex.m_UV);
            return seed;
        }
    };
} // namespace std

namespace GfxRenderEngine
{

    void Builder::LoadSprite(Sprite const& sprite, float const amplification, int const unlit, glm::vec4 const& color)
    {
        m_Vertices.clear();
        m_Indices.clear();
        m_Vertices.reserve(4);
        m_Indices.reserve(6);
        // 0 - 1
        // | / |
        // 3 - 2

        // index 0, 0.0f,  1.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{-1.0f, 1.0f, 0.0f},
                                /*col*/ glm::vec4{0.0f, 0.1f, 0.9f, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, 1.0f},
                                /*uv*/ glm::vec2{sprite.m_Pos1X, sprite.m_Pos1Y});
        // index 1, 1.0f,  1.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{1.0f, 1.0f, 0.0f},
                                /*col*/ glm::vec4{0.0f, 0.1f, 0.9f, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, 1.0f},
                                /*uv*/ glm::vec2{sprite.m_Pos2X, sprite.m_Pos1Y});
        // index 2, 1.0f,  0.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{1.0f, -1.0f, 0.0f},
                                /*col*/ glm::vec4{0.0f, 0.9f, 0.1f, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, 1.0f},
                                /*uv*/ glm::vec2{sprite.m_Pos2X, sprite.m_Pos2Y});
        // index 3, 0.0f,  0.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{-1.0f, -1.0f, 0.0f},
                                /*col*/ glm::vec4{0.0f, 0.9f, 0.1f, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, 1.0f},
                                /*uv*/ glm::vec2{sprite.m_Pos1X, sprite.m_Pos2Y});

        m_Indices.emplace_back(0);
        m_Indices.emplace_back(1);
        m_Indices.emplace_back(3);
        m_Indices.emplace_back(1);
        m_Indices.emplace_back(2);
        m_Indices.emplace_back(3);
    }

    void Builder::LoadParticle(glm::vec4 const& color)
    {
        m_Vertices.clear();
        m_Indices.clear();

        m_Vertices.reserve(4);
        m_Indices.reserve(6);

        // 0 - 1
        // | / |
        // 3 - 2

        // index 0, 0.0f,  1.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{-1.0f, 1.0f, 0.0f},
                                /*col*/ glm::vec4{color.x, color.y, color.z, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, -1.0f},
                                /*uv*/ glm::vec2{0.0f, 1.0f});

        // index 1, 1.0f,  1.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{1.0f, 1.0f, 0.0f},
                                /*col*/ glm::vec4{color.x, color.y, color.z, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, -1.0f},
                                /*uv*/ glm::vec2{1.0f, 1.0f});

        // index 2, 1.0f,  0.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{1.0f, -1.0f, 0.0f},
                                /*col*/ glm::vec4{color.x, color.y, color.z, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, -1.0f},
                                /*uv*/ glm::vec2{1.0f, 0.0f});

        // index 3, 0.0f,  0.0f
        m_Vertices.emplace_back(/*pos*/ glm::vec3{-1.0f, -1.0f, 0.0f},
                                /*col*/ glm::vec4{color.x, color.y, color.z, 1.0f},
                                /*norm*/ glm::vec3{0.0f, 0.0f, -1.0f},
                                /*uv*/ glm::vec2{0.0f, 0.0f});

        m_Indices.emplace_back(0);
        m_Indices.emplace_back(1);
        m_Indices.emplace_back(3);
        m_Indices.emplace_back(1);
        m_Indices.emplace_back(2);
        m_Indices.emplace_back(3);
    }

    entt::entity Builder::LoadCubemap(std::vector<std::string> const& faces, Registry& registry)
    {
        ZoneScopedN("Builder::LoadCubemap()");
        entt::entity entity;
        static constexpr uint VERTEX_COUNT = 36;

        m_Vertices.clear();
        m_Indices.clear();
        m_Cubemaps.clear();

        glm::vec3 cubemapVertices[VERTEX_COUNT] = {// positions
                                                   {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
                                                   {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},

                                                   {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
                                                   {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},

                                                   {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
                                                   {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},

                                                   {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
                                                   {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},

                                                   {-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},
                                                   {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f},

                                                   {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},
                                                   {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f}};

        // create vertices
        {
            m_Vertices.reserve(VERTEX_COUNT);
            for (uint i = 0; i < VERTEX_COUNT; i++)
            {
                m_Vertices.emplace_back(/*pos*/ std::move(cubemapVertices[i]),
                                        /*col*/ glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
                                        /*norm*/ glm::vec3{0.0f, 0.0f, 0.0f},
                                        /*uv*/ glm::vec2{0.0f, 0.0f});
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
            Submesh submesh{};
            submesh.m_FirstVertex = 0;
            submesh.m_VertexCount = VERTEX_COUNT;

            { // create material descriptor
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtCubemap, m_Cubemaps[0]);
                submesh.m_Material.m_MaterialDescriptor = materialDescriptor;
            }

            m_Submeshes.push_back(submesh);
        }

        // create game object
        {
            auto model = Engine::m_Engine->LoadModel(*this);
            entity = registry.Create();
            MeshComponent mesh{"cubemap", model};
            registry.emplace<MeshComponent>(entity, mesh);
            TransformComponent transform{};
            registry.emplace<TransformComponent>(entity, transform);
            CubemapComponent cubemapComponent{};
            registry.emplace<CubemapComponent>(entity, cubemapComponent);
        }

        return entity;
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

    void Builder::CalculateTangentsFromIndexBuffer(std::vector<uint> const& indices)
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
            auto& vertex = m_Vertices[index];

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
                    {
                        tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                    }

                    m_Vertices[vertexIndex1].m_Tangent = tangent;
                    m_Vertices[vertexIndex2].m_Tangent = tangent;
                    m_Vertices[vertexIndex3].m_Tangent = tangent;

                    break;
            }
            cnt = (cnt + 1) % 3;
        }
    }
} // namespace GfxRenderEngine
