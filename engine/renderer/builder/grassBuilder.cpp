
/* Engine Copyright (c) 2025 Engine Development Team
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

#include "gtc/type_ptr.hpp"

#include "engine.h"
#include "renderer/builder/grassBuilder.h"
#include "renderer/builder/fastgltfBuilder.h"
#include "auxiliary/file.h"
#include "scene/scene.h"
#include "scene/gltf.h"

namespace GfxRenderEngine
{
    GrassBuilder::GrassBuilder(Grass::GrassSpec& grassSpec, Scene& scene) : m_GrassSpec{grassSpec}, m_Scene{scene} {}

    bool GrassBuilder::LoadMask()
    {
        if (!EngineCore::FileExists(m_GrassSpec.m_FilepathGrassModel))
        {
            LOG_CORE_CRITICAL("GrassBuilder::LoadMask: {0} not found", m_GrassSpec.m_FilepathGrassModel);
            return Gltf::GLTF_LOAD_FAILURE;
        }

        if (!EngineCore::FileExists(m_GrassSpec.m_FilepathGrassMask))
        {
            LOG_CORE_CRITICAL("GrassBuilder::LoadMask: {0} not found", m_GrassSpec.m_FilepathGrassMask);
            return Gltf::GLTF_LOAD_FAILURE;
        }

        { // load mask from file
            auto path = std::filesystem::path{m_GrassSpec.m_FilepathGrassMask};

            // glTF files list their required extensions
            constexpr auto extensions =
                fastgltf::Extensions::KHR_mesh_quantization | fastgltf::Extensions::KHR_texture_transform;

            constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                         fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers |
                                         fastgltf::Options::LoadExternalImages | fastgltf::Options::GenerateMeshIndices;

            fastgltf::GltfDataBuffer dataBuffer;
            fastgltf::Parser parser(extensions);
            // load raw data of the file (can be gltf or glb)
            dataBuffer.loadFromFile(path);

            // parse (function determines if gltf or glb)
            fastgltf::Expected<fastgltf::Asset> asset = parser.loadGltf(&dataBuffer, path.parent_path(), gltfOptions);
            auto assetErrorCode = asset.error();

            if (assetErrorCode != fastgltf::Error::None)
            {
                PrintAssetError(assetErrorCode);
                return Gltf::GLTF_LOAD_FAILURE;
            }
            m_GltfAsset = std::move(asset.get());
        }

        return Gltf::GLTF_LOAD_SUCCESS;
    }

    bool GrassBuilder::LoadVertexData()
    {
        bool ok = Gltf::GLTF_LOAD_SUCCESS;
        uint numMeshes = m_GltfAsset.meshes.size();
        m_MaskData.resize(numMeshes);
        for (uint meshIndex{0}; meshIndex < numMeshes; ++meshIndex)
        {
            ok = ok && LoadVertexData(meshIndex);
        }
        return ok;
    }

    bool GrassBuilder::Build()
    {
        if (!LoadMask())
        {
            return false;
        }
        if (!LoadVertexData())
        {
            return false;
        }
        if (!ExtractQuads())
        {
            return false;
        }
        if (!CreateInstances())
        {
            return false;
        }
        return true;
    }

    bool GrassBuilder::CreateInstances()
    {
        auto& registry = m_Scene.GetRegistry();
        auto& sceneGraph = m_Scene.GetSceneGraph();
        auto& dictionary = m_Scene.GetDictionary();

        bool grassModelFound = EngineCore::FileExists(m_GrassSpec.m_FilepathGrassModel) &&
                               !EngineCore::IsDirectory(m_GrassSpec.m_FilepathGrassModel);

        if (!grassModelFound)
        {
            return Gltf::GLTF_LOAD_FAILURE;
        }

        for (auto& maskData : m_MaskData)
        {
            Resources::ResourceBuffers resourceBuffers;
            uint grassInstances = maskData.m_Quads.size();
            { // instance buffer
                std::vector<Grass::GrassShaderData> bufferData(grassInstances);
                for (uint quadIndex{0}; auto& quad : maskData.m_Quads)
                {
                    Vertex& v0 = maskData.m_Vertices[quad.m_Indices[0]];
                    Vertex& v1 = maskData.m_Vertices[quad.m_Indices[1]];
                    Vertex& v2 = maskData.m_Vertices[quad.m_Indices[2]];
                    Vertex& v3 = maskData.m_Vertices[quad.m_Indices[3]];

                    { // translation
                        glm::vec3& p0 = v0.m_Position;
                        glm::vec3& p1 = v1.m_Position;
                        glm::vec3& p2 = v2.m_Position;
                        glm::vec3& p3 = v3.m_Position;
                        float& translationX = bufferData[quadIndex].m_Translation.x;
                        float& translationY = bufferData[quadIndex].m_Translation.y;
                        float& translationZ = bufferData[quadIndex].m_Translation.z;
                        translationX = (p0.x + p1.x + p2.x + p3.x) / 4.0f;
                        translationY = (p0.y + p1.y + p2.y + p3.y) / 4.0f;
                        translationZ = (p0.z + p1.z + p2.z + p3.z) / 4.0f;
                    }

                    { // rotation
                        float& rotationX = bufferData[quadIndex].m_Rotation.x;
                        float& rotationY = bufferData[quadIndex].m_Rotation.y;
                        float& rotationZ = bufferData[quadIndex].m_Rotation.z;
                        rotationX = 0.0f;
                        rotationY = 0.0f;
                        rotationZ = 0.0f;
                    }
                    ++quadIndex;
                }

                auto& ubo = resourceBuffers[Resources::HEIGHTMAP];
                ubo = Buffer::Create(grassInstances * sizeof(Grass::GrassShaderData),
                                     Buffer::BufferUsage::STORAGE_BUFFER_VISIBLE_TO_CPU);
                ubo->MapBuffer();
                // update ubo
                ubo->WriteToBuffer(bufferData.data());
                ubo->Flush();
            }

            { // grass parameters
                int bufferSize = sizeof(Grass::GrassParameters);
                Grass::GrassParameters grassParameters{.m_Width = 1,  // not used
                                                       .m_Height = 1, // not used
                                                       .m_ScaleXZ = m_GrassSpec.m_ScaleXZ,
                                                       .m_ScaleY = m_GrassSpec.m_ScaleY};
                auto& ubo = resourceBuffers[Resources::MULTI_PURPOSE_BUFFER];
                ubo = Buffer::Create(bufferSize, Buffer::BufferUsage::UNIFORM_BUFFER_VISIBLE_TO_CPU);
                ubo->MapBuffer();
                // update ubo
                ubo->WriteToBuffer(&grassParameters);
                ubo->Flush();
            }

            FastgltfBuilder builder(m_GrassSpec.m_FilepathGrassModel, m_Scene, &resourceBuffers);
            builder.SetDictionaryPrefix("grass");
            builder.Load(1 /*1 instance in scene graph (grass has the instance count in the tag)*/);

            entt::entity grassEntityRoot =
                dictionary.Retrieve(std::string("grass::") + m_GrassSpec.m_FilepathGrassModel + "::0::root");
            if (grassEntityRoot != entt::null)
            {
                auto rootNode = sceneGraph.GetNodeByGameObject(grassEntityRoot);
                auto& grassNode = sceneGraph.GetNode(rootNode.GetChild(0)); // grass model must be single game object
                Grass2Tag grass2Tag{grassInstances};
                registry.emplace<Grass2Tag>(grassNode.GetGameObject(), grass2Tag);

                auto& transform = registry.get<TransformComponent>(grassEntityRoot);
                transform.SetRotation(m_GrassSpec.m_Rotation);
                transform.SetTranslation(m_GrassSpec.m_Translation);
                transform.SetScale({m_GrassSpec.m_Scale});
            }
        }

        return true;
    }

    bool GrassBuilder::ExtractQuads()
    {
        bool ok = true;
        CORE_ASSERT(m_MaskData.size() == 1, "only one mask supported");
        for (auto& maskData : m_MaskData)
        {
            LOG_CORE_INFO("GrassBuilder::Build: fetching quads");
            auto& indices = maskData.m_Indices;

            { // sanity check
                float mod = indices.size() % 6;
                bool divisibleBy6 = (mod == 0.0f);
                CORE_ASSERT(divisibleBy6, "number of vertices must be divisible by 6");
                if (!divisibleBy6)
                {
                    return false;
                }
            }
            auto& quads = maskData.m_Quads;
            uint numberOfQuads = indices.size() / 6;
            quads.resize(numberOfQuads);
            for (uint quadIndex{0}; auto& quad : quads)
            {
                quad.m_Indices[0] = indices[quadIndex * 6 + 0];
                quad.m_Indices[1] = indices[quadIndex * 6 + 1];
                quad.m_Indices[2] = indices[quadIndex * 6 + 2];
                quad.m_Indices[3] = indices[quadIndex * 6 + 5];
                ++quadIndex;
            }
            ok = ok && (quads.size() != 0);
        }

        return ok;
    }

    bool GrassBuilder::LoadVertexData(uint const meshIndex)
    {
        uint numPrimitives = m_GltfAsset.meshes[meshIndex].primitives.size();
        CORE_ASSERT(numPrimitives == 1, "gltf mask must have 1 submesh");

        for (const auto& glTFPrimitive : m_GltfAsset.meshes[meshIndex].primitives)
        {

            size_t vertexCount = 0;
            size_t indexCount = 0;

            auto& vertices = m_MaskData[meshIndex].m_Vertices;
            auto& indices = m_MaskData[meshIndex].m_Indices;

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const void* colorBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* tangentsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;

                fastgltf::ComponentType colorBufferComponentType = fastgltf::ComponentType::Invalid;

                // Get buffer data for vertex positions
                if (glTFPrimitive.findAttribute("POSITION") != glTFPrimitive.attributes.end())
                {
                    auto componentType =
                        LoadAccessor<float>(m_GltfAsset.accessors[glTFPrimitive.findAttribute("POSITION")->second],
                                            positionBuffer, &vertexCount);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex color
                if (glTFPrimitive.findAttribute("COLOR_0") != glTFPrimitive.attributes.end())
                {
                    fastgltf::Accessor& accessor = m_GltfAsset.accessors[glTFPrimitive.findAttribute("COLOR_0")->second];
                    colorBufferComponentType = accessor.componentType;
                    switch (colorBufferComponentType)
                    {
                        case fastgltf::ComponentType::Float:
                        {
                            const float* buffer;
                            LoadAccessor<float>(m_GltfAsset.accessors[glTFPrimitive.findAttribute("COLOR_0")->second],
                                                buffer);
                            colorBuffer = buffer;
                            break;
                        }
                        case fastgltf::ComponentType::UnsignedShort:
                        {
                            const uint16_t* buffer;
                            LoadAccessor<uint16_t>(m_GltfAsset.accessors[glTFPrimitive.findAttribute("COLOR_0")->second],
                                                   buffer);
                            colorBuffer = buffer;
                            break;
                        }
                        case fastgltf::ComponentType::UnsignedByte:
                        {
                            const uint8_t* buffer;
                            LoadAccessor<uint8_t>(m_GltfAsset.accessors[glTFPrimitive.findAttribute("COLOR_0")->second],
                                                  buffer);
                            colorBuffer = buffer;
                            break;
                        }
                        default:
                        {
                            int componentType = fastgltf::getGLComponentType(colorBufferComponentType);
                            CORE_ASSERT(false, "unexpected component type " + std::to_string(componentType));
                            break;
                        }
                    }
                }

                // Get buffer data for vertex normals
                if (glTFPrimitive.findAttribute("NORMAL") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfAsset.accessors[glTFPrimitive.findAttribute("NORMAL")->second], normalsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex tangents
                if (glTFPrimitive.findAttribute("TANGENT") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfAsset.accessors[glTFPrimitive.findAttribute("TANGENT")->second], tangentsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.findAttribute("TEXCOORD_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfAsset.accessors[glTFPrimitive.findAttribute("TEXCOORD_0")->second], texCoordsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }

                // create model's vertex buffer
                vertices.resize(vertexCount);
                uint vertexIndex = 0;
                for (size_t vertexIterator = 0; vertexIterator < vertexCount; ++vertexIterator)
                {
                    Vertex& vertex = vertices[vertexIndex];

                    // position
                    auto position = positionBuffer ? glm::make_vec3(&positionBuffer[vertexIterator * 3]) : glm::vec3(0.0f);
                    vertex.m_Position = glm::vec3(position.x, position.y, position.z);

                    // color
                    glm::vec3 vertexColor{1.0f};
                    switch (colorBufferComponentType)
                    {
                        case fastgltf::ComponentType::Float:
                        {
                            vertexColor =
                                colorBuffer ? glm::make_vec3(&((static_cast<const float*>(colorBuffer))[vertexIterator * 3]))
                                            : glm::vec3(1.0f);
                            break;
                        }
                        case fastgltf::ComponentType::UnsignedShort:
                        {
                            const uint16_t* vec3 = &((static_cast<const uint16_t*>(colorBuffer))[vertexIterator * 3]);
                            float norm = 0xFFFF;
                            vertexColor =
                                colorBuffer ? glm::vec3(vec3[0] / norm, vec3[1] / norm, vec3[2] / norm) : glm::vec3(1.0f);
                            break;
                        }
                        case fastgltf::ComponentType::UnsignedByte:
                        {
                            const uint8_t* vec3 = &((static_cast<const uint8_t*>(colorBuffer))[vertexIterator * 3]);
                            float norm = 0xFF;
                            vertexColor =
                                colorBuffer ? glm::vec3(vec3[0] / norm, vec3[1] / norm, vec3[2] / norm) : glm::vec3(1.0f);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    vertex.m_Color = glm::vec4(vertexColor.x, vertexColor.y, vertexColor.z, 1.0f);

                    // normal
                    vertex.m_Normal = glm::normalize(
                        glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[vertexIterator * 3]) : glm::vec3(0.0f)));

                    // uv
                    auto uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[vertexIterator * 2]) : glm::vec3(0.0f);
                    vertex.m_UV = uv;

                    // tangent
                    glm::vec4 t = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[vertexIterator * 4]) : glm::vec4(0.0f);
                    vertex.m_Tangent = glm::vec3(t.x, t.y, t.z) * t.w;

                    ++vertexIndex;
                }
            }

            // Indices
            if (glTFPrimitive.indicesAccessor.has_value())
            {
                auto& accessor = m_GltfAsset.accessors[glTFPrimitive.indicesAccessor.value()];
                indexCount = accessor.count;

                // append indices for submesh to global index array
                size_t globalIndicesOffset = indices.size();
                indices.resize(indices.size() + indexCount);
                uint* destination = indices.data() + globalIndicesOffset;
                fastgltf::iterateAccessorWithIndex<uint>(m_GltfAsset, accessor, [&](uint submeshIndex, size_t iterator)
                                                         { destination[iterator] = submeshIndex; });
            }
        }
        return m_MaskData[meshIndex].m_Indices.size() > 5; // at least one quad with six indices found
    }

    void GrassBuilder::PrintAssetError(fastgltf::Error assetErrorCode)
    {
        switch (assetErrorCode)
        {
            case fastgltf::Error::None:
            {
                LOG_CORE_CRITICAL("error code: ");
                break;
            }
            case fastgltf::Error::InvalidPath:
            {
                LOG_CORE_CRITICAL("error code: The glTF directory passed to Load is invalid.");
                break;
            }
            case fastgltf::Error::MissingExtensions:
            {
                LOG_CORE_CRITICAL(
                    "error code: One or more extensions are required by the glTF but not enabled in the Parser.");
                break;
            }
            case fastgltf::Error::UnknownRequiredExtension:
            {
                LOG_CORE_CRITICAL("error code: An extension required by the glTF is not supported by fastgltf.");
                break;
            }
            case fastgltf::Error::InvalidJson:
            {
                LOG_CORE_CRITICAL("error code: An error occurred while parsing the JSON.");
                break;
            }
            case fastgltf::Error::InvalidGltf:
            {
                LOG_CORE_CRITICAL("error code: The glTF is either missing something or has invalid data.");
                break;
            }
            case fastgltf::Error::InvalidOrMissingAssetField:
            {
                LOG_CORE_CRITICAL("error code: The glTF asset object is missing or invalid.");
                break;
            }
            case fastgltf::Error::InvalidGLB:
            {
                LOG_CORE_CRITICAL("error code: The GLB container is invalid.");
                break;
            }
            case fastgltf::Error::MissingField:
            {
                LOG_CORE_CRITICAL("error code: A field is missing in the JSON stream.");
                break;
            }
            case fastgltf::Error::MissingExternalBuffer:
            {
                LOG_CORE_CRITICAL("error code: With Options::LoadExternalBuffers, an external buffer was not found.");
                break;
            }
            case fastgltf::Error::UnsupportedVersion:
            {
                LOG_CORE_CRITICAL("error code: The glTF version is not supported by fastgltf.");
                break;
            }
            case fastgltf::Error::InvalidURI:
            {
                LOG_CORE_CRITICAL("error code: A URI from a buffer or image failed to be parsed.");
                break;
            }
            case fastgltf::Error::InvalidFileData:
            {
                LOG_CORE_CRITICAL("error code: The file data is invalid, or the file type could not be determined.");
                break;
            }
            default:
            {
                LOG_CORE_CRITICAL("error code: inkown fault code");
                break;
            }
        }
    }

} // namespace GfxRenderEngine
