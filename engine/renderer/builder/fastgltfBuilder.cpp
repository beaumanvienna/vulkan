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

#include "gtc/type_ptr.hpp"
#include "stb_image.h"

#include "core.h"
#include "renderer/builder/fastgltfBuilder.h"
#include "renderer/materialDescriptor.h"
#include "auxiliary/file.h"

namespace GfxRenderEngine
{

    FastgltfBuilder::FastgltfBuilder(const std::string& filepath, Scene& scene)
        : m_Filepath{filepath}, m_SkeletalAnimation{0}, m_Registry{scene.GetRegistry()}, m_SceneGraph{scene.GetSceneGraph()},
          m_Dictionary{scene.GetDictionary()}, m_InstanceCount{0}, m_InstanceIndex{0}, m_MaterialFeatures{0}
    {
        m_Basepath = EngineCore::GetPathWithoutFilename(filepath);
    }

    bool FastgltfBuilder::LoadGltf(uint const instanceCount, int const sceneID)
    {
        { // load ascii from file
            auto path = std::filesystem::path{m_Filepath};

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
            m_GltfModel = std::move(asset.get());
        }

        if (!m_GltfModel.meshes.size() && !m_GltfModel.lights.size() && !m_GltfModel.cameras.size())
        {
            LOG_CORE_CRITICAL("LoadGltf: no meshes found in {0}", m_Filepath);
            return Gltf::GLTF_LOAD_FAILURE;
        }

        if (sceneID > Gltf::GLTF_NOT_USED) // a scene ID was provided
        {
            // check if valid
            if ((m_GltfModel.scenes.size() - 1) < static_cast<size_t>(sceneID))
            {
                LOG_CORE_CRITICAL("LoadGltf: scene not found in {0}", m_Filepath);
                return Gltf::GLTF_LOAD_FAILURE;
            }
        }

        LoadImagesGltf();
        LoadSkeletonsGltf();
        LoadMaterialsGltf();

        // PASS 1
        // mark gltf nodes to receive a game object ID if they have a mesh or any child has
        // --> create array of flags for all nodes of the gltf file
        m_HasMesh.resize(m_GltfModel.nodes.size(), false);
        {
            // if a scene ID was provided, use it, otherwise use scene 0
            int sceneIDLocal = (sceneID > Gltf::GLTF_NOT_USED) ? sceneID : 0;

            fastgltf::Scene& scene = m_GltfModel.scenes[sceneIDLocal];
            size_t nodeCount = scene.nodeIndices.size();
            for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
            {
                MarkNode(scene, scene.nodeIndices[nodeIndex]);
            }
        }

        // PASS 2 (for all instances)
        m_InstanceCount = instanceCount;
        for (m_InstanceIndex = 0; m_InstanceIndex < m_InstanceCount; ++m_InstanceIndex)
        {
            // create group game object(s) for all instances to apply transform from JSON file to
            auto entity = m_Registry.create();

            std::string name = EngineCore::GetFilenameWithoutPathAndExtension(m_Filepath);
            auto shortName = name + "::" + std::to_string(m_InstanceIndex) + "::root";
            auto longName = m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::root";
            uint groupNode = m_SceneGraph.CreateNode(entity, shortName, longName, m_Dictionary);
            m_SceneGraph.GetRoot().AddChild(groupNode);

            {
                TransformComponent transform{};
                m_Registry.emplace<TransformComponent>(entity, transform);
            }

            // a scene ID was provided
            if (sceneID > Gltf::GLTF_NOT_USED)
            {
                ProcessScene(m_GltfModel.scenes[sceneID], groupNode);
            }
            else // no scene ID was provided --> use all scenes
            {
                for (auto& scene : m_GltfModel.scenes)
                {
                    ProcessScene(scene, groupNode);
                }
            }
        }
        return Gltf::GLTF_LOAD_SUCCESS;
    }

    bool FastgltfBuilder::MarkNode(fastgltf::Scene& scene, int const gltfNodeIndex)
    {
        // each recursive call of this function marks a node in "m_HasMesh" if itself or a child has a mesh
        auto& node = m_GltfModel.nodes[gltfNodeIndex];

        // does this gltf node have a mesh?
        bool localHasMesh = (node.meshIndex.has_value() || node.cameraIndex.has_value() || node.lightIndex.has_value());

        // do any of the child nodes have a mesh?
        size_t childNodeCount = node.children.size();
        for (size_t childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            int gltfChildNodeIndex = node.children[childNodeIndex];
            bool childHasMesh = MarkNode(scene, gltfChildNodeIndex);
            localHasMesh = localHasMesh || childHasMesh;
        }
        m_HasMesh[gltfNodeIndex] = localHasMesh;
        return localHasMesh;
    }

    void FastgltfBuilder::ProcessScene(fastgltf::Scene& scene, uint const parentNode)
    {
        size_t nodeCount = scene.nodeIndices.size();
        if (!nodeCount)
        {
            LOG_CORE_WARN("Builder::ProcessScene: empty scene in {0}", m_Filepath);
            return;
        }

        m_RenderObject = 0;
        for (uint nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            ProcessNode(scene, scene.nodeIndices[nodeIndex], parentNode);
        }
    }

    void FastgltfBuilder::ProcessNode(fastgltf::Scene& scene, int const gltfNodeIndex, uint const parentNode)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];
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
                currentNode = CreateGameObject(scene, gltfNodeIndex, parentNode);
            }
            else // one or more children have a mesh, but not this one --> create group node
            {
                // create game object and transform component
                auto entity = m_Registry.create();

                // create scene graph node and add to parent
                auto shortName = "::" + std::to_string(m_InstanceIndex) + "::" + std::string(scene.name) + "::" + nodeName;
                auto longName = m_Filepath + shortName;
                currentNode = m_SceneGraph.CreateNode(entity, shortName, longName, m_Dictionary);
                m_SceneGraph.GetNode(parentNode).AddChild(currentNode);

                {
                    TransformComponent transform{};
                    LoadTransformationMatrix(transform, gltfNodeIndex);
                    m_Registry.emplace<TransformComponent>(entity, transform);
                }
            }
        }

        size_t childNodeCount = node.children.size();
        for (size_t childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            int gltfChildNodeIndex = node.children[childNodeIndex];
            ProcessNode(scene, gltfChildNodeIndex, currentNode);
        }
    }

    uint FastgltfBuilder::CreateGameObject(fastgltf::Scene& scene, int const gltfNodeIndex, uint const parentNode)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];
        std::string nodeName(node.name);
        int meshIndex = node.meshIndex.has_value() ? node.meshIndex.value() : Gltf::GLTF_NOT_USED;
        int lightIndex = node.lightIndex.has_value() ? node.lightIndex.value() : Gltf::GLTF_NOT_USED;
        int cameraIndex = node.cameraIndex.has_value() ? node.cameraIndex.value() : Gltf::GLTF_NOT_USED;

        auto entity = m_Registry.create();
        auto baseName = "::" + std::to_string(m_InstanceIndex) + "::" + std::string(scene.name) + "::" + nodeName;
        auto shortName = EngineCore::GetFilenameWithoutPathAndExtension(m_Filepath) + baseName;
        auto longName = m_Filepath + baseName;

        uint newNode = m_SceneGraph.CreateNode(entity, shortName, longName, m_Dictionary);
        m_SceneGraph.GetNode(parentNode).AddChild(newNode);

        TransformComponent transform{};
        LoadTransformationMatrix(transform, gltfNodeIndex);

        if (meshIndex != Gltf::GLTF_NOT_USED)
        {
            // create a model

            // *** Instancing ***
            // create instance tag for first game object;
            // and collect further instances in it.
            // The renderer can loop over all instance tags
            // to retrieve the corresponding game objects.

            if (!m_InstanceIndex)
            {
                InstanceTag instanceTag;
                instanceTag.m_Instances.push_back(entity);
                m_InstanceBuffer = InstanceBuffer::Create(m_InstanceCount);
                instanceTag.m_InstanceBuffer = m_InstanceBuffer;
                instanceTag.m_InstanceBuffer->SetInstanceData(m_InstanceIndex, transform.GetMat4Global(),
                                                              transform.GetNormalMatrix());
                m_Registry.emplace<InstanceTag>(entity, instanceTag);
                transform.SetInstance(m_InstanceBuffer, m_InstanceIndex);
                m_InstancedObjects.push_back(entity);

                // create model for 1st instance
                LoadVertexDataGltf(meshIndex);
                LOG_CORE_INFO("Vertex count: {0}, Index count: {1} (file: {2}, node: {3})", m_Vertices.size(),
                              m_Indices.size(), m_Filepath, nodeName);
                { // assign material
                    uint primitiveIndex = 0;
                    for (const auto& glTFPrimitive : m_GltfModel.meshes[meshIndex].primitives)
                    {
                        ModelSubmesh& submesh = m_Submeshes[primitiveIndex++];

                        if (glTFPrimitive.materialIndex.has_value())
                        {
                            AssignMaterial(submesh, glTFPrimitive.materialIndex.value());
                        }
                        else
                        {
                            LOG_CORE_ERROR("submesh has no material, check your 3D model");
                            AssignMaterial(submesh, Gltf::GLTF_NOT_USED);
                        }
                    }
                }
                m_Model = Engine::m_Engine->LoadModel(*this);

                // material tags (can have multiple tags)
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrNoMap)
                {
                    PbrNoMapTag pbrNoMapTag{};
                    m_Registry.emplace<PbrNoMapTag>(entity, pbrNoMapTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseMap)
                {
                    PbrDiffuseTag pbrDiffuseTag{};
                    m_Registry.emplace<PbrDiffuseTag>(entity, pbrDiffuseTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseSAMap)
                {
                    PbrDiffuseSATag pbrDiffuseSATag{};
                    m_Registry.emplace<PbrDiffuseSATag>(entity, pbrDiffuseSATag);

                    SkeletalAnimationTag skeletalAnimationTag{};
                    m_Registry.emplace<SkeletalAnimationTag>(entity, skeletalAnimationTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalMap)
                {
                    PbrDiffuseNormalTag pbrDiffuseNormalTag;
                    m_Registry.emplace<PbrDiffuseNormalTag>(entity, pbrDiffuseNormalTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalSAMap)
                {
                    PbrDiffuseNormalSATag pbrDiffuseNormalSATag;
                    m_Registry.emplace<PbrDiffuseNormalSATag>(entity, pbrDiffuseNormalSATag);

                    SkeletalAnimationTag skeletalAnimationTag{};
                    m_Registry.emplace<SkeletalAnimationTag>(entity, skeletalAnimationTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMap)
                {
                    PbrDiffuseNormalRoughnessMetallicTag pbrDiffuseNormalRoughnessMetallicTag;
                    m_Registry.emplace<PbrDiffuseNormalRoughnessMetallicTag>(entity, pbrDiffuseNormalRoughnessMetallicTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSAMap)
                {
                    PbrDiffuseNormalRoughnessMetallicSATag pbrDiffuseNormalRoughnessMetallicSATag;
                    m_Registry.emplace<PbrDiffuseNormalRoughnessMetallicSATag>(entity,
                                                                               pbrDiffuseNormalRoughnessMetallicSATag);

                    SkeletalAnimationTag skeletalAnimationTag{};
                    m_Registry.emplace<SkeletalAnimationTag>(entity, skeletalAnimationTag);
                }

                // emissive materials
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrEmissive)
                {
                    PbrEmissiveTag pbrEmissiveTag{};
                    m_Registry.emplace<PbrEmissiveTag>(entity, pbrEmissiveTag);
                }
                if (m_MaterialFeatures & MaterialDescriptor::MtPbrEmissiveTexture)
                {
                    PbrEmissiveTextureTag pbrEmissiveTextureTag{};
                    m_Registry.emplace<PbrEmissiveTextureTag>(entity, pbrEmissiveTextureTag);
                }

                if (m_MaterialFeatures & MaterialDescriptor::ALL_PBR_MATERIALS)
                {
                    PbrMaterial pbrMaterial{};
                    m_Registry.emplace<PbrMaterial>(entity, pbrMaterial);
                }
            }
            else
            {
                entt::entity instance = m_InstancedObjects[m_RenderObject++];
                InstanceTag& instanceTag = m_Registry.get<InstanceTag>(instance);
                instanceTag.m_Instances.push_back(entity);
                instanceTag.m_InstanceBuffer->SetInstanceData(m_InstanceIndex, transform.GetMat4Global(),
                                                              transform.GetNormalMatrix());
                transform.SetInstance(instanceTag.m_InstanceBuffer, m_InstanceIndex);
            }

            { // add mesh component to all instances
                MeshComponent mesh{nodeName, m_Model};
                m_Registry.emplace<MeshComponent>(entity, mesh);
            }
        }
        else if (lightIndex != Gltf::GLTF_NOT_USED)
        {
            // create a light
            fastgltf::Light& glTFLight = m_GltfModel.lights[lightIndex];
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
            fastgltf::Camera& glTFCamera = m_GltfModel.cameras[cameraIndex];
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
                float yfov = pPerspective->yfov * 100;
                float zfar = pPerspective->zfar.has_value() ? pPerspective->zfar.value() : 100.0f;
                float znear = pPerspective->znear;

                PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, zfar, znear);
                m_Registry.emplace<PerspectiveCameraComponent>(entity, perspectiveCameraComponent);
            }
        }
        m_Registry.emplace<TransformComponent>(entity, transform);

        return newNode;
    }

    bool FastgltfBuilder::GetImageFormatGltf(uint const imageIndex)
    {
        for (fastgltf::Material& material : m_GltfModel.materials)
        {
            if (material.pbrData.baseColorTexture.has_value()) // albedo aka diffuse map aka bas color -> sRGB
            {
                uint diffuseTextureIndex = material.pbrData.baseColorTexture.value().textureIndex;
                auto& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                if (imageIndex == diffuseTexture.imageIndex.value())
                {
                    return Texture::USE_SRGB;
                }
            }
            else if (material.emissiveTexture.has_value())
            {
                uint emissiveTextureIndex = material.emissiveTexture.value().textureIndex;
                auto& emissiveTexture = m_GltfModel.textures[emissiveTextureIndex];
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
        if (m_GltfModel.textures[index].samplerIndex.has_value())
        {
            size_t sampler = m_GltfModel.textures[index].samplerIndex.value();
            if (m_GltfModel.samplers[sampler].minFilter.has_value())
            {
                filter = m_GltfModel.samplers[sampler].minFilter.value();
            }
        }
        return static_cast<int>(filter);
    }

    int FastgltfBuilder::GetMagFilter(uint index)
    {
        fastgltf::Filter filter = fastgltf::Filter::Linear;
        if (m_GltfModel.textures[index].samplerIndex.has_value())
        {
            size_t sampler = m_GltfModel.textures[index].samplerIndex.value();
            if (m_GltfModel.samplers[sampler].magFilter.has_value())
            {
                filter = m_GltfModel.samplers[sampler].magFilter.value();
            }
        }
        return static_cast<int>(filter);
    }

    void FastgltfBuilder::LoadImagesGltf()
    {
        size_t numImages = m_GltfModel.images.size();
        m_Images.resize(numImages);

        // retrieve all images from the glTF file
        for (uint imageIndex = 0; imageIndex < numImages; ++imageIndex)
        {
            fastgltf::Image& glTFImage = m_GltfModel.images[imageIndex];
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
                        int magFilter = GetMinFilter(imageIndex);
                        bool imageFormat = GetImageFormatGltf(imageIndex);
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
                        int magFilter = GetMinFilter(imageIndex);
                        bool imageFormat = GetImageFormatGltf(imageIndex);
                        texture->Init(width, height, imageFormat, buffer, minFilter, magFilter);

                        stbi_image_free(buffer);
                    },
                    [&](fastgltf::sources::BufferView& view)
                    {
                        auto& bufferView = m_GltfModel.bufferViews[view.bufferViewIndex];
                        auto& bufferFromBufferView = m_GltfModel.buffers[bufferView.bufferIndex];

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
                                    int magFilter = GetMinFilter(imageIndex);
                                    bool imageFormat = GetImageFormatGltf(imageIndex);
                                    texture->Init(width, height, imageFormat, buffer, minFilter, magFilter);

                                    stbi_image_free(buffer);
                                }},
                            bufferFromBufferView.data);
                    },
                    [&](auto& arg) // default branch if image data is not supported
                    { LOG_CORE_CRITICAL("not supported default branch " + glTFImage.name); },
                },
                glTFImage.data);

            m_Images[imageIndex] = texture;
        }
    }

    void FastgltfBuilder::LoadMaterialsGltf()
    {
        size_t numMaterials = m_GltfModel.materials.size();
        m_Materials.resize(numMaterials);
        for (uint materialIndex = 0; materialIndex < numMaterials; ++materialIndex)
        {
            fastgltf::Material& glTFMaterial = m_GltfModel.materials[materialIndex];

            Material material{};
            material.m_Features = m_SkeletalAnimation;
            material.m_Roughness = glTFMaterial.pbrData.roughnessFactor;
            material.m_Metallic = glTFMaterial.pbrData.metallicFactor;

            // diffuse color
            {
                material.m_DiffuseColor = glm::make_vec3(glTFMaterial.pbrData.baseColorFactor.data());
            }

            // diffuse map aka basecolor aka albedo
            if (glTFMaterial.pbrData.baseColorTexture.has_value())
            {
                uint diffuseMapIndex = glTFMaterial.pbrData.baseColorTexture.value().textureIndex;
                material.m_DiffuseMapIndex = m_GltfModel.textures[diffuseMapIndex].imageIndex.value();
                material.m_Features |= Material::HAS_DIFFUSE_MAP;
            }

            // normal map
            if (glTFMaterial.normalTexture.has_value())
            {
                uint normalMapIndex = glTFMaterial.normalTexture.value().textureIndex;
                material.m_NormalMapIndex = m_GltfModel.textures[normalMapIndex].imageIndex.value();
                material.m_NormalMapIntensity = glTFMaterial.normalTexture.value().scale;
                material.m_Features |= Material::HAS_NORMAL_MAP;
            }

            if (glTFMaterial.pbrData.metallicRoughnessTexture.has_value())
            {
                int metallicRoughnessMapIndex = glTFMaterial.pbrData.metallicRoughnessTexture.value().textureIndex;
                material.m_RoughnessMetallicMapIndex = m_GltfModel.textures[metallicRoughnessMapIndex].imageIndex.value();
                material.m_Features |= Material::HAS_ROUGHNESS_METALLIC_MAP;
            }

            // emissive factor and emissive strength
            {
                // emissive factor
                glm::vec3 emissiveFactor = glm::make_vec3(glTFMaterial.emissiveFactor.data());
                material.m_EmissiveFactor = emissiveFactor;

                // emissive strength
                // set the emissive strength only if an emissive texture was provided or an emissive vertex color via
                // emissiveFactor in either way, emissiveFactor must have been provided (no need to also check for an
                // emissive texture via glTFMaterial.emissiveTexture.has_value()) emissive strength is just an extra
                // value via an extension
                // ("emissiveStrength" is normally not provided, but factored into emssiveFactor, default is 1.0f)
                bool emissiveMaterial = (emissiveFactor != glm::vec3(0, 0, 0));
                if (emissiveMaterial)
                {
                    // simplified (only using 1st component of emissive factor, instead of all three)
                    material.m_EmissiveStrength = glTFMaterial.emissiveStrength * material.m_EmissiveFactor.r;
                }
                else
                {
                    // emissive texture nor emissive vertex color provided -> do not use as emissive material
                    material.m_EmissiveStrength = 0.0f;
                }
            }

            // emissive texture
            if (glTFMaterial.emissiveTexture.has_value())
            {
                uint emissiveTextureIndex = glTFMaterial.emissiveTexture.value().textureIndex;
                material.m_EmissiveMapIndex = m_GltfModel.textures[emissiveTextureIndex].imageIndex.value();
                material.m_Features |= Material::HAS_EMISSIVE_MAP;
            }

            m_Materials[materialIndex] = material;
        }
    }

    void FastgltfBuilder::LoadVertexDataGltf(uint const meshIndex)
    {
        // handle vertex data
        m_Vertices.clear();
        m_Indices.clear();
        m_Submeshes.clear();
        m_MaterialFeatures = 0;

        uint numPrimitives = m_GltfModel.meshes[meshIndex].primitives.size();
        m_Submeshes.resize(numPrimitives);
        uint primitiveIndex = 0;
        for (const auto& glTFPrimitive : m_GltfModel.meshes[meshIndex].primitives)
        {
            ModelSubmesh& submesh = m_Submeshes[primitiveIndex++];

            submesh.m_FirstVertex = static_cast<uint32_t>(m_Vertices.size());
            submesh.m_FirstIndex = static_cast<uint32_t>(m_Indices.size());
            submesh.m_InstanceCount = m_InstanceCount;

            size_t vertexCount = 0;
            size_t indexCount = 0;

            glm::vec3 diffuseColor = glm::vec3(0.5f, 0.5f, 1.0f);
            if (glTFPrimitive.materialIndex.has_value())
            {
                size_t materialIndex = glTFPrimitive.materialIndex.value();
                CORE_ASSERT(materialIndex < m_Materials.size(),
                            "LoadVertexDataGltf: glTFPrimitive.materialIndex must be less than m_Materials.size()");
                diffuseColor = m_Materials[materialIndex].m_DiffuseColor;
            }

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* tangentsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const uint* jointsBuffer = nullptr;
                const float* weightsBuffer = nullptr;

                fastgltf::ComponentType jointsBufferDataType = fastgltf::ComponentType::Invalid;

                // Get buffer data for vertex positions
                if (glTFPrimitive.findAttribute("POSITION") != glTFPrimitive.attributes.end())
                {
                    auto componentType =
                        LoadAccessor<float>(m_GltfModel.accessors[glTFPrimitive.findAttribute("POSITION")->second],
                                            positionBuffer, &vertexCount);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.findAttribute("NORMAL") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.findAttribute("NORMAL")->second], normalsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex tangents
                if (glTFPrimitive.findAttribute("TANGENT") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.findAttribute("TANGENT")->second], tangentsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.findAttribute("TEXCOORD_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.findAttribute("TEXCOORD_0")->second], texCoordsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }

                // Get buffer data for joints
                if (glTFPrimitive.findAttribute("JOINTS_0") != glTFPrimitive.attributes.end())
                {
                    jointsBufferDataType = LoadAccessor<uint>(
                        m_GltfModel.accessors[glTFPrimitive.findAttribute("JOINTS_0")->second], jointsBuffer);
                    CORE_ASSERT((fastgltf::getGLComponentType(jointsBufferDataType) == GL_BYTE) ||
                                    (fastgltf::getGLComponentType(jointsBufferDataType) == GL_UNSIGNED_BYTE),
                                "unexpected component type");
                }
                // Get buffer data for joint weights
                if (glTFPrimitive.findAttribute("WEIGHTS_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.findAttribute("WEIGHTS_0")->second], weightsBuffer);
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                }

                // Append data to model's vertex buffer
                uint numVerticesBefore = m_Vertices.size();
                m_Vertices.resize(numVerticesBefore + vertexCount);
                uint vertexIndex = numVerticesBefore;
                for (size_t v = 0; v < vertexCount; v++)
                {
                    Vertex vertex{};
                    vertex.m_Amplification = 1.0f;
                    auto position = positionBuffer ? glm::make_vec3(&positionBuffer[v * 3]) : glm::vec3(0.0f);
                    vertex.m_Position = glm::vec3(position.x, position.y, position.z);
                    vertex.m_Normal =
                        glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));

                    glm::vec4 t = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f);
                    vertex.m_Tangent = glm::vec3(t.x, t.y, t.z) * t.w;

                    auto uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                    vertex.m_UV = glm::vec2(uv.x, 1.0f - uv.y);
                    vertex.m_Color = diffuseColor;
                    if (jointsBuffer && weightsBuffer)
                    {
                        switch (getGLComponentType(jointsBufferDataType))
                        {
                            case GL_BYTE:
                            case GL_UNSIGNED_BYTE:
                                vertex.m_JointIds =
                                    glm::ivec4(glm::make_vec4(&(reinterpret_cast<const int8_t*>(jointsBuffer)[v * 4])));
                                break;
                            case GL_SHORT:
                            case GL_UNSIGNED_SHORT:
                                vertex.m_JointIds =
                                    glm::ivec4(glm::make_vec4(&(reinterpret_cast<const int16_t*>(jointsBuffer)[v * 4])));
                                break;
                            case GL_INT:
                            case GL_UNSIGNED_INT:
                                vertex.m_JointIds =
                                    glm::ivec4(glm::make_vec4(&(reinterpret_cast<const int32_t*>(jointsBuffer)[v * 4])));
                                break;
                            default:
                                LOG_CORE_CRITICAL("data type of joints buffer not found");
                                break;
                        }
                        vertex.m_Weights = glm::make_vec4(&weightsBuffer[v * 4]);
                    }
                    m_Vertices[vertexIndex] = vertex;
                    ++vertexIndex;
                }

                // calculate tangents
                if (!tangentsBuffer)
                {
                    LOG_CORE_CRITICAL("no tangents in gltf file found, calculating tangents manually");
                    CalculateTangents();
                }
            }

            // Indices
            if (glTFPrimitive.indicesAccessor.has_value())
            {
                {
                    auto& accessor = m_GltfModel.accessors[glTFPrimitive.indicesAccessor.value()];
                    indexCount = accessor.count;

                    // append indices for submesh to global index array
                    size_t globalIndicesOffset = m_Indices.size();
                    m_Indices.resize(m_Indices.size() + indexCount);

                    std::vector<uint> submeshIndices;
                    submeshIndices.resize(indexCount);

                    fastgltf::iterateAccessorWithIndex<uint>(m_GltfModel, accessor,
                                                             [&](uint submeshIndex, size_t iterator)
                                                             { submeshIndices[iterator] = submeshIndex; });

                    // copy submesh indices into global index array
                    uint* src = submeshIndices.data();
                    uint* dest = m_Indices.data() + globalIndicesOffset;
                    uint length = indexCount * sizeof(uint);
                    memcpy(dest, src, length);
                }
            }

            submesh.m_VertexCount = vertexCount;
            submesh.m_IndexCount = indexCount;
        }
    }

    void FastgltfBuilder::LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];

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

    void FastgltfBuilder::AssignMaterial(ModelSubmesh& submesh, int const materialIndex)
    {
        if (materialIndex == Gltf::GLTF_NOT_USED)
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMapInstanced, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrNoMap;
            submesh.m_MaterialProperties.m_Roughness = 0.5f;
            submesh.m_MaterialProperties.m_Metallic = 0.1f;

            LOG_CORE_INFO("default material assigned: material index {0}, PbrNoMap (1)", materialIndex);
            return;
        }

        if (!(static_cast<size_t>(materialIndex) < m_Materials.size()))
        {
            LOG_CORE_CRITICAL("AssignMaterial: materialIndex must be less than m_Materials.size()");
        }

        auto& material = m_Materials[materialIndex];
        // assign only those material features that are actually needed in the renderer
        submesh.m_MaterialProperties.m_NormalMapIntensity = material.m_NormalMapIntensity;
        submesh.m_MaterialProperties.m_Roughness = material.m_Roughness;
        submesh.m_MaterialProperties.m_Metallic = material.m_Metallic;
        submesh.m_MaterialProperties.m_EmissiveStrength = material.m_EmissiveStrength;

        uint pbrFeatures = material.m_Features & (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP |
                                                  Material::HAS_ROUGHNESS_METALLIC_MAP | Material::HAS_SKELETAL_ANIMATION);

        if (pbrFeatures == Material::NO_MAP)
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMapInstanced, instanceUbo);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrNoMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrNoMap (2), features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == Material::HAS_DIFFUSE_MAP)
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex]};
                std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseMapInstanced, textures, instanceUbo);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuse, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseSAMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseSAMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: vdiffuseSAMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseSAMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseSAMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseSAMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseSA, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: normalMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex]};
                std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseNormalMapInstanced, textures, instanceUbo);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormal, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: normalMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseNormalSAMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalSAMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalSA, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures ==
                 (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMetallicMapIndex = material.m_RoughnessMetallicMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex            < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: normalMapIndex             < m_Images.size()");
            CORE_ASSERT(roughnessMetallicMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: roughnessMetallicMapIndex  < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMetallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMapInstanced, textures, instanceUbo);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallic, features: 0x{1:x}",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP |
                                 Material::HAS_ROUGHNESS_METALLIC_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMetallicMapIndex = material.m_RoughnessMetallicMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex            < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: normalMapIndex             < m_Images.size()");
            CORE_ASSERT(roughnessMetallicMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: roughnessMetallicMapIndex  < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMetallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSAMap, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSAMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallicSA, features: 0x{1:x}",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            LOG_CORE_CRITICAL("material diffuseRoughnessMetallic not supported");
        }
        else if (pbrFeatures & (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_METALLIC_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMetallicMapIndex = material.m_RoughnessMetallicMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex            < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: normalMapIndex             < m_Images.size()");
            CORE_ASSERT(roughnessMetallicMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: roughnessMetallicMapIndex  < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMetallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMap, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallic, features: 0x{1:x}",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures & Material::HAS_DIFFUSE_MAP)
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "FastgltfBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseMap, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuse, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMap, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
            }
            m_MaterialFeatures |= MaterialDescriptor::MtPbrNoMap;

            LOG_CORE_INFO("material assigned: material index {0}, PbrNoMap (3), features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }

        // emissive materials
        if (material.m_EmissiveStrength != 0)
        {
            // emissive texture
            if (material.m_Features & Material::HAS_EMISSIVE_MAP)
            {
                uint emissiveMapIndex = material.m_EmissiveMapIndex;
                CORE_ASSERT(emissiveMapIndex < m_Images.size(),
                            "FastgltfBuilder::AssignMaterial: emissiveMapIndex < m_Images.size()");

                { // create material descriptor
                    std::vector<std::shared_ptr<Texture>> textures{m_Images[emissiveMapIndex]};
                    std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                    auto materialDescriptor =
                        MaterialDescriptor::Create(MaterialDescriptor::MtPbrEmissiveTextureInstanced, textures, instanceUbo);
                    submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                }
                m_MaterialFeatures |= MaterialDescriptor::MtPbrEmissiveTexture;

                LOG_CORE_INFO("material assigned: material index {0}, PbrEmissiveTexture, features: 0x{1:x}", materialIndex,
                              material.m_Features);
            }
            else // emissive vertex color
            {
                { // create material descriptor

                    std::vector<std::shared_ptr<Buffer>> instanceUbo{m_InstanceBuffer->GetBuffer()};
                    auto materialDescriptor =
                        MaterialDescriptor::Create(MaterialDescriptor::MtPbrEmissiveInstanced, instanceUbo);
                    submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                }
                m_MaterialFeatures |= MaterialDescriptor::MtPbrEmissive;

                LOG_CORE_INFO("material assigned: material index {0}, PbrEmissive, features: 0x{1:x}", materialIndex,
                              material.m_Features);
            }
        }
    }

    void FastgltfBuilder::CalculateTangents()
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

    void FastgltfBuilder::CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices)
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
                        tangent = glm::vec3(1.0f, 0.0f, 0.0f);

                    m_Vertices[vertexIndex1].m_Tangent = tangent;
                    m_Vertices[vertexIndex2].m_Tangent = tangent;
                    m_Vertices[vertexIndex3].m_Tangent = tangent;

                    break;
            }
            cnt = (cnt + 1) % 3;
        }
    }

    void FastgltfBuilder::PrintAssetError(fastgltf::Error assetErrorCode)
    {
        LOG_CORE_CRITICAL("FastgltfBuilder::LoadGltf: couldn't load {0}", m_Filepath);
        switch (assetErrorCode)
        {
            case fastgltf::Error::None:
            {
                LOG_CORE_CRITICAL("error code: ");
                break;
            }
            case fastgltf::Error::InvalidPath:
            {
                LOG_CORE_CRITICAL("error code: The glTF directory passed to loadGLTF is invalid.");
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
