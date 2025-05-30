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

#include "renderer/builder/fastgltfVertexLoader.h"

namespace GfxRenderEngine
{
    FastgltfVertexLoader::FastgltfVertexLoader(const std::string& filepath, TriangleList& triangles)
        : m_Filepath{filepath}, m_Triangles{triangles}
    {
    }

    bool FastgltfVertexLoader::Load(int const sceneID)
    {

        { // load from file
            auto path = std::filesystem::path{m_Filepath};

            // glTF files list their required extensions
            constexpr auto extensions =
                fastgltf::Extensions::KHR_mesh_quantization | fastgltf::Extensions::KHR_materials_emissive_strength |
                fastgltf::Extensions::KHR_lights_punctual | fastgltf::Extensions::KHR_texture_transform;

            constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                         fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers |
                                         fastgltf::Options::GenerateMeshIndices;

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

        if (!m_GltfAsset.meshes.size())
        {
            LOG_CORE_CRITICAL("Load: no meshes found in {0}", m_Filepath);
            return Gltf::GLTF_LOAD_FAILURE;
        }

        if (sceneID > Gltf::GLTF_NOT_USED) // a scene ID was provided
        {
            // check if valid
            if ((m_GltfAsset.scenes.size() - 1) < static_cast<size_t>(sceneID))
            {
                LOG_CORE_CRITICAL("Load: scene not found in {0}", m_Filepath);
                return Gltf::GLTF_LOAD_FAILURE;
            }
        }

        // a scene ID was provided
        if (sceneID > Gltf::GLTF_NOT_USED)
        {
            ProcessScene(m_GltfAsset.scenes[sceneID]);
        }
        else // no scene ID was provided --> use all scenes
        {
            for (auto& scene : m_GltfAsset.scenes)
            {
                ProcessScene(scene);
            }
        }
        return Gltf::GLTF_LOAD_SUCCESS;
    }

    void FastgltfVertexLoader::ProcessScene(fastgltf::Scene& scene)
    {
        size_t nodeCount = scene.nodeIndices.size();
        if (!nodeCount)
        {
            LOG_CORE_WARN("Builder::ProcessScene: empty scene in {0}", m_Filepath);
            return;
        }

        for (uint nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            ProcessNode(&scene, scene.nodeIndices[nodeIndex]);
        }
    }

    void FastgltfVertexLoader::ProcessNode(fastgltf::Scene* scene, int const gltfNodeIndex)
    {

        auto& node = m_GltfAsset.nodes[gltfNodeIndex];
        std::string nodeName(node.name);

        if (node.meshIndex.has_value())
        {
            int meshIndex = node.meshIndex.value();
            LoadVertexData(meshIndex);
        }

        size_t childNodeCount = node.children.size();
        for (size_t childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            int gltfChildNodeIndex = node.children[childNodeIndex];
            ProcessNode(scene, gltfChildNodeIndex);
        }
    }

    // load vertex data
    void FastgltfVertexLoader::LoadVertexData(uint const meshIndex)
    {

        for (const auto& glTFPrimitive : m_GltfAsset.meshes[meshIndex].primitives)
        {

            size_t vertexCount = 0;
            size_t indexCount = 0;

            // Vertices
            {
                const float* positionBuffer = nullptr;

                // Get buffer data for vertex positions
                if (glTFPrimitive.findAttribute("POSITION") != glTFPrimitive.attributes.end())
                {
                    auto componentType =
                        LoadAccessor<float>(m_GltfAsset.accessors[glTFPrimitive.findAttribute("POSITION")->second],
                                            positionBuffer, &vertexCount);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }

                // Append data to model's vertex buffer
                uint numVerticesBefore = m_Vertices.size();
                m_Vertices.resize(numVerticesBefore + vertexCount);
                uint vertexIndex = numVerticesBefore;
                for (size_t vertexIterator = 0; vertexIterator < vertexCount; ++vertexIterator)
                {
                    FastgltfVertexLoader::Vertex vertex{};
                    // position
                    auto position = positionBuffer ? glm::make_vec3(&positionBuffer[vertexIterator * 3]) : glm::vec3(0.0f);
                    vertex.m_Position = glm::vec3(position.x, position.y, position.z);

                    m_Vertices[vertexIndex] = vertex;
                    ++vertexIndex;
                }
            }

            // Indices
            if (glTFPrimitive.indicesAccessor.has_value())
            {
                auto& accessor = m_GltfAsset.accessors[glTFPrimitive.indicesAccessor.value()];
                indexCount = accessor.count;

                // append indices for submesh to global index array
                size_t globalIndicesOffset = m_Indicies.size();
                m_Indicies.resize(m_Indicies.size() + indexCount);
                uint* destination = m_Indicies.data() + globalIndicesOffset;
                fastgltf::iterateAccessorWithIndex<uint>(m_GltfAsset, accessor, [&](uint submeshIndex, size_t iterator)
                                                         { destination[iterator] = submeshIndex; });
                uint numTriangles = m_Indicies.size() / 3;
                m_Triangles.resize(numTriangles);
                for (uint iterator{0}; Triangle& triangle : m_Triangles)
                {
                    triangle.mV[0] = ConvertToFloat3(m_Vertices[m_Indicies[iterator * 3]].m_Position);
                    triangle.mV[1] = ConvertToFloat3(m_Vertices[m_Indicies[iterator * 3 + 1]].m_Position);
                    triangle.mV[2] = ConvertToFloat3(m_Vertices[m_Indicies[iterator * 3 + 2]].m_Position);
                    ++iterator;
                }
            }
        }
    }

    void FastgltfVertexLoader::PrintAssetError(fastgltf::Error assetErrorCode)
    {
        LOG_CORE_CRITICAL("FastgltfVertexLoader::Load: couldn't load {0}", m_Filepath);
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
