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
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "core.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/fbxBuilder.h"
#include "renderer/materialDescriptor.h"
#include "auxiliary/instrumentation.h"
#include "auxiliary/file.h"

#include "VKmodel.h"

namespace GfxRenderEngine
{

    FbxBuilder::FbxBuilder(const std::string& filepath, Scene& scene)
        : m_Filepath{filepath}, m_SkeletalAnimation{false}, m_Registry{scene.GetRegistry()},
          m_SceneGraph{scene.GetSceneGraph()}, m_Dictionary{scene.GetDictionary()}, m_InstanceCount{0}, m_InstanceIndex{0},
          m_FbxScene{nullptr}, m_FbxNoBuiltInTangents{false}
    {
        m_Basepath = EngineCore::GetPathWithoutFilename(filepath);
    }

    bool FbxBuilder::Load(uint const instanceCount, int const sceneID)
    {
        PROFILE_SCOPE("FbxBuilder::Load");
        Assimp::Importer importer;
        m_FbxScene =
            importer.ReadFile(m_Filepath, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals |
                                              aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

        if (m_FbxScene == nullptr)
        {
            LOG_CORE_CRITICAL("FbxBuilder::Load error: {0}", m_Filepath);
            return Fbx::FBX_LOAD_FAILURE;
        }

        if (!m_FbxScene->mNumMeshes)
        {
            LOG_CORE_CRITICAL("FbxBuilder::Load: no meshes found in {0}", m_Filepath);
            return Fbx::FBX_LOAD_FAILURE;
        }

        if (sceneID > Fbx::FBX_NOT_USED) // a scene ID was provided
        {
            LOG_CORE_WARN("FbxBuilder::Load: scene ID for fbx not supported (in file {0})", m_Filepath);
        }

        LoadSkeletonsFbx();
        LoadMaterials();

        // PASS 1
        // mark Fbx nodes to receive a game object ID if they have a mesh or any child has
        // --> create array of flags for all nodes of the Fbx file
        MarkNode(m_FbxScene->mRootNode);

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

            uint hasMeshIndex = Fbx::FBX_ROOT_NODE;
            m_RenderObject = 0;
            ProcessNode(m_FbxScene->mRootNode, groupNode, hasMeshIndex);
        }
        return Fbx::FBX_LOAD_SUCCESS;
    }

    bool FbxBuilder::MarkNode(const aiNode* fbxNodePtr)
    {
        // each recursive call of this function marks a node in "m_HasMesh" if itself or a child has a mesh

        // does this Fbx node have a mesh?
        bool localHasMesh = false;

        // check if at least one mesh is usable, i.e. is a triangle mesh
        for (uint nodeMeshIndex = 0; nodeMeshIndex < fbxNodePtr->mNumMeshes; ++nodeMeshIndex)
        {
            // retrieve index for global/scene mesh array
            uint sceneMeshIndex = fbxNodePtr->mMeshes[nodeMeshIndex];
            aiMesh* mesh = m_FbxScene->mMeshes[sceneMeshIndex];

            // check if triangle mesh
            if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
            {
                localHasMesh = true;
                break;
            }
        }

        int hasMeshIndex = m_HasMesh.size();
        m_HasMesh.push_back(localHasMesh); // reserve space in m_HasMesh, so that ProcessNode can find it

        // do any of the child nodes have a mesh?
        uint childNodeCount = fbxNodePtr->mNumChildren;
        for (uint childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            bool childHasMesh = MarkNode(fbxNodePtr->mChildren[childNodeIndex]);
            localHasMesh = localHasMesh || childHasMesh;
        }
        m_HasMesh[hasMeshIndex] = localHasMesh;
        return localHasMesh;
    }

    void FbxBuilder::ProcessNode(const aiNode* fbxNodePtr, uint const parentNode, uint& hasMeshIndex)
    {
        std::string nodeName = std::string(fbxNodePtr->mName.C_Str());
        uint currentNode = parentNode;

        if (m_HasMesh[hasMeshIndex])
        {
            if (fbxNodePtr->mNumMeshes)
            {
                currentNode = CreateGameObject(fbxNodePtr, parentNode);
            }
            else // one or more children have a mesh, but not this one --> create group node
            {
                // create game object and transform component
                auto entity = m_Registry.Create();
                {
                    TransformComponent transform(LoadTransformationMatrix(fbxNodePtr));
                    if (fbxNodePtr->mParent == m_FbxScene->mRootNode)
                    {
                        auto scale = transform.GetScale();
                        transform.SetScale({scale.x / 100.0f, scale.y / 100.0f, scale.z / 100.0f});
                        auto translation = transform.GetTranslation();
                        transform.SetTranslation({translation.x / 100.0f, translation.y / 100.0f, translation.z / 100.0f});
                    }
                    m_Registry.emplace<TransformComponent>(entity, transform);
                }

                // create scene graph node and add to parent
                auto name =
                    m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;
                currentNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);
            }
        }
        ++hasMeshIndex;

        uint childNodeCount = fbxNodePtr->mNumChildren;
        for (uint childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            ProcessNode(fbxNodePtr->mChildren[childNodeIndex], currentNode, hasMeshIndex);
        }
    }

    uint FbxBuilder::CreateGameObject(const aiNode* fbxNodePtr, uint const parentNode)
    {
        std::string nodeName = std::string(fbxNodePtr->mName.C_Str());

        auto entity = m_Registry.Create();
        auto name = m_DictionaryPrefix + "::" + m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;
        uint newNode = m_SceneGraph.CreateNode(parentNode, entity, name, m_Dictionary);

        TransformComponent transform(LoadTransformationMatrix(fbxNodePtr));
        if (fbxNodePtr->mParent == m_FbxScene->mRootNode)
        {
            auto scale = transform.GetScale();
            transform.SetScale({scale.x / 100.0f, scale.y / 100.0f, scale.z / 100.0f});
            auto translation = transform.GetTranslation();
            transform.SetTranslation({translation.x / 100.0f, translation.y / 100.0f, translation.z / 100.0f});
        }

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
            LoadVertexData(fbxNodePtr);
            LOG_CORE_INFO("Vertex count: {0}, Index count: {1} (file: {2}, node: {3})", m_Vertices.size(), m_Indices.size(),
                          m_Filepath, nodeName);
            for (uint submeshIndex = 0; submeshIndex < fbxNodePtr->mNumMeshes; ++submeshIndex)
            {
                uint fbxMeshIndex = fbxNodePtr->mMeshes[submeshIndex];
                AssignMaterial(m_Submeshes[submeshIndex], m_FbxScene->mMeshes[fbxMeshIndex]->mMaterialIndex);
            }
            m_Model = Engine::m_Engine->LoadModel(*this);

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

    std::shared_ptr<Texture> FbxBuilder::LoadTexture(std::string const& filepath, bool useSRGB)
    {
        std::shared_ptr<Texture> texture;
        bool loadSucess = false;

        if (EngineCore::FileExists(filepath) && !EngineCore::IsDirectory(filepath))
        {
            texture = Texture::Create();
            loadSucess = texture->Init(filepath, useSRGB);
        }
        else if (EngineCore::FileExists(m_Basepath + filepath) && !EngineCore::IsDirectory(m_Basepath + filepath))
        {
            texture = Texture::Create();
            loadSucess = texture->Init(m_Basepath + filepath, useSRGB);
        }
        else
        {
            LOG_CORE_CRITICAL("bool FbxBuilder::LoadTexture(): file '{0}' not found", filepath);
        }

        if (loadSucess)
        {
            m_Textures.push_back(texture);
            return texture;
        }
        return nullptr;
    }

    void FbxBuilder::LoadMap(const aiMaterial* fbxMaterial, aiTextureType textureType, int materialIndex)
    {
        uint textureCount = fbxMaterial->GetTextureCount(textureType);
        if (!textureCount)
        {
            return;
        }

        Material& material = m_Materials[materialIndex];
        Material::PbrMaterial& pbrMaterial = material.m_PbrMaterial;
        Material::MaterialTextures& materialTextures = m_MaterialTextures[materialIndex];

        aiString aiFilepath;
        auto getTexture = fbxMaterial->GetTexture(textureType, 0 /* first map*/, &aiFilepath);
        std::string fbxFilepath(aiFilepath.C_Str());
        std::string filepath(fbxFilepath);
        if (getTexture == aiReturn_SUCCESS)
        {
            switch (textureType)
            {
                // LoadTexture is inside switch statement for sRGB and UNORM
                case aiTextureType_DIFFUSE:
                {
                    auto texture = LoadTexture(filepath, Texture::USE_SRGB);
                    if (texture)
                    {
                        materialTextures[Material::DIFFUSE_MAP_INDEX] = texture;
                        pbrMaterial.m_Features |= Material::HAS_DIFFUSE_MAP;
                    }
                    break;
                }
                case aiTextureType_NORMALS:
                {
                    auto texture = LoadTexture(filepath, Texture::USE_UNORM);
                    if (texture)
                    {
                        materialTextures[Material::NORMAL_MAP_INDEX] = texture;
                        pbrMaterial.m_Features |= Material::HAS_NORMAL_MAP;
                    }
                    break;
                }
                case aiTextureType_SHININESS: // assimp XD
                {
                    auto texture = LoadTexture(filepath, Texture::USE_UNORM);
                    if (texture)
                    {
                        materialTextures[Material::ROUGHNESS_MAP_INDEX] = texture;
                        pbrMaterial.m_Features |= Material::HAS_ROUGHNESS_MAP;
                    }
                    break;
                }
                case aiTextureType_METALNESS:
                {
                    auto texture = LoadTexture(filepath, Texture::USE_UNORM);
                    if (texture)
                    {
                        materialTextures[Material::METALLIC_MAP_INDEX] = texture;
                        pbrMaterial.m_Features |= Material::HAS_METALLIC_MAP;
                    }
                    break;
                }
                case aiTextureType_EMISSIVE:
                {
                    auto texture = LoadTexture(filepath, Texture::USE_SRGB);
                    if (texture)
                    {
                        materialTextures[Material::EMISSIVE_MAP_INDEX] = texture;
                        pbrMaterial.m_Features |= Material::HAS_EMISSIVE_MAP;
                        pbrMaterial.m_EmissiveColor = glm::vec3(1.0f);
                    }
                    break;
                }
                default:
                {
                    CORE_ASSERT(false, "texture type not recognized");
                }
            }
        }
    }

    void FbxBuilder::LoadProperties(const aiMaterial* fbxMaterial, Material::PbrMaterial& pbrMaterial)
    {
        { // diffuse
            aiColor3D diffuseColor;
            if (fbxMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS)
            {
                pbrMaterial.m_DiffuseColor.r = diffuseColor.r;
                pbrMaterial.m_DiffuseColor.g = diffuseColor.g;
                pbrMaterial.m_DiffuseColor.b = diffuseColor.b;
            }
        }
        { // roughness
            float roughnessFactor;
            if (fbxMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == aiReturn_SUCCESS)
            {
                pbrMaterial.m_Roughness = roughnessFactor;
            }
            else
            {
                pbrMaterial.m_Roughness = 0.1f;
            }
        }

        { // metallic
            float metallicFactor;
            if (fbxMaterial->Get(AI_MATKEY_REFLECTIVITY, metallicFactor) == aiReturn_SUCCESS)
            {
                pbrMaterial.m_Metallic = metallicFactor;
            }
            else if (fbxMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
            {
                pbrMaterial.m_Metallic = metallicFactor;
            }
            else
            {
                pbrMaterial.m_Metallic = 0.886f;
            }
        }

        { // emissive color
            aiColor3D emission;
            auto result = fbxMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emission);
            if (result == aiReturn_SUCCESS)
            {
                pbrMaterial.m_EmissiveColor = glm::vec3(emission.r, emission.g, emission.b);
            }
        }

        { // emissive strength
            float emissiveStrength;
            auto result = fbxMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveStrength);
            if (result == aiReturn_SUCCESS)
            {
                pbrMaterial.m_EmissiveStrength = emissiveStrength;
            }
        }

        pbrMaterial.m_NormalMapIntensity = 1.0f;
    }

    void FbxBuilder::LoadMaterials()
    {
        uint numMaterials = m_FbxScene->mNumMaterials;
        m_Materials.resize(numMaterials);
        m_MaterialTextures.resize(numMaterials);
        for (uint materialIndex = 0; materialIndex < numMaterials; ++materialIndex)
        {
            const aiMaterial* fbxMaterial = m_FbxScene->mMaterials[materialIndex];
            // PrintMaps(fbxMaterial);

            Material& material = m_Materials[materialIndex];

            LoadProperties(fbxMaterial, material.m_PbrMaterial);

            LoadMap(fbxMaterial, aiTextureType_DIFFUSE, materialIndex);
            LoadMap(fbxMaterial, aiTextureType_NORMALS, materialIndex);
            LoadMap(fbxMaterial, aiTextureType_SHININESS, materialIndex);
            LoadMap(fbxMaterial, aiTextureType_METALNESS, materialIndex);
            LoadMap(fbxMaterial, aiTextureType_EMISSIVE, materialIndex);
        }
    }

    // load vertex data
    void FbxBuilder::LoadVertexData(const aiNode* fbxNodePtr, int vertexColorSet, uint uvSet)
    {
        m_Vertices.clear();
        m_Indices.clear();
        m_Submeshes.clear();

        m_FbxNoBuiltInTangents = false;
        uint numMeshes = fbxNodePtr->mNumMeshes;
        if (numMeshes)
        {
            m_Submeshes.resize(numMeshes);
            for (uint meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
            {
                LoadVertexData(fbxNodePtr, meshIndex, fbxNodePtr->mMeshes[meshIndex], vertexColorSet, uvSet);
            }
            if (m_FbxNoBuiltInTangents) // at least one mesh did not have tangents
            {
                LOG_CORE_CRITICAL("no tangents in fbx file found, calculating tangents manually");
                CalculateTangents();
            }
        }
    }

    void FbxBuilder::LoadVertexData(const aiNode* fbxNodePtr, uint const meshIndex, uint const fbxMeshIndex,
                                    int vertexColorSet, uint uvSet)
    {
        const aiMesh* mesh = m_FbxScene->mMeshes[fbxMeshIndex];

        // only triangle mesh supported
        if (!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
        {
            LOG_CORE_CRITICAL("FbxBuilder::LoadVertexData: only triangle meshes are supported");
            return;
        }

        const uint numVertices = mesh->mNumVertices;
        const uint numFaces = mesh->mNumFaces;
        const uint numIndices = numFaces * 3; // 3 indices per triangle a.k.a face

        size_t numVerticesBefore = m_Vertices.size();
        size_t numIndicesBefore = m_Indices.size();
        m_Vertices.resize(numVerticesBefore + numVertices);
        m_Indices.resize(numIndicesBefore + numIndices);

        Submesh& submesh = m_Submeshes[meshIndex];
        submesh.m_FirstVertex = numVerticesBefore;
        submesh.m_FirstIndex = numIndicesBefore;
        submesh.m_VertexCount = numVertices;
        submesh.m_IndexCount = numIndices;
        submesh.m_InstanceCount = m_InstanceCount;

        { // vertices
            bool hasPositions = mesh->HasPositions();
            bool hasNormals = mesh->HasNormals();
            bool hasTangents = mesh->HasTangentsAndBitangents();
            bool hasUVs = mesh->HasTextureCoords(uvSet);
            bool hasColors = mesh->HasVertexColors(vertexColorSet);

            CORE_ASSERT(hasPositions, "no postions found in " + m_Filepath);
            CORE_ASSERT(hasNormals, "no normals found in " + m_Filepath);

            m_FbxNoBuiltInTangents = m_FbxNoBuiltInTangents || (!hasTangents);

            uint vertexIndex = numVerticesBefore;
            for (uint fbxVertexIndex = 0; fbxVertexIndex < numVertices; ++fbxVertexIndex)
            {
                Vertex& vertex = m_Vertices[vertexIndex];

                if (hasPositions)
                { // position (guaranteed to always be there)
                    aiVector3D& positionFbx = mesh->mVertices[fbxVertexIndex];
                    vertex.m_Position = glm::vec3(positionFbx.x, positionFbx.y, positionFbx.z);
                }

                if (hasNormals) // normals
                {
                    aiVector3D& normalFbx = mesh->mNormals[fbxVertexIndex];
                    vertex.m_Normal = glm::vec3(normalFbx.x, normalFbx.y, normalFbx.z);
                }

                if (hasTangents) // tangents
                {
                    aiVector3D& tangentFbx = mesh->mTangents[fbxVertexIndex];
                    vertex.m_Tangent = glm::vec3(tangentFbx.x, tangentFbx.y, tangentFbx.z);
                }

                if (hasUVs) // uv coordinates
                {
                    aiVector3D& uvFbx = mesh->mTextureCoords[uvSet][fbxVertexIndex];
                    vertex.m_UV = glm::vec2(uvFbx.x, uvFbx.y);
                }

                // vertex colors
                {
                    glm::vec4 vertexColor;
                    uint materialIndex = mesh->mMaterialIndex;
                    if (hasColors)
                    {
                        aiColor4D& colorFbx = mesh->mColors[vertexColorSet][fbxVertexIndex];
                        glm::vec3 linearColor = glm::pow(glm::vec3(colorFbx.r, colorFbx.g, colorFbx.b), glm::vec3(2.2f));
                        vertexColor = glm::vec4(linearColor.r, linearColor.g, linearColor.b, colorFbx.a);
                        vertex.m_Color = vertexColor * m_Materials[materialIndex].m_PbrMaterial.m_DiffuseColor;
                    }
                    else
                    {
                        vertex.m_Color = m_Materials[materialIndex].m_PbrMaterial.m_DiffuseColor;
                    }
                }
                ++vertexIndex;
            }
        }

        // Indices
        {
            uint index = numIndicesBefore;
            for (uint faceIndex = 0; faceIndex < numFaces; ++faceIndex)
            {
                const aiFace& face = mesh->mFaces[faceIndex];
                m_Indices[index + 0] = face.mIndices[0];
                m_Indices[index + 1] = face.mIndices[1];
                m_Indices[index + 2] = face.mIndices[2];
                index += 3;
            }
        }

        // bone indices and bone weights
        {
            uint numberOfBones = mesh->mNumBones;
            std::vector<uint> numberOfBonesBoundtoVertex;
            numberOfBonesBoundtoVertex.resize(m_Vertices.size(), 0);
            for (uint boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
            {
                aiBone& bone = *mesh->mBones[boneIndex];
                uint numberOfWeights = bone.mNumWeights;

                // loop over vertices that are bound to that bone
                for (uint weightIndex = 0; weightIndex < numberOfWeights; ++weightIndex)
                {
                    uint vertexId = bone.mWeights[weightIndex].mVertexId;
                    CORE_ASSERT(vertexId < m_Vertices.size(), "memory violation");
                    float weight = bone.mWeights[weightIndex].mWeight;
                    switch (numberOfBonesBoundtoVertex[vertexId])
                    {
                        case 0:
                            m_Vertices[vertexId].m_JointIds.x = boneIndex;
                            m_Vertices[vertexId].m_Weights.x = weight;
                            break;
                        case 1:
                            m_Vertices[vertexId].m_JointIds.y = boneIndex;
                            m_Vertices[vertexId].m_Weights.y = weight;
                            break;
                        case 2:
                            m_Vertices[vertexId].m_JointIds.z = boneIndex;
                            m_Vertices[vertexId].m_Weights.z = weight;
                            break;
                        case 3:
                            m_Vertices[vertexId].m_JointIds.w = boneIndex;
                            m_Vertices[vertexId].m_Weights.w = weight;
                            break;
                        default:
                            break;
                    }
                    // track how many times this bone was hit
                    // (up to four bones can be bound to a vertex)
                    ++numberOfBonesBoundtoVertex[vertexId];
                }
            }
            // normalize weights
            for (uint vertexIndex = 0; vertexIndex < m_Vertices.size(); ++vertexIndex)
            {
                glm::vec4& boneWeights = m_Vertices[vertexIndex].m_Weights;
                float weightSum = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
                if (weightSum > std::numeric_limits<float>::epsilon())
                {
                    m_Vertices[vertexIndex].m_Weights = glm::vec4(boneWeights.x / weightSum, boneWeights.y / weightSum,
                                                                  boneWeights.z / weightSum, boneWeights.w / weightSum);
                }
            }
        }
    }

    glm::mat4 FbxBuilder::LoadTransformationMatrix(const aiNode* fbxNodePtr)
    {
        aiMatrix4x4 t = fbxNodePtr->mTransformation;
        glm::mat4 m;

        m[0][0] = (float)t.a1;
        m[0][1] = (float)t.b1;
        m[0][2] = (float)t.c1;
        m[0][3] = (float)t.d1;
        m[1][0] = (float)t.a2;
        m[1][1] = (float)t.b2;
        m[1][2] = (float)t.c2;
        m[1][3] = (float)t.d2;
        m[2][0] = (float)t.a3;
        m[2][1] = (float)t.b3;
        m[2][2] = (float)t.c3;
        m[2][3] = (float)t.d3;
        m[3][0] = (float)t.a4;
        m[3][1] = (float)t.b4;
        m[3][2] = (float)t.c4;
        m[3][3] = (float)t.d4;

        return m;
    }

    void FbxBuilder::AssignMaterial(Submesh& submesh, int const materialIndex)
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

        LOG_CORE_INFO("material assigned (fastgltf): material index {0}", materialIndex);
    }

    void FbxBuilder::CalculateTangents()
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

    void FbxBuilder::CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices)
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

    void FbxBuilder::SetDictionaryPrefix(std::string const& dictionaryPrefix) { m_DictionaryPrefix = dictionaryPrefix; }

    void FbxBuilder::PrintMaps(const aiMaterial* fbxMaterial)
    {
        auto materialName = fbxMaterial->GetName();
        LOG_CORE_ERROR("material name: {0}", std::string(materialName.C_Str()));
        LOG_CORE_CRITICAL("aiTextureType_NONE                  = {0}", fbxMaterial->GetTextureCount(aiTextureType_NONE));
        LOG_CORE_CRITICAL("aiTextureType_DIFFUSE               = {0}", fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE));
        LOG_CORE_CRITICAL("aiTextureType_SPECULAR              = {0}", fbxMaterial->GetTextureCount(aiTextureType_SPECULAR));
        LOG_CORE_CRITICAL("aiTextureType_AMBIENT               = {0}", fbxMaterial->GetTextureCount(aiTextureType_AMBIENT));
        LOG_CORE_CRITICAL("aiTextureType_EMISSIVE              = {0}", fbxMaterial->GetTextureCount(aiTextureType_EMISSIVE));
        LOG_CORE_CRITICAL("aiTextureType_HEIGHT                = {0}", fbxMaterial->GetTextureCount(aiTextureType_HEIGHT));
        LOG_CORE_CRITICAL("aiTextureType_NORMALS               = {0}", fbxMaterial->GetTextureCount(aiTextureType_NORMALS));
        LOG_CORE_CRITICAL("aiTextureType_SHININESS             = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_SHININESS));
        LOG_CORE_CRITICAL("aiTextureType_OPACITY               = {0}", fbxMaterial->GetTextureCount(aiTextureType_OPACITY));
        LOG_CORE_CRITICAL("aiTextureType_DISPLACEMENT          = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_DISPLACEMENT));
        LOG_CORE_CRITICAL("aiTextureType_LIGHTMAP              = {0}", fbxMaterial->GetTextureCount(aiTextureType_LIGHTMAP));
        LOG_CORE_CRITICAL("aiTextureType_REFLECTION            = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_REFLECTION));
        LOG_CORE_CRITICAL("aiTextureType_BASE_COLOR            = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_BASE_COLOR));
        LOG_CORE_CRITICAL("aiTextureType_NORMAL_CAMERA         = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_NORMAL_CAMERA));
        LOG_CORE_CRITICAL("aiTextureType_EMISSION_COLOR        = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_EMISSION_COLOR));
        LOG_CORE_CRITICAL("aiTextureType_METALNESS             = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_METALNESS));
        LOG_CORE_CRITICAL("aiTextureType_DIFFUSE_ROUGHNESS     = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS));
        LOG_CORE_CRITICAL("aiTextureType_AMBIENT_OCCLUSION     = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION));
        LOG_CORE_CRITICAL("aiTextureType_SHEEN                 = {0}", fbxMaterial->GetTextureCount(aiTextureType_SHEEN));
        LOG_CORE_CRITICAL("aiTextureType_CLEARCOAT             = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_CLEARCOAT));
        LOG_CORE_CRITICAL("aiTextureType_TRANSMISSION          = {0}",
                          fbxMaterial->GetTextureCount(aiTextureType_TRANSMISSION));
        LOG_CORE_CRITICAL("aiTextureType_UNKNOWN               = {0}", fbxMaterial->GetTextureCount(aiTextureType_UNKNOWN));

        float factor;
        LOG_CORE_CRITICAL("AI_MATKEY_BASE_COLOR                = {0}",
                          fbxMaterial->Get(AI_MATKEY_BASE_COLOR, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_ROUGHNESS_FACTOR          = {0}",
                          fbxMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_METALLIC_FACTOR           = {0}",
                          fbxMaterial->Get(AI_MATKEY_METALLIC_FACTOR, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_COLOR_DIFFUSE             = {0}",
                          fbxMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_COLOR_EMISSIVE            = {0}",
                          fbxMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_USE_EMISSIVE_MAP          = {0}",
                          fbxMaterial->Get(AI_MATKEY_USE_EMISSIVE_MAP, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_EMISSIVE_INTENSITY        = {0}",
                          fbxMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_COLOR_SPECULAR            = {0}",
                          fbxMaterial->Get(AI_MATKEY_COLOR_SPECULAR, factor) == aiReturn_SUCCESS);
        LOG_CORE_CRITICAL("AI_MATKEY_REFLECTIVITY              = {0}",
                          fbxMaterial->Get(AI_MATKEY_REFLECTIVITY, factor) == aiReturn_SUCCESS);

        uint numProperties = fbxMaterial->mNumProperties;
        for (uint propertyIndex = 0; propertyIndex < numProperties; ++propertyIndex)
        {
            aiMaterialProperty* materialProperty = fbxMaterial->mProperties[propertyIndex];
            std::string key = std::string(materialProperty->mKey.C_Str());
            LOG_CORE_CRITICAL("key: {0}", key);
        }
    }
} // namespace GfxRenderEngine
