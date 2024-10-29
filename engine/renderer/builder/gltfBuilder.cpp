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
#include "renderer/instanceBuffer.h"
#include "renderer/builder/gltfBuilder.h"
#include "renderer/materialDescriptor.h"
#include "auxiliary/instrumentation.h"
#include "auxiliary/file.h"

namespace GfxRenderEngine
{

    GltfBuilder::GltfBuilder(const std::string& filepath, Scene& scene)
        : m_Filepath{filepath}, m_SkeletalAnimation{false}, m_Registry{scene.GetRegistry()},
          m_SceneGraph{scene.GetSceneGraph()}, m_Dictionary{scene.GetDictionary()}, m_InstanceCount{0}, m_InstanceIndex{0}
    {
        m_Basepath = EngineCore::GetPathWithoutFilename(filepath);
    }

    bool GltfBuilder::Load(uint const instanceCount, int const sceneID)
    {
        PROFILE_SCOPE("GltfBuilder::Load");
        stbi_set_flip_vertically_on_load(false);
        auto extension = EngineCore::GetFileExtension(m_Filepath);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        { // load from file
            std::string warn, err;
            if (extension == ".glb")
            {
                if (!m_GltfLoader.LoadBinaryFromFile(&m_GltfModel, &err, &warn, m_Filepath))
                {
                    LOG_CORE_CRITICAL("Load errors glb: {0}, warnings: {1}", err, warn);
                    return Gltf::GLTF_LOAD_FAILURE;
                }
            }
            else if (extension == ".gltf")
            {
                if (!m_GltfLoader.LoadASCIIFromFile(&m_GltfModel, &err, &warn, m_Filepath))
                {
                    LOG_CORE_CRITICAL("Load errors gltf: {0}, warnings: {1}", err, warn);
                    return Gltf::GLTF_LOAD_FAILURE;
                }
            }
            else
            {
                LOG_CORE_CRITICAL("Load errors: unrecognized extension {0}", extension);
                return Gltf::GLTF_LOAD_FAILURE;
            }
        }

        if (!m_GltfModel.meshes.size())
        {
            LOG_CORE_CRITICAL("Load: no meshes found in {0}", m_Filepath);
            return Gltf::GLTF_LOAD_FAILURE;
        }

        if (sceneID > Gltf::GLTF_NOT_USED) // a scene ID was provided
        {
            // check if valid
            if ((m_GltfModel.scenes.size() - 1) < static_cast<size_t>(sceneID))
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
        m_HasMesh.resize(m_GltfModel.nodes.size(), false);
        if (sceneID > Gltf::GLTF_NOT_USED) // a scene ID was provided
        {
            auto& scene = m_GltfModel.scenes[sceneID];
            size_t nodeCount = scene.nodes.size();
            for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
            {
                MarkNode(scene, scene.nodes[nodeIndex]);
            }
        }
        else // no scene ID was provided --> use all scenes
        {
            for (auto& scene : m_GltfModel.scenes)
            {
                size_t nodeCount = scene.nodes.size();
                for (size_t nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
                {
                    MarkNode(scene, scene.nodes[nodeIndex]);
                }
            }
        }

        // PASS 2 (for all instances)
        m_InstanceCount = instanceCount;
        for (m_InstanceIndex = 0; m_InstanceIndex < m_InstanceCount; ++m_InstanceIndex)
        {
            // create group game object(s) for all instances to apply transform from JSON file to
            auto entity = m_Registry.Create();

            auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::root";
            uint groupNode = m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);

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

    bool GltfBuilder::MarkNode(tinygltf::Scene& scene, int const gltfNodeIndex)
    {
        // each recursive call of this function marks a node in "m_HasMesh" if itself or a child has a mesh
        auto& node = m_GltfModel.nodes[gltfNodeIndex];

        // does this gltf node have a mesh?
        bool localHasMesh = (node.mesh != Gltf::GLTF_NOT_USED);

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

    void GltfBuilder::ProcessScene(tinygltf::Scene& scene, uint const parentNode)
    {
        size_t nodeCount = scene.nodes.size();
        if (!nodeCount)
        {
            LOG_CORE_WARN("Builder::ProcessScene: empty scene in {0}", m_Filepath);
            return;
        }

        m_RenderObject = 0;
        for (uint nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
        {
            ProcessNode(scene, scene.nodes[nodeIndex], parentNode);
        }
    }

    void GltfBuilder::ProcessNode(tinygltf::Scene& scene, int const gltfNodeIndex, uint const parentNode)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];
        auto& nodeName = node.name;
        auto meshIndex = node.mesh;

        uint currentNode = parentNode;

        if (m_HasMesh[gltfNodeIndex])
        {
            if (meshIndex > Gltf::GLTF_NOT_USED)
            {
                currentNode = CreateGameObject(scene, gltfNodeIndex, parentNode);
            }
            else // one or more children have a mesh, but not this one --> create group node
            {
                // create game object and transform component
                auto entity = m_Registry.Create();

                // create scene graph node and add to parent
                auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(m_InstanceIndex) +
                            "::" + scene.name + "::" + nodeName;
                currentNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);

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

    uint GltfBuilder::CreateGameObject(tinygltf::Scene& scene, int const gltfNodeIndex, uint const parentNode)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];
        auto& nodeName = node.name;
        uint meshIndex = node.mesh;

        auto entity = m_Registry.Create();
        auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::" + scene.name +
                    "::" + nodeName;
        uint newNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);

        TransformComponent transform{};
        LoadTransformationMatrix(transform, gltfNodeIndex);

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
            LoadVertexData(meshIndex);
            LOG_CORE_INFO("Vertex count: {0}, Index count: {1} (file: {2}, node: {3})", m_Vertices.size(), m_Indices.size(),
                          m_Filepath, nodeName);
            { // assign material
                uint primitiveIndex = 0;
                for (const auto& glTFPrimitive : m_GltfModel.meshes[meshIndex].primitives)
                {
                    Submesh& submesh = m_Submeshes[primitiveIndex];
                    ++primitiveIndex;
                    AssignMaterial(submesh, glTFPrimitive.material);
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
            m_Model = Engine::m_Engine->LoadModel(*this);
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

        { // add mesh and transform components to all instances
            MeshComponent mesh{nodeName, m_Model};
            m_Registry.emplace<MeshComponent>(entity, mesh);
            m_Registry.emplace<TransformComponent>(entity, transform);
        }

        return newNode;
    }

    int GltfBuilder::GetMinFilter(uint index)
    {
        int sampler = m_GltfModel.textures[index].sampler;
        int filter = m_GltfModel.samplers[sampler].minFilter;
        std::string& name = m_GltfModel.images[index].name;
        switch (filter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            {
                break;
            }
            case Gltf::GLTF_NOT_USED:
            {
                // use default filter
                filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                break;
            }
            default:
            {
                // use default filter
                filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                LOG_CORE_ERROR("minFilter: filter {0} not found, name = {1}", filter, name);
                break;
            }
        }
        return filter;
    }

    int GltfBuilder::GetMagFilter(uint index)
    {
        int sampler = m_GltfModel.textures[index].sampler;
        int filter = m_GltfModel.samplers[sampler].magFilter;
        std::string& name = m_GltfModel.images[index].name;
        switch (filter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            {
                break;
            }
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            {
                break;
            }
            case Gltf::GLTF_NOT_USED:
            {
                // use default filter
                filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                break;
            }
            default:
            {
                filter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
                LOG_CORE_ERROR("magFilter: filter {0} not found, name = {1}", filter, name);
                break;
            }
        }
        return filter;
    }

    void GltfBuilder::LoadTextures()
    {
        m_TextureOffset = m_Textures.size();
        size_t numTextures = m_GltfModel.images.size();
        m_Textures.resize(m_TextureOffset + numTextures);

        // retrieve all images from the glTF file
        for (uint imageIndex = 0; imageIndex < numTextures; ++imageIndex)
        {
            std::string imageFilepath = m_Basepath + m_GltfModel.images[imageIndex].uri;
            tinygltf::Image& glTFImage = m_GltfModel.images[imageIndex];

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
            int minFilter = GetMinFilter(imageIndex);
            int magFilter = GetMinFilter(imageIndex);
            bool imageFormat = GetImageFormat(imageIndex);
            texture->Init(glTFImage.width, glTFImage.height, imageFormat, buffer, minFilter, magFilter);
#ifdef DEBUG
            texture->SetFilename(imageFilepath);
#endif
            m_Textures[imageIndex] = texture;
        }
    }

    bool GltfBuilder::GetImageFormat(uint const imageIndex)
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

    void GltfBuilder::LoadMaterials()
    {
        size_t numMaterials = m_GltfModel.materials.size();
        m_Materials.resize(numMaterials);
        m_MaterialTextures.resize(numMaterials);

        uint materialIndex = 0;
        for (Material& material : m_Materials)
        {
            tinygltf::Material glTFMaterial = m_GltfModel.materials[materialIndex];
            Material::PbrMaterial& pbrMaterial = material.m_PbrMaterial;
            Material::MaterialTextures& materialTextures = m_MaterialTextures[materialIndex];

            // diffuse color aka base color factor
            // used as constant color, if no diffuse texture is provided
            // else, multiplied in the shader with each sample from the diffuse texture
            if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end())
            {
                pbrMaterial.m_DiffuseColor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
            }

            // diffuse map aka basecolor aka albedo
            if (glTFMaterial.pbrMetallicRoughness.baseColorTexture.index != Gltf::GLTF_NOT_USED)
            {
                int diffuseTextureIndex = glTFMaterial.pbrMetallicRoughness.baseColorTexture.index;
                tinygltf::Texture& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                materialTextures[Material::DIFFUSE_MAP_INDEX] = m_Textures[diffuseTexture.source];
                pbrMaterial.m_Features |= Material::HAS_DIFFUSE_MAP;
            }
            else if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end())
            {
                LOG_CORE_WARN("using legacy field values/baseColorTexture");
                int diffuseTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
                tinygltf::Texture& diffuseTexture = m_GltfModel.textures[diffuseTextureIndex];
                materialTextures[Material::DIFFUSE_MAP_INDEX] = m_Textures[diffuseTexture.source];
                pbrMaterial.m_Features |= Material::HAS_DIFFUSE_MAP;
            }

            // normal map
            if (glTFMaterial.normalTexture.index != Gltf::GLTF_NOT_USED)
            {
                int normalTextureIndex = glTFMaterial.normalTexture.index;
                tinygltf::Texture& normalTexture = m_GltfModel.textures[normalTextureIndex];
                materialTextures[Material::NORMAL_MAP_INDEX] = m_Textures[normalTexture.source];
                pbrMaterial.m_NormalMapIntensity = glTFMaterial.normalTexture.scale;
                pbrMaterial.m_Features |= Material::HAS_NORMAL_MAP;
            }

            // constant values for roughness and metallicness
            {
                pbrMaterial.m_Roughness = glTFMaterial.pbrMetallicRoughness.roughnessFactor;
                pbrMaterial.m_Metallic = glTFMaterial.pbrMetallicRoughness.metallicFactor;
            }

            // texture for roughness and metallicness
            if (glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index != Gltf::GLTF_NOT_USED)
            {
                int MetallicRoughnessTextureIndex = glTFMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
                tinygltf::Texture& metallicRoughnessTexture = m_GltfModel.textures[MetallicRoughnessTextureIndex];
                materialTextures[Material::ROUGHNESS_METALLIC_MAP_INDEX] = m_Textures[metallicRoughnessTexture.source];
                pbrMaterial.m_Features |= Material::HAS_ROUGHNESS_METALLIC_MAP;
            }

            // emissive color and emissive strength
            if (glTFMaterial.emissiveFactor.size() == 3)
            {
                pbrMaterial.m_EmissiveColor = glm::make_vec3(glTFMaterial.emissiveFactor.data());

                pbrMaterial.m_EmissiveStrength = 1.0f; // default is 1.0f
                auto it = glTFMaterial.extensions.find("KHR_materials_emissive_strength");
                if (it != glTFMaterial.extensions.end())
                {
                    auto extension = it->second;
                    if (extension.IsObject())
                    {
                        auto emissiveStrength = extension.Get("emissiveStrength");
                        if (emissiveStrength.IsReal())
                        {
                            pbrMaterial.m_EmissiveStrength = emissiveStrength.GetNumberAsDouble();
                        }
                    }
                }
            }

            // emissive texture
            if (glTFMaterial.emissiveTexture.index != Gltf::GLTF_NOT_USED)
            {
                int emissiveTextureIndex = glTFMaterial.emissiveTexture.index;
                tinygltf::Texture& emissiveTexture = m_GltfModel.textures[emissiveTextureIndex];
                materialTextures[Material::EMISSIVE_MAP_INDEX] = m_Textures[emissiveTexture.source];
                pbrMaterial.m_Features |= Material::HAS_EMISSIVE_MAP;
            }

            ++materialIndex;
        }
    }

    // load vertex data
    void GltfBuilder::LoadVertexData(uint const meshIndex)
    {
        m_Vertices.clear();
        m_Indices.clear();
        m_Submeshes.clear();

        uint numPrimitives = m_GltfModel.meshes[meshIndex].primitives.size();
        m_Submeshes.resize(numPrimitives);

        uint primitiveIndex = 0;
        for (const auto& glTFPrimitive : m_GltfModel.meshes[meshIndex].primitives)
        {
            Submesh& submesh = m_Submeshes[primitiveIndex];
            ++primitiveIndex;

            submesh.m_FirstVertex = static_cast<uint32_t>(m_Vertices.size());
            submesh.m_FirstIndex = static_cast<uint32_t>(m_Indices.size());
            submesh.m_InstanceCount = m_InstanceCount;

            uint vertexCount = 0;
            uint indexCount = 0;

            glm::vec4 diffuseColor(1.0f);
            if (glTFPrimitive.material != Gltf::GLTF_NOT_USED)
            {
                size_t materialIndex = glTFPrimitive.material;
                CORE_ASSERT(materialIndex < m_Materials.size(),
                            "LoadVertexData: glTFPrimitive.materialIndex must be less than m_Materials.size()");
                diffuseColor = m_Materials[materialIndex].m_PbrMaterial.m_DiffuseColor;
            }

            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* colorBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* tangentsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const uint* jointsBuffer = nullptr;
                const float* weightsBuffer = nullptr;

                int jointsBufferDataType = 0;

                // Get buffer data for vertex positions
                if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
                {
                    auto componentType =
                        LoadAccessor<float>(m_GltfModel.accessors[glTFPrimitive.attributes.find("POSITION")->second],
                                            positionBuffer, &vertexCount);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex color
                if (glTFPrimitive.attributes.find("COLOR_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("COLOR_0")->second], colorBuffer);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("NORMAL")->second], normalsBuffer);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex tangents
                if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("TANGENT")->second], tangentsBuffer);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second], texCoordsBuffer);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }

                // Get buffer data for joints
                if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end())
                {
                    jointsBufferDataType = LoadAccessor<uint>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("JOINTS_0")->second], jointsBuffer);
                    CORE_ASSERT((jointsBufferDataType == GL_BYTE) || (jointsBufferDataType == GL_UNSIGNED_BYTE),
                                "unexpected component type");
                }
                // Get buffer data for joint weights
                if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end())
                {
                    auto componentType = LoadAccessor<float>(
                        m_GltfModel.accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second], weightsBuffer);
                    CORE_ASSERT(componentType == GL_FLOAT, "unexpected component type");
                }

                // Append data to model's vertex buffer
                uint numVerticesBefore = m_Vertices.size();
                m_Vertices.resize(numVerticesBefore + vertexCount);
                uint vertexIndex = numVerticesBefore;
                for (size_t vertexIterator = 0; vertexIterator < vertexCount; ++vertexIterator)
                {
                    Vertex vertex{};
                    // position
                    auto position = positionBuffer ? glm::make_vec3(&positionBuffer[vertexIterator * 3]) : glm::vec3(0.0f);
                    vertex.m_Position = glm::vec3(position.x, position.y, position.z);

                    // normal
                    vertex.m_Normal = glm::normalize(
                        glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[vertexIterator * 3]) : glm::vec3(0.0f)));

                    // color
                    auto vertexColor = colorBuffer ? glm::make_vec3(&colorBuffer[vertexIterator * 3]) : glm::vec3(1.0f);
                    vertex.m_Color = glm::vec4(vertexColor.x, vertexColor.y, vertexColor.z, 1.0f) * diffuseColor;

                    // uv
                    vertex.m_UV = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[vertexIterator * 2]) : glm::vec3(0.0f);

                    // tangent
                    glm::vec4 t = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[vertexIterator * 4]) : glm::vec4(0.0f);
                    vertex.m_Tangent = glm::vec3(t.x, t.y, t.z) * t.w;

                    // joint indices and joint weights
                    if (jointsBuffer && weightsBuffer)
                    {
                        switch (jointsBufferDataType)
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
                    m_Vertices[vertexIndex] = vertex;
                    ++vertexIndex;
                }

                // calculate tangents
                if (!tangentsBuffer)
                {
                    CalculateTangents();
                }
            }
            // Indices
            {
                const uint32_t* buffer;
                uint count = 0;
                auto componentType = LoadAccessor<uint32_t>(m_GltfModel.accessors[glTFPrimitive.indices], buffer, &count);

                indexCount += count;

                // glTF supports different component types of indices
                switch (componentType)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* buf = buffer;
                        for (size_t index = 0; index < count; index++)
                        {
                            m_Indices.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(buffer);
                        for (size_t index = 0; index < count; index++)
                        {
                            m_Indices.push_back(buf[index]);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(buffer);
                        for (size_t index = 0; index < count; index++)
                        {
                            m_Indices.push_back(buf[index]);
                        }
                        break;
                    }
                    default:
                    {
                        CORE_ASSERT(false, "unexpected component type, index component type not supported!");
                        return;
                    }
                }
            }

            submesh.m_VertexCount = vertexCount;
            submesh.m_IndexCount = indexCount;
        }
    }

    void GltfBuilder::LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex)
    {
        auto& node = m_GltfModel.nodes[gltfNodeIndex];

        if (node.matrix.size() == 16)
        {
            transform.SetMat4Local(glm::make_mat4x4(node.matrix.data()));
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

    void GltfBuilder::AssignMaterial(Submesh& submesh, int const materialIndex)
    {
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
            std::shared_ptr<Buffer> instanceUbo{m_InstanceBuffer->GetBuffer()};
            resourceBuffers[Resources::INSTANCE_BUFFER_INDEX] = instanceUbo;
            if (m_SkeletalAnimation)
            {
                resourceBuffers[Resources::SKELETAL_ANIMATION_BUFFER_INDEX] = m_ShaderData;
            }
            submesh.m_Resources.m_ResourceDescriptor = ResourceDescriptor::Create(resourceBuffers);
        }

        LOG_CORE_INFO("material assigned (tinygltf): material index {0}", materialIndex);
    }

    void GltfBuilder::CalculateTangents()
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

    void GltfBuilder::SetDictionaryPrefix(std::string const& dictionaryPrefix) { m_DictionaryPrefix = dictionaryPrefix; }

    void GltfBuilder::CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices)
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
} // namespace GfxRenderEngine
