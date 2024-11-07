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

#include "gtc/type_ptr.hpp"
#include "stb_image.h"

#include "core.h"
#include "scene/scene.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/fastgltfBuilder.h"
#include "renderer/materialDescriptor.h"
#include "auxiliary/instrumentation.h"
#include "auxiliary/file.h"

namespace GfxRenderEngine
{
    FastgltfBuilder::FastgltfBuilder(const std::string& filepath, Scene& scene, Resources::ResourceBuffers* resourceBuffers)
        : m_Filepath{filepath}, m_SkeletalAnimation{false}, m_Registry{scene.GetRegistry()},
          m_SceneGraph{scene.GetSceneGraph()}, m_Dictionary{scene.GetDictionary()},
          m_Basepath{EngineCore::GetPathWithoutFilename(filepath)}, m_GroupNode{Gltf::GLTF_NOT_USED}
    {
        if (resourceBuffers)
        {
            // optionally: additional resources not originating from a 3D file can be provided here
            // e.g. a height map for terrain required in the vertex shader
            // these additional resource go into descriptor set 2,
            // along with the instance and skeletal animation buffer
            m_ResourceBuffersPre = *resourceBuffers;
        }
    }

    FastgltfBuilder::FastgltfBuilder(const std::string& filepath, Scene& scene, int groupNode)
        : m_Filepath{filepath}, m_SkeletalAnimation{false}, m_Registry{scene.GetRegistry()},
          m_SceneGraph{scene.GetSceneGraph()}, m_Dictionary{scene.GetDictionary()},
          m_Basepath{EngineCore::GetPathWithoutFilename(filepath)}, m_GroupNode{groupNode}
    {
    }

    bool FastgltfBuilder::Load(uint const instanceCount, int const sceneID)
    {
        PROFILE_SCOPE("FastgltfBuilder::Load");
        ZoneScopedN("FastgltfBuilder::Load");
        stbi_set_flip_vertically_on_load(false);

        { // load from file

            ZoneTransientN(variableName, EngineCore::GetFilenameWithoutPathAndExtension(m_Filepath).c_str(), true);
            auto path = std::filesystem::path{m_Filepath};

            // glTF files list their required extensions
            constexpr auto extensions =
                fastgltf::Extensions::KHR_mesh_quantization | fastgltf::Extensions::KHR_materials_emissive_strength |
                fastgltf::Extensions::KHR_lights_punctual | fastgltf::Extensions::KHR_texture_transform;

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
            m_Models.resize(m_GltfAsset.nodes.size());
        }

        if (!m_GltfAsset.meshes.size() && !m_GltfAsset.lights.size() && !m_GltfAsset.cameras.size())
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

        LoadTextures();
        LoadSkeletonsGltf();
        LoadMaterials();

        // PASS 1
        // mark gltf nodes to receive a game object ID if they have a mesh or any child has
        // --> create array of flags for all nodes of the gltf file
        m_HasMesh.resize(m_GltfAsset.nodes.size(), false);
        {
            // if a scene ID was provided, use it, otherwise use scene 0
            int sceneIDLocal = (sceneID > Gltf::GLTF_NOT_USED) ? sceneID : 0;

            fastgltf::Scene const& scene = m_GltfAsset.scenes[sceneIDLocal];
            size_t nodeCount = scene.nodeIndices.size();
            for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
            {
                MarkNode(scene.nodeIndices[nodeIndex]);
            }
        }

        // PASS 2 (for all instances)
        m_InstanceCount = instanceCount;
        for (uint instanceIndex = 0; instanceIndex < m_InstanceCount; ++instanceIndex)
        {
            uint groupNode;
            if (m_GroupNode == Gltf::GLTF_NOT_USED)
            { // create group game object(s) for all instances to apply transform from JSON file to
                auto entity = m_Registry.Create();
                auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(instanceIndex) + "::root";
                groupNode = m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);

                {
                    TransformComponent transform{};
                    m_Registry.emplace<TransformComponent>(entity, transform);
                }
            }
            else
            {
                groupNode = static_cast<uint>(m_GroupNode);
            }

            // a scene ID was provided
            if (sceneID > Gltf::GLTF_NOT_USED)
            {
                ProcessScene(m_GltfAsset.scenes[sceneID], groupNode, instanceIndex);
            }
            else // no scene ID was provided --> use all scenes
            {
                for (auto& scene : m_GltfAsset.scenes)
                {
                    ProcessScene(scene, groupNode, instanceIndex);
                }
            }
        }
        return Gltf::GLTF_LOAD_SUCCESS;
    }

    bool FastgltfBuilder::Load(uint const instanceCount, std::vector<entt::entity>& firstInstances, bool useSceneGraph)
    {
        m_UseSceneGraph = useSceneGraph;
        bool returnValue = Load(instanceCount);
        firstInstances = m_FirstInstances;
        return returnValue;
    }

    bool FastgltfBuilder::MarkNode(int const gltfNodeIndex)
    {
        // each recursive call of this function marks a node in "m_HasMesh" if itself or a child has a mesh
        auto& node = m_GltfAsset.nodes[gltfNodeIndex];

        // does this gltf node have a mesh?
        bool localHasMesh = (node.meshIndex.has_value() || node.cameraIndex.has_value() || node.lightIndex.has_value());

        // do any of the child nodes have a mesh?
        size_t childNodeCount = node.children.size();
        for (size_t childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            int gltfChildNodeIndex = node.children[childNodeIndex];
            bool childHasMesh = MarkNode(gltfChildNodeIndex);
            localHasMesh = localHasMesh || childHasMesh;
        }
        m_HasMesh[gltfNodeIndex] = localHasMesh;
        return localHasMesh;
    }

    void FastgltfBuilder::ProcessScene(fastgltf::Scene& scene, uint const parentNode, uint instanceIndex)
    {
        size_t nodeCount = scene.nodeIndices.size();
        if (!nodeCount)
        {
            LOG_CORE_WARN("Builder::ProcessScene: empty scene in {0}", m_Filepath);
            return;
        }

        for (uint nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            ProcessNode(&scene, scene.nodeIndices[nodeIndex], parentNode, instanceIndex);
        }

        auto wait = [](std::future<bool>&& future) { future.get(); };
        m_NodeFuturesQueue.DoAll(wait);
    }

    void FastgltfBuilder::ProcessNode(fastgltf::Scene* scene, int const gltfNodeIndex, uint const parentNode,
                                      uint instanceIndex)
    {
        auto loadNode = [this, scene, gltfNodeIndex, parentNode, instanceIndex]()
        {
            ZoneScopedN("FastgltfBuilder::ProcessNode");
            auto& node = m_GltfAsset.nodes[gltfNodeIndex];
            std::string nodeName(node.name);

            uint currentNode = parentNode;

            if (m_HasMesh[gltfNodeIndex])
            {
                int meshIndex = node.meshIndex.has_value() ? node.meshIndex.value() : Gltf::GLTF_NOT_USED;
                int lightIndex = node.lightIndex.has_value() ? node.lightIndex.value() : Gltf::GLTF_NOT_USED;
                int cameraIndex = node.cameraIndex.has_value() ? node.cameraIndex.value() : Gltf::GLTF_NOT_USED;
                if ((meshIndex != Gltf::GLTF_NOT_USED) || (lightIndex != Gltf::GLTF_NOT_USED) ||
                    (cameraIndex != Gltf::GLTF_NOT_USED))
                {
                    currentNode = CreateGameObject(scene, gltfNodeIndex, parentNode, instanceIndex);
                }
                else // one or more children have a mesh, but not this one --> create group node
                {
                    if (m_UseSceneGraph)
                    {
                        std::lock_guard<std::mutex> guard(m_Mutex);
                        // create game object and transform component
                        auto entity = m_Registry.Create();

                        // create scene graph node and add to parent
                        auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(instanceIndex) +
                                    "::" + std::string(scene->name) + "::" + nodeName;
                        currentNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);

                        {
                            TransformComponent transform{};
                            LoadTransformationMatrix(transform, gltfNodeIndex);
                            m_Registry.emplace<TransformComponent>(entity, transform);
                        }
                    }
                }
            }

            size_t childNodeCount = node.children.size();
            for (size_t childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
            {
                int gltfChildNodeIndex = node.children[childNodeIndex];
                ProcessNode(scene, gltfChildNodeIndex, currentNode, instanceIndex);
            }
            return true;
        };
        m_NodeFuturesQueue.EmplaceBack(Engine::m_Engine->m_PoolSecondary.SubmitTask(loadNode));
    }

    uint FastgltfBuilder::CreateGameObject(fastgltf::Scene* scene, int const gltfNodeIndex, uint const parentNode,
                                           uint instanceIndex)
    {
        auto& node = m_GltfAsset.nodes[gltfNodeIndex];
        std::string nodeName(node.name);

        int meshIndex = node.meshIndex.has_value() ? node.meshIndex.value() : Gltf::GLTF_NOT_USED;
        int lightIndex = node.lightIndex.has_value() ? node.lightIndex.value() : Gltf::GLTF_NOT_USED;
        int cameraIndex = node.cameraIndex.has_value() ? node.cameraIndex.value() : Gltf::GLTF_NOT_USED;

        auto entity = m_Registry.Create();
        uint newNode{0};
        if (m_UseSceneGraph)
        {
            auto baseName = "::" + std::to_string(instanceIndex) + "::" + std::string(scene->name) + "::" + nodeName;
            auto name = m_DictionaryPrefix + "::" + m_Filepath + baseName;
            newNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);
        }

        TransformComponent transform{};
        LoadTransformationMatrix(transform, gltfNodeIndex);

        if (meshIndex != Gltf::GLTF_NOT_USED)
        {
            // create a model
            Model::ModelData modelData = {
                .m_Skeleton = m_Skeleton, .m_ShaderData = m_ShaderData, .m_Animations = m_Animations};

            // *** Instancing ***
            // create instance tag for first game object;
            // and collect further instances in it.
            // The renderer can loop over all instance tags
            // to retrieve the corresponding game objects.

            if (!instanceIndex)
            {
                InstanceTag instanceTag;
                instanceTag.m_Instances.resize(m_InstanceCount);
                instanceTag.m_Instances[instanceIndex] = entity;

                auto& instanceBuffer = instanceTag.m_InstanceBuffer;
                instanceBuffer = InstanceBuffer::Create(m_InstanceCount);
                instanceBuffer->SetInstanceData(instanceIndex, transform.GetMat4Global(), transform.GetNormalMatrix());

                m_Registry.emplace<InstanceTag>(entity, instanceTag);
                transform.SetInstance(instanceBuffer, instanceIndex);

                {
                    std::lock_guard<std::mutex> guard(m_Mutex);
                    m_InstancedObjects[gltfNodeIndex] = entity;
                    m_FirstInstances.push_back(entity);
                }

                // create model for 1st instance
                LoadVertexData(meshIndex, modelData);
                LOG_CORE_INFO("Vertex count: {0}, Index count: {1} (file: {2}, node: {3})", modelData.m_Vertices.size(),
                              modelData.m_Indices.size(), m_Filepath, nodeName);

                { // assign material
                    uint primitiveIndex = 0;
                    for (const auto& glTFPrimitive : m_GltfAsset.meshes[meshIndex].primitives)
                    {
                        Submesh& submesh = modelData.m_Submeshes[primitiveIndex];
                        ++primitiveIndex;

                        if (glTFPrimitive.materialIndex.has_value())
                        {
                            AssignMaterial(submesh, glTFPrimitive.materialIndex.value(), instanceBuffer.get());
                        }
                        else
                        {
                            LOG_CORE_ERROR("submesh has no material, check your 3D model");
                            AssignMaterial(submesh, Gltf::GLTF_NOT_USED, instanceBuffer.get());
                        }
                    }
                }

                // material tags (can have multiple tags)
                {
                    PbrMaterialTag pbrMaterialTag{};
                    m_Registry.emplace<PbrMaterialTag>(entity, pbrMaterialTag);
                }

                if (m_SkeletalAnimation)
                {
                    SkeletalAnimationTag skeletalAnimationTag{};
                    m_Registry.emplace<SkeletalAnimationTag>(entity, skeletalAnimationTag);
                }

                // submit to engine
                m_Models[gltfNodeIndex] = Engine::m_Engine->LoadModel(modelData);
            }
            else
            {
                std::lock_guard lock(m_Mutex);
                CORE_ASSERT(m_InstancedObjects.find(gltfNodeIndex) != m_InstancedObjects.end(),
                            "instanced object not found");
                entt::entity instance = m_InstancedObjects[gltfNodeIndex];
                InstanceTag& instanceTag = m_Registry.get<InstanceTag>(instance);
                instanceTag.m_Instances[instanceIndex] = entity;
                instanceTag.m_InstanceBuffer->SetInstanceData(instanceIndex, transform.GetMat4Global(),
                                                              transform.GetNormalMatrix());
                transform.SetInstance(instanceTag.m_InstanceBuffer, instanceIndex);
            }

            { // add mesh component to all instances
                MeshComponent mesh{nodeName, m_Models[gltfNodeIndex]};
                m_Registry.emplace<MeshComponent>(entity, mesh);
            }
        }
        else if (lightIndex != Gltf::GLTF_NOT_USED)
        {
            // create a light
            fastgltf::Light& glTFLight = m_GltfAsset.lights[lightIndex];
            switch (glTFLight.type)
            {
                case fastgltf::LightType::Directional:
                {
                    break;
                }
                case fastgltf::LightType::Spot:
                {
                    break;
                }
                case fastgltf::LightType::Point:
                {
                    PointLightComponent pointLightComponent{};
                    pointLightComponent.m_LightIntensity = glTFLight.intensity / 2500.0f;
                    pointLightComponent.m_Radius = glTFLight.range.has_value() ? glTFLight.range.value() : 0.1f;
                    pointLightComponent.m_Color = glm::make_vec3(glTFLight.color.data());

                    m_Registry.emplace<PointLightComponent>(entity, pointLightComponent);
                    break;
                }
                default:
                {
                    CORE_ASSERT(false, "fastgltfBuilder: type of light not supported");
                }
            }
        }
        else if (cameraIndex != Gltf::GLTF_NOT_USED)
        {
            // create a camera
            fastgltf::Camera& glTFCamera = m_GltfAsset.cameras[cameraIndex];
            if (const auto* pOrthographic = std::get_if<fastgltf::Camera::Orthographic>(&glTFCamera.camera))
            {
                float xmag = pOrthographic->xmag;
                float ymag = pOrthographic->ymag;
                float zfar = pOrthographic->zfar;
                float znear = pOrthographic->znear;

                OrthographicCameraComponent orthographicCameraComponent(xmag, ymag, zfar, znear);
                m_Registry.emplace<OrthographicCameraComponent>(entity, orthographicCameraComponent);
            }
            else if (const auto* pPerspective = std::get_if<fastgltf::Camera::Perspective>(&glTFCamera.camera))
            {
                float aspectRatio = pPerspective->aspectRatio.has_value() ? pPerspective->aspectRatio.value() : 1.0f;
                float yfov = pPerspective->yfov;
                float zfar = pPerspective->zfar.has_value() ? pPerspective->zfar.value() : 500.0f;
                float znear = pPerspective->znear;

                PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, zfar, znear);
                m_Registry.emplace<PerspectiveCameraComponent>(entity, perspectiveCameraComponent);
            }
        }
        m_Registry.emplace<TransformComponent>(entity, transform);

        return newNode;
    } // namespace GfxRenderEngine

    bool FastgltfBuilder::GetImageFormat(uint const imageIndex)
    {
        for (fastgltf::Material& material : m_GltfAsset.materials)
        {
            if (material.pbrData.baseColorTexture.has_value()) // albedo aka diffuse map aka bas color -> sRGB
            {
                uint diffuseTextureIndex = material.pbrData.baseColorTexture.value().textureIndex;
                auto& diffuseTexture = m_GltfAsset.textures[diffuseTextureIndex];
                if (imageIndex == diffuseTexture.imageIndex.value())
                {
                    return Texture::USE_SRGB;
                }
            }
            if (material.emissiveTexture.has_value())
            {
                uint emissiveTextureIndex = material.emissiveTexture.value().textureIndex;
                auto& emissiveTexture = m_GltfAsset.textures[emissiveTextureIndex];
                if (imageIndex == emissiveTexture.imageIndex.value())
                {
                    return Texture::USE_SRGB;
                }
            }
        }

        return Texture::USE_UNORM;
    }

    int FastgltfBuilder::GetMinFilter(uint index)
    {
        fastgltf::Filter filter = fastgltf::Filter::Linear;
        if (m_GltfAsset.textures[index].samplerIndex.has_value())
        {
            size_t sampler = m_GltfAsset.textures[index].samplerIndex.value();
            if (m_GltfAsset.samplers[sampler].minFilter.has_value())
            {
                filter = m_GltfAsset.samplers[sampler].minFilter.value();
            }
        }
        return static_cast<int>(filter);
    }

    int FastgltfBuilder::GetMagFilter(uint index)
    {
        fastgltf::Filter filter = fastgltf::Filter::Linear;
        if (m_GltfAsset.textures[index].samplerIndex.has_value())
        {
            size_t sampler = m_GltfAsset.textures[index].samplerIndex.value();
            if (m_GltfAsset.samplers[sampler].magFilter.has_value())
            {
                filter = m_GltfAsset.samplers[sampler].magFilter.value();
            }
        }
        return static_cast<int>(filter);
    }

    void FastgltfBuilder::LoadTextures()
    {
        size_t numTextures = m_GltfAsset.images.size();
        m_Textures.resize(numTextures);
        std::vector<std::future<std::shared_ptr<Texture>>> futures;
        futures.resize(numTextures);

        // retrieve all images from the glTF file
        for (uint imageIndex = 0; imageIndex < numTextures; ++imageIndex)
        {
            auto loadtexture = [this, imageIndex]()
            {
                ZoneScopedNC("FastgltfBuilder::LoadTextures", 0x0000ff);

                fastgltf::Image& glTFImage = m_GltfAsset.images[imageIndex];
                auto texture = Texture::Create();

                // image data is of type std::variant: the data type can be a URI/filepath, an Array, or a BufferView
                // std::visit calls the appropriate function
                std::visit(
                    fastgltf::visitor{
                        [&](fastgltf::sources::URI& filePath) // load from file name
                        {
                            const std::string imageFilepath(filePath.uri.path().begin(), filePath.uri.path().end());

                            CORE_ASSERT(filePath.fileByteOffset == 0, "no offset data support with stbi " + glTFImage.name);
                            CORE_ASSERT(filePath.uri.isLocalPath(), "no local file " + glTFImage.name);

                            int width = 0, height = 0, nrChannels = 0;
                            unsigned char* buffer =
                                stbi_load(imageFilepath.c_str(), &width, &height, &nrChannels, 4 /*int desired_channels*/);
                            CORE_ASSERT(buffer, "stbi failed (image data = URI) " + glTFImage.name);
                            CORE_ASSERT(nrChannels == 4, "wrong number of channels");

                            int minFilter = GetMinFilter(imageIndex);
                            int magFilter = GetMagFilter(imageIndex);
                            bool imageFormat = GetImageFormat(imageIndex);
                            texture->Init(width, height, imageFormat, buffer, minFilter, magFilter);

                            stbi_image_free(buffer);
                        },
                        [&](fastgltf::sources::Array& vector) // load from memory
                        {
                            int width = 0, height = 0, nrChannels = 0;

                            using byte = unsigned char;
                            byte* buffer = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                                 &width, &height, &nrChannels, 4 /*int desired_channels*/);
                            CORE_ASSERT(buffer, "stbi failed (image data = Array) " + glTFImage.name);

                            int minFilter = GetMinFilter(imageIndex);
                            int magFilter = GetMagFilter(imageIndex);
                            bool imageFormat = GetImageFormat(imageIndex);
                            texture->Init(width, height, imageFormat, buffer, minFilter, magFilter);

                            stbi_image_free(buffer);
                        },
                        [&](fastgltf::sources::BufferView& view) // load from buffer view
                        {
                            auto& bufferView = m_GltfAsset.bufferViews[view.bufferViewIndex];
                            auto& bufferFromBufferView = m_GltfAsset.buffers[bufferView.bufferIndex];

                            std::visit(
                                fastgltf::visitor{
                                    [&](auto& arg) // default branch if image data is not supported
                                    {
                                        LOG_CORE_CRITICAL("not supported default branch (image data = BUfferView) " +
                                                          glTFImage.name);
                                    },
                                    [&](fastgltf::sources::Array& vector) // load from memory
                                    {
                                        int width = 0, height = 0, nrChannels = 0;
                                        using byte = unsigned char;
                                        byte* buffer = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                             static_cast<int>(bufferView.byteLength), &width,
                                                                             &height, &nrChannels, 4);
                                        CORE_ASSERT(buffer, "stbi failed (image data = Array) " + glTFImage.name);

                                        int minFilter = GetMinFilter(imageIndex);
                                        int magFilter = GetMagFilter(imageIndex);
                                        bool imageFormat = GetImageFormat(imageIndex);
                                        texture->Init(width, height, imageFormat, buffer, minFilter, magFilter);

                                        stbi_image_free(buffer);
                                    }},
                                bufferFromBufferView.data);
                        },
                        [&](auto& arg) // default branch if image data is not supported
                        { LOG_CORE_CRITICAL("not supported default branch " + glTFImage.name); },
                    },
                    glTFImage.data);
                return texture;
            };
            futures[imageIndex] = Engine::m_Engine->m_PoolSecondary.SubmitTask(loadtexture);
        }
        for (uint imageIndex = 0; imageIndex < numTextures; ++imageIndex)
        {
            m_Textures[imageIndex] = futures[imageIndex].get();
        }
    }

    void FastgltfBuilder::LoadMaterials()
    {
        size_t numMaterials = m_GltfAsset.materials.size();
        m_Materials.resize(numMaterials);
        m_MaterialTextures.resize(numMaterials);

        uint materialIndex = 0;
        for (Material& material : m_Materials)
        {
            fastgltf::Material& glTFMaterial = m_GltfAsset.materials[materialIndex];
            Material::PbrMaterial& pbrMaterial = material.m_PbrMaterial;
            Material::MaterialTextures& materialTextures = m_MaterialTextures[materialIndex];

            // diffuse color aka base color factor
            // used as constant color, if no diffuse texture is provided
            // else, multiplied in the shader with each sample from the diffuse texture
            {
                pbrMaterial.m_DiffuseColor = glm::make_vec4(glTFMaterial.pbrData.baseColorFactor.data());
            }

            // diffuse map aka basecolor aka albedo
            if (glTFMaterial.pbrData.baseColorTexture.has_value())
            {
                uint diffuseMapIndex = glTFMaterial.pbrData.baseColorTexture.value().textureIndex;
                uint imageIndex = m_GltfAsset.textures[diffuseMapIndex].imageIndex.value();
                materialTextures[Material::DIFFUSE_MAP_INDEX] = m_Textures[imageIndex];
                pbrMaterial.m_Features |= Material::HAS_DIFFUSE_MAP;
            }

            // normal map
            if (glTFMaterial.normalTexture.has_value())
            {
                uint normalMapIndex = glTFMaterial.normalTexture.value().textureIndex;
                uint imageIndex = m_GltfAsset.textures[normalMapIndex].imageIndex.value();
                materialTextures[Material::NORMAL_MAP_INDEX] = m_Textures[imageIndex];
                pbrMaterial.m_NormalMapIntensity = glTFMaterial.normalTexture.value().scale;
                pbrMaterial.m_Features |= Material::HAS_NORMAL_MAP;
            }

            // constant values for roughness and metallicness
            {
                pbrMaterial.m_Roughness = glTFMaterial.pbrData.roughnessFactor;
                pbrMaterial.m_Metallic = glTFMaterial.pbrData.metallicFactor;
            }

            // texture for roughness and metallicness
            if (glTFMaterial.pbrData.metallicRoughnessTexture.has_value())
            {
                int metallicRoughnessMapIndex = glTFMaterial.pbrData.metallicRoughnessTexture.value().textureIndex;
                uint imageIndex = m_GltfAsset.textures[metallicRoughnessMapIndex].imageIndex.value();
                materialTextures[Material::ROUGHNESS_METALLIC_MAP_INDEX] = m_Textures[imageIndex];
                pbrMaterial.m_Features |= Material::HAS_ROUGHNESS_METALLIC_MAP;
            }

            // emissive color and emissive strength
            {
                pbrMaterial.m_EmissiveColor = glm::make_vec3(glTFMaterial.emissiveFactor.data());
                pbrMaterial.m_EmissiveStrength = glTFMaterial.emissiveStrength;
            }

            // emissive texture
            if (glTFMaterial.emissiveTexture.has_value())
            {
                uint emissiveTextureIndex = glTFMaterial.emissiveTexture.value().textureIndex;
                uint imageIndex = m_GltfAsset.textures[emissiveTextureIndex].imageIndex.value();
                materialTextures[Material::EMISSIVE_MAP_INDEX] = m_Textures[imageIndex];
                pbrMaterial.m_Features |= Material::HAS_EMISSIVE_MAP;
            }

            ++materialIndex;
        }
    }

    // load vertex data
    void FastgltfBuilder::LoadVertexData(uint const meshIndex, Model::ModelData& modelData)
    {
        ZoneScopedN("FastgltfBuilder::LoadVertexData");
        auto& vertices = modelData.m_Vertices;
        auto& indices = modelData.m_Indices;
        auto& submeshes = modelData.m_Submeshes;

        uint numPrimitives = m_GltfAsset.meshes[meshIndex].primitives.size();
        submeshes.resize(numPrimitives);

        uint primitiveIndex = 0;
        for (const auto& glTFPrimitive : m_GltfAsset.meshes[meshIndex].primitives)
        {
            Submesh& submesh = submeshes[primitiveIndex];
            ++primitiveIndex;

            submesh.m_FirstVertex = static_cast<uint>(vertices.size());
            submesh.m_FirstIndex = static_cast<uint>(indices.size());
            submesh.m_InstanceCount = m_InstanceCount;

            size_t vertexCount = 0;
            size_t indexCount = 0;

            glm::vec4 diffuseColor = glm::vec4(1.0f);
            if (glTFPrimitive.materialIndex.has_value())
            {
                size_t materialIndex = glTFPrimitive.materialIndex.value();
                CORE_ASSERT(materialIndex < m_Materials.size(),
                            "LoadVertexData: glTFPrimitive.materialIndex must be less than m_Materials.size()");
                diffuseColor = m_Materials[materialIndex].m_PbrMaterial.m_DiffuseColor;
            }

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const void* colorBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* tangentsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const uint* jointsBuffer = nullptr;
                const float* weightsBuffer = nullptr;

                fastgltf::ComponentType jointsBufferComponentType = fastgltf::ComponentType::Invalid;
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

                // Get buffer data for joints
                if (glTFPrimitive.findAttribute("JOINTS_0") != glTFPrimitive.attributes.end())
                {
                    jointsBufferComponentType = LoadAccessor<uint>(
                        m_GltfAsset.accessors[glTFPrimitive.findAttribute("JOINTS_0")->second], jointsBuffer);
                    auto glComponentType = fastgltf::getGLComponentType(jointsBufferComponentType);
                    CORE_ASSERT((glComponentType == GL_BYTE) || (glComponentType == GL_UNSIGNED_BYTE),
                                "unexpected component type " + std::to_string(glComponentType));
                }
                // Get buffer data for joint weights
                if (glTFPrimitive.findAttribute("WEIGHTS_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfAsset.accessors[glTFPrimitive.findAttribute("WEIGHTS_0")->second], weightsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }

                // Append data to model's vertex buffer
                uint numVerticesBefore = vertices.size();
                vertices.resize(numVerticesBefore + vertexCount);
                uint vertexIndex = numVerticesBefore;
                for (size_t vertexIterator = 0; vertexIterator < vertexCount; ++vertexIterator)
                {
                    Vertex vertex{};
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
                    vertex.m_Color = glm::vec4(vertexColor.x, vertexColor.y, vertexColor.z, 1.0f) * diffuseColor;

                    // normal
                    vertex.m_Normal = glm::normalize(
                        glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[vertexIterator * 3]) : glm::vec3(0.0f)));

                    // uv
                    auto uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[vertexIterator * 2]) : glm::vec3(0.0f);
                    vertex.m_UV = uv;

                    // tangent
                    glm::vec4 t = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[vertexIterator * 4]) : glm::vec4(0.0f);
                    vertex.m_Tangent = glm::vec3(t.x, t.y, t.z) * t.w;

                    // joint indices and joint weights
                    if (jointsBuffer && weightsBuffer)
                    {
                        switch (getGLComponentType(jointsBufferComponentType))
                        {
                            case GL_BYTE:
                            case GL_UNSIGNED_BYTE:
                                vertex.m_JointIds = glm::ivec4(
                                    glm::make_vec4(&(reinterpret_cast<const int8_t*>(jointsBuffer)[vertexIterator * 4])));
                                break;
                            case GL_SHORT:
                            case GL_UNSIGNED_SHORT:
                                vertex.m_JointIds = glm::ivec4(
                                    glm::make_vec4(&(reinterpret_cast<const int16_t*>(jointsBuffer)[vertexIterator * 4])));
                                break;
                            case GL_INT:
                            case GL_UNSIGNED_INT:
                                vertex.m_JointIds = glm::ivec4(
                                    glm::make_vec4(&(reinterpret_cast<const int32_t*>(jointsBuffer)[vertexIterator * 4])));
                                break;
                            default:
                                LOG_CORE_CRITICAL("data type of joints buffer not found");
                                break;
                        }
                        vertex.m_Weights = glm::make_vec4(&weightsBuffer[vertexIterator * 4]);
                    }
                    vertices[vertexIndex] = vertex;
                    ++vertexIndex;
                }

                // calculate tangents
                if (!tangentsBuffer)
                {
                    CalculateTangents(modelData);
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

            submesh.m_VertexCount = vertexCount;
            submesh.m_IndexCount = indexCount;
        }
    }

    void FastgltfBuilder::LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex)
    {
        auto& node = m_GltfAsset.nodes[gltfNodeIndex];

        std::visit(
            fastgltf::visitor{
                [&](auto& arg) // default branch if image data is not supported
                { LOG_CORE_CRITICAL("not supported default branch (LoadTransformationMatrix) "); },
                [&](fastgltf::TRS& TRS)
                {
                    transform.SetScale({TRS.scale[0], TRS.scale[1], TRS.scale[2]});
                    // note the order w, x, y, z
                    transform.SetRotation({TRS.rotation[3], TRS.rotation[0], TRS.rotation[1], TRS.rotation[2]});
                    transform.SetTranslation({TRS.translation[0], TRS.translation[1], TRS.translation[2]});
                },
                [&](fastgltf::Node::TransformMatrix& matrix) { transform.SetMat4Local(glm::make_mat4x4(matrix.data())); }},
            node.transform);
    }

    void FastgltfBuilder::AssignMaterial(Submesh& submesh, int const materialIndex, InstanceBuffer* instanceBuffer)
    {
        ZoneScopedN("AssignMaterial");
        { // material
            if (!(static_cast<size_t>(materialIndex) < m_Materials.size()))
            {
                LOG_CORE_CRITICAL("AssignMaterial: materialIndex must be less than m_Materials.size()");
            }

            Material& material = submesh.m_Material;

            // material
            if (materialIndex != Gltf::GLTF_NOT_USED)
            {
                material = m_Materials[materialIndex];
                material.m_MaterialTextures = m_MaterialTextures[materialIndex];
            }

            // create material descriptor
            material.m_MaterialDescriptor =
                MaterialDescriptor::Create(MaterialDescriptor::MaterialType::MtPbr, material.m_MaterialTextures);
        }

        { // resources
            Resources::ResourceBuffers& resourceBuffers = submesh.m_Resources.m_ResourceBuffers;
            resourceBuffers = m_ResourceBuffersPre;
            std::shared_ptr<Buffer> instanceUbo{instanceBuffer->GetBuffer()};
            resourceBuffers[Resources::INSTANCE_BUFFER_INDEX] = instanceUbo;
            if (m_SkeletalAnimation)
            {
                resourceBuffers[Resources::SKELETAL_ANIMATION_BUFFER_INDEX] = m_ShaderData;
            }
            submesh.m_Resources.m_ResourceDescriptor = ResourceDescriptor::Create(resourceBuffers);
        }

        LOG_CORE_INFO("material assigned (fastgltf): material index {0}", materialIndex);
    }

    void FastgltfBuilder::CalculateTangents(Model::ModelData& modelData)
    {
        if (modelData.m_Indices.size())
        {
            CalculateTangentsFromIndexBuffer(modelData, modelData.m_Indices);
        }
        else
        {
            uint vertexCount = modelData.m_Vertices.size();
            if (vertexCount)
            {
                std::vector<uint> indices;
                indices.resize(vertexCount);
                for (uint i = 0; i < vertexCount; i++)
                {
                    indices[i] = i;
                }
                CalculateTangentsFromIndexBuffer(modelData, indices);
            }
        }
    }

    void FastgltfBuilder::CalculateTangentsFromIndexBuffer(Model::ModelData& modelData, const std::vector<uint>& indices)
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
            auto& vertex = modelData.m_Vertices[index];

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

                    modelData.m_Vertices[vertexIndex1].m_Tangent = tangent;
                    modelData.m_Vertices[vertexIndex2].m_Tangent = tangent;
                    modelData.m_Vertices[vertexIndex3].m_Tangent = tangent;

                    break;
            }
            cnt = (cnt + 1) % 3;
        }
    }

    void FastgltfBuilder::SetDictionaryPrefix(std::string const& dictionaryPrefix) { m_DictionaryPrefix = dictionaryPrefix; }

    void FastgltfBuilder::PrintAssetError(fastgltf::Error assetErrorCode)
    {
        LOG_CORE_CRITICAL("FastgltfBuilder::Load: couldn't load {0}", m_Filepath);
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
