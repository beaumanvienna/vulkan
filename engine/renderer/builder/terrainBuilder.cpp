
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
#include "renderer/image.h"
#include "renderer/model.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/terrainBuilder.h"
#include "auxiliary/file.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{
    void TerrainBuilder::ColorTerrain(Terrain::TerrainSpec const& terrainSpec, Image const& heightMap)
    {
        if (!EngineCore::FileExists(terrainSpec.m_FilepathColorMap))
        {
            return;
        }
        Image colorMap{terrainSpec.m_FilepathColorMap};

        if (!colorMap.IsValid())
        {
            LOG_CORE_CRITICAL("color map did not load: {0}", terrainSpec.m_FilepathColorMap);
            return;
        }

        if (!(colorMap.BytesPerPixel() == 4))
        {
            LOG_CORE_CRITICAL("color map must be rgba (got {0} bytes per pixel) from {1}", colorMap.BytesPerPixel(),
                              terrainSpec.m_FilepathColorMap);
            return;
        }

        if (!((colorMap.Width() == heightMap.Width()) && (colorMap.Height()) == heightMap.Height()))
        {
            LOG_CORE_CRITICAL("color map  and height map dimensions must match: color map width: {0}, color map height: "
                              "{1}, height map width: {2}, height map height: {3}, color map: {4}, heigh map: {5}",
                              colorMap.Width(), colorMap.Height(), heightMap.Width(), heightMap.Height(),
                              terrainSpec.m_FilepathColorMap, terrainSpec.m_FilepathHeightMap);
            return;
        }

        uint* imageData = reinterpret_cast<uint*>(colorMap.Get());
        size_t vertexCounter = 0;
        for (int y = 0; y < colorMap.Height(); ++y)
        {
            int yOffset = y * colorMap.Width();
            for (int x = 0; x < colorMap.Width(); ++x)
            { // image format is rgba in reverse
                uint abgr = imageData[yOffset + x];
                float r = (0xff & (abgr >> 0)) / 255.0f;
                float g = (0xff & (abgr >> 8)) / 255.0f;
                float b = (0xff & (abgr >> 16)) / 255.0f;
                float a = (0xff & (abgr >> 24)) / 255.0f;
                m_Vertices[vertexCounter].m_Color = glm::vec4(r, g, b, a);
                ++vertexCounter;
            }
        }
    }

    bool TerrainBuilder::PopulateTerrainData(Image const& heightMap)
    {

        int const& cols = heightMap.Width();
        int const& rows = heightMap.Height();
        if (!(rows > 0 && cols > 0))
        {
            LOG_CORE_CRITICAL("heightmap is incomplete");
            return false;
        }

        { // vertices
            m_Vertices.resize(rows * cols);
            int vertexCounter = 0;
            for (int row = 0; row < rows; ++row)
            {
                int rowOffset = row * cols;
                int rowPlusOneOffset = (row + 1) * cols;
                int rowMinusOneOffset = (row - 1) * cols;

                for (int col = 0; col < cols; ++col)
                {
                    Vertex& vertex = m_Vertices[vertexCounter];
                    auto byteToFloat = [](uchar const& byte) { return static_cast<uint>(byte) / 255.0f; };

                    float originY = byteToFloat(heightMap[vertexCounter]);

                    vertex.m_Position = glm::vec3(col, originY, row);
                    vertex.m_Color = glm::vec4(0.f, 0.f, originY / 3.0f, 1.0f);

                    // compute normals via neighbors
                    //     up
                    // left O right
                    //    down
                    glm::vec3 sumNormals(0.0f);
                    float leftY = col > 0 ? byteToFloat(heightMap[rowOffset + col - 1]) : 0.0f;
                    float rightY = col < cols - 1 ? byteToFloat(heightMap[rowOffset + col + 1]) : 0.0f;
                    float upY = row < rows - 1 ? byteToFloat(heightMap[rowPlusOneOffset + col]) : 0.0f;
                    float downY = row > 0 ? byteToFloat(heightMap[rowMinusOneOffset + col]) : 0.0f;

                    float dx = 1.0f;
                    float dz = 1.0f;

                    glm::vec3 left = glm::vec3(-dx, leftY - originY, 0.0f);
                    glm::vec3 right = glm::vec3(dx, rightY - originY, 0.0f);
                    glm::vec3 up = glm::vec3(0.0f, upY - originY, dz);
                    glm::vec3 down = glm::vec3(0.0f, downY - originY, -dz);

                    auto normalComponent = [&](glm::vec3 a, glm::vec3 b)
                    {
                        glm::vec3 normal;
                        if (col > 0 && row > 0 && col < cols - 1 && row < rows - 1)
                        {
                            // Cross products to compute normals
                            normal = glm::cross(a, b);
                        }
                        else
                        {
                            normal = glm::vec3(0.0f, 1.0f, 0.0f);
                        }
                        return normal;
                    };

                    // smoothshading
                    sumNormals = normalComponent(left, -down);
                    sumNormals += normalComponent(-down, right);
                    sumNormals += normalComponent(right, -up);
                    sumNormals += normalComponent(-up, left);

                    vertex.m_Normal = glm::normalize(sumNormals);
                    ++vertexCounter;
                }
            }
        }

        { // indices
            m_Indices.resize(rows * cols * 6 /*six indices per quad*/);
            int index = 0;
            for (int row = 0; row < rows - 1; ++row)
            {
                uint rowOffset = row * cols;
                uint rowPlusOneOffset = (row + 1) * cols;
                for (int col = 0; col < cols - 1; ++col)
                {
                    uint topLeft = rowOffset + col;
                    uint topRight = topLeft + 1;
                    uint bottomLeft = rowPlusOneOffset + col;
                    uint bottomRight = bottomLeft + 1;

                    m_Indices[index++] = topLeft;
                    m_Indices[index++] = bottomLeft;
                    m_Indices[index++] = topRight;
                    m_Indices[index++] = topRight;
                    m_Indices[index++] = bottomLeft;
                    m_Indices[index++] = bottomRight;
                }
            }
        }
        CalculateTangents();
        return true;
    }

    bool TerrainBuilder::LoadTerrain(Scene& scene, int instanceCount, Terrain::TerrainSpec const& terrainSpec)
    {
        ZoneScopedNC("TerrainBuilder::LoadTerrain", 0xFF0000);
        TerrainComponent terrainComponent{};
        auto& registry = scene.GetRegistry();
        auto& sceneGraph = scene.GetSceneGraph();
        auto& dictionary = scene.GetDictionary();

        m_Vertices.clear();
        m_Indices.clear();
        m_Submeshes.clear();

        { // terrain data
            terrainComponent.m_HeightMap = std::make_shared<Image>(terrainSpec.m_FilepathHeightMap);
            Image& heightMap = *terrainComponent.m_HeightMap.get();

            if (!heightMap.IsValid())
            {
                LOG_CORE_CRITICAL("TerrainBuilder::LoadTerrain: Couldn't load file {0}", terrainSpec.m_FilepathHeightMap);
                return false;
            }

            if (heightMap.BytesPerPixel() != 1)
            {
                LOG_CORE_CRITICAL("TerrainBuilder::LoadTerrain: heightmap {0} must be 8bpc GRAY (1 byte per pixel)",
                                  terrainSpec.m_FilepathHeightMap);
                return false;
            }

            bool succesful = PopulateTerrainData(heightMap);
            if (!succesful)
            {
                return false;
            }
            ColorTerrain(terrainSpec, heightMap);
        }

        { // create game objects for all instances

            InstanceTag instanceTag;

            for (int instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex)
            {
                // create game object
                entt::entity entity = registry.Create();
                std::shared_ptr<Model> model;
                TransformComponent transform{};
                instanceTag.m_Instances.push_back(entity);

                // add to scene graph
                auto name = terrainSpec.m_FilepathTerrainDescription + "::" + std::to_string(instanceIndex);
                sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, dictionary);

                // only for the 1st instance
                if (!instanceIndex)
                {
                    // create instance buffer
                    instanceTag.m_InstanceBuffer = InstanceBuffer::Create(instanceCount);
                    registry.emplace<InstanceTag>(entity, instanceTag);

                    Submesh submesh{};
                    submesh.m_FirstIndex = 0;
                    submesh.m_FirstVertex = 0;
                    submesh.m_IndexCount = m_Indices.size();
                    submesh.m_VertexCount = m_Vertices.size();
                    submesh.m_InstanceCount = instanceCount;

                    submesh.m_Material.m_PbrMaterial = terrainSpec.m_PbrMaterial;

                    { // create material descriptor
                        Material::MaterialTextures materialTextures;

                        auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbr, materialTextures);
                        submesh.m_Material.m_MaterialDescriptor = materialDescriptor;
                    }
                    { // create resource descriptor
                        Resources::ResourceBuffers resourceBuffers;

                        resourceBuffers[Resources::INSTANCE_BUFFER_INDEX] = instanceTag.m_InstanceBuffer->GetBuffer();
                        auto resourceDescriptor = ResourceDescriptor::Create(resourceBuffers);
                        submesh.m_Resources.m_ResourceDescriptor = resourceDescriptor;
                    }
                    m_Submeshes.push_back(submesh);
                    model = Engine::m_Engine->LoadModel(*this);

                    PbrMaterialTag pbrMaterialTag{};
                    registry.emplace<PbrMaterialTag>(entity, pbrMaterialTag);
                }

                instanceTag.m_InstanceBuffer->SetInstanceData(instanceIndex, transform.GetMat4Global(),
                                                              transform.GetNormalMatrix());
                transform.SetInstance(instanceTag.m_InstanceBuffer, instanceIndex);
                registry.emplace<TransformComponent>(entity, transform);

                auto shortName = EngineCore::GetFilenameWithoutPathAndExtension(terrainSpec.m_FilepathTerrainDescription) +
                                 std::string("::") + std::to_string(instanceIndex);
                MeshComponent mesh{shortName, model};
                registry.emplace<MeshComponent>(entity, mesh);
                registry.emplace<TerrainComponent>(entity, terrainComponent);
            }
        }

        { // populate landscape
            // create a height map on the GPU for grass locations and load a grass model
            Terrain::GrassSpec const& grassSpec = terrainSpec.m_GrassSpec;
            bool GrassModelFound = EngineCore::FileExists(grassSpec.m_FilepathGrassModel) &&
                                   !EngineCore::IsDirectory(grassSpec.m_FilepathGrassModel);
            bool densityMapFound = EngineCore::FileExists(grassSpec.m_FilepathDensityMap) &&
                                   !EngineCore::IsDirectory(grassSpec.m_FilepathDensityMap);
            if (GrassModelFound && densityMapFound)
            {
                Image heightMap(grassSpec.m_FilepathGrassHeightMap);
                Image densityMap(grassSpec.m_FilepathDensityMap);
                CORE_ASSERT((heightMap.Width() == densityMap.Width()) && (heightMap.Height() == densityMap.Height()),
                            "dimensions must match");
                uint heightMapSize = heightMap.Size();
                Resources::ResourceBuffers resourceBuffers;
                uint grassInstances = 0;
                {
                    {
                        std::vector<Terrain::GrassShaderData> bufferData(heightMapSize);
                        for (uint mapIndex = 0; mapIndex < heightMapSize; ++mapIndex)
                        {
                            float normalizedRandom = std::rand() / static_cast<float>(RAND_MAX);
                            float normalizedDenisty = densityMap[mapIndex] / 255.0f;
                            float randomizedDensity = normalizedRandom * normalizedDenisty;
                            bool placeGrass = (heightMap[mapIndex] > 0) && (randomizedDensity > 0.05f);
                            if (placeGrass)
                            {
                                bufferData[grassInstances].m_Height = heightMap[mapIndex];
                                bufferData[grassInstances].m_Index = mapIndex;
                                ++grassInstances;
                            }
                        }
                        CORE_ASSERT(grassInstances, "no grass placed");
                        auto& ubo = resourceBuffers[Resources::HEIGHTMAP];
                        ubo = Buffer::Create(grassInstances * sizeof(Terrain::GrassShaderData),
                                             Buffer::BufferUsage::STORAGE_BUFFER_VISIBLE_TO_CPU);
                        ubo->MapBuffer();
                        // update ubo
                        ubo->WriteToBuffer(bufferData.data());
                        ubo->Flush();
                    }

                    {
                        Terrain::GrassParameters grassParameters{.m_Width = heightMap.Width(),
                                                                 .m_Height = heightMap.Height(),
                                                                 .m_ScaleXZ = grassSpec.m_ScaleXZ,
                                                                 .m_ScaleY = grassSpec.m_ScaleY};
                        int bufferSize = sizeof(Terrain::GrassParameters);
                        auto& ubo = resourceBuffers[Resources::MULTI_PURPOSE_BUFFER];
                        ubo = Buffer::Create(bufferSize, Buffer::BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU);
                        ubo->MapBuffer();
                        // update ubo
                        ubo->WriteToBuffer(&grassParameters);
                        ubo->Flush();
                    }

                    FastgltfBuilder builder(grassSpec.m_FilepathGrassModel, scene, &resourceBuffers);
                    builder.SetDictionaryPrefix("terrain");
                    builder.Load(1 /*1 instance in scene graph (grass has the instance count in the tag)*/);

                    entt::entity grassEntityRoot =
                        dictionary.Retrieve(std::string("terrain::") + grassSpec.m_FilepathGrassModel + "::0::root");
                    if (grassEntityRoot != entt::null)
                    {
                        auto rootNode = sceneGraph.GetNodeByGameObject(grassEntityRoot);
                        auto& grassNode = sceneGraph.GetNode(rootNode.GetChild(0)); // grass model must be single game object
                        GrassTag grassTag{grassInstances};
                        registry.emplace<GrassTag>(grassNode.GetGameObject(), grassTag);

                        auto& transform = registry.get<TransformComponent>(grassEntityRoot);
                        transform.SetRotation(terrainSpec.m_GrassSpec.m_Rotation);
                        transform.SetTranslation(terrainSpec.m_GrassSpec.m_Translation);
                        transform.SetScale({terrainSpec.m_GrassSpec.m_Scale});
                    }
                }
            }
        }

        return true;
    }

    void TerrainBuilder::CalculateTangents()
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

    void TerrainBuilder::CalculateTangentsFromIndexBuffer(std::vector<uint> const& indices)
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
