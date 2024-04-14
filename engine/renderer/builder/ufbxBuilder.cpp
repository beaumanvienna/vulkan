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
#include "renderer/builder/ufbxBuilder.h"
#include "renderer/materialDescriptor.h"
#include "auxiliary/file.h"

#include "VKmodel.h"

namespace GfxRenderEngine
{

    UFbxBuilder::UFbxBuilder(const std::string& filepath, Scene& scene)
        : m_Filepath{filepath}, m_SkeletalAnimation{0}, m_Registry{scene.GetRegistry()}, m_SceneGraph{scene.GetSceneGraph()},
          m_Dictionary{scene.GetDictionary()}, m_InstanceCount{0}, m_InstanceIndex{0}, m_FbxScene{nullptr},
          m_FbxNoBuiltInTangents{false}, m_MaterialFeatures{0}
    {
        m_Basepath = EngineCore::GetPathWithoutFilename(filepath);
    }

    bool UFbxBuilder::Load(uint const instanceCount, int const sceneID)
    {
        ufbx_load_opts opts{};
        opts.load_external_files = true;
        opts.ignore_missing_external_files = true;
        opts.generate_missing_normals = true;
        opts.target_axes = {
            .right = UFBX_COORDINATE_AXIS_POSITIVE_X,
            .up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
            .front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
        };
        opts.target_unit_meters = 1.0f;

        ufbx_error error;
        m_FbxScene = ufbx_load_file(m_Filepath.c_str(), &opts, &error);

        if (m_FbxScene == nullptr)
        {
            char buffer[1024];
            ufbx_format_error(buffer, sizeof(buffer), &error);
            LOG_CORE_CRITICAL("UFbxBuilder::Load error: file: {0}, error: {1}", m_Filepath, buffer);
            return Fbx::FBX_LOAD_FAILURE;
        }

        if (!m_FbxScene->meshes.count)
        {
            LOG_CORE_CRITICAL("UFbxBuilder::Load: no meshes found in {0}", m_Filepath);
            return Fbx::FBX_LOAD_FAILURE;
        }

        if (sceneID > Fbx::FBX_NOT_USED) // a scene ID was provided
        {
            LOG_CORE_WARN("UFbxBuilder::Load: scene ID for fbx not supported (in file {0})", m_Filepath);
        }

        LoadSkeletonsFbx();
        LoadMaterials();

        // PASS 1
        // mark Fbx nodes to receive a game object ID if they have a mesh or any child has
        //--> create array of flags for all nodes of the Fbx file
        MarkNode(m_FbxScene->root_node);

        // PASS 2 (for all instances)
        m_InstanceCount = instanceCount;
        for (m_InstanceIndex = 0; m_InstanceIndex < m_InstanceCount; ++m_InstanceIndex)
        {
            uint hasMeshIndex = Fbx::FBX_ROOT_NODE;
            m_RenderObject = 0;
            ProcessNode(m_FbxScene->root_node, SceneGraph::ROOT_NODE, hasMeshIndex);
        }
        ufbx_free_scene(m_FbxScene);
        return Fbx::FBX_LOAD_SUCCESS;
    }

    bool UFbxBuilder::MarkNode(const ufbx_node* fbxNodePtr)
    {
        // each recursive call of this function marks a node in "m_HasMesh" if itself or a child has a mesh

        // does this Fbx node have a mesh?
        bool localHasMesh = false;

        // check if triangle mesh

        if (fbxNodePtr->mesh && fbxNodePtr->mesh->num_triangles)
        {
            localHasMesh = true;
        }

        int hasMeshIndex = m_HasMesh.size();
        m_HasMesh.push_back(localHasMesh); // reserve space in m_HasMesh, so that ProcessNode can find it

        // do any of the child nodes have a mesh?
        uint childNodeCount = fbxNodePtr->children.count;
        for (uint childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            bool childHasMesh = MarkNode(fbxNodePtr->children[childNodeIndex]);
            localHasMesh = localHasMesh || childHasMesh;
        }
        m_HasMesh[hasMeshIndex] = localHasMesh;
        return localHasMesh;
    }

    void UFbxBuilder::ProcessNode(const ufbx_node* fbxNodePtr, uint parentNode, uint& hasMeshIndex)
    {
        std::string nodeName;
        if (fbxNodePtr == m_FbxScene->root_node)
        {
            nodeName = fbxNodePtr->name.length ? fbxNodePtr->name.data : "root";
        }
        else
        {
            nodeName = fbxNodePtr->name.length ? fbxNodePtr->name.data : "group node";
        }

        uint currentNode = parentNode;

        if (m_HasMesh[hasMeshIndex])
        {
            if (fbxNodePtr->mesh && fbxNodePtr->mesh->num_triangles)
            {
                currentNode = CreateGameObject(fbxNodePtr, parentNode);
            }
            else // one or more children have a mesh, but not this one --> create group node
            {
                // create game object and transform component
                auto entity = m_Registry.create();
                {
                    glm::vec3 scale;
                    glm::quat rotation;
                    glm::vec3 translation;
                    LoadTransformationMatrix(fbxNodePtr, scale, rotation, translation);
                    TransformComponent transform(scale, rotation, translation);
                    if (fbxNodePtr->parent == m_FbxScene->root_node)
                    {
                        // map fbx to gltf
                        transform.SetScale({scale.x / 100.0f, scale.y / 100.0f, scale.z / 100.0f});
                        transform.SetTranslation({translation.x / 100.0f, translation.y / 100.0f, translation.z / 100.0f});
                    }
                    m_Registry.emplace<TransformComponent>(entity, transform);
                }

                // create scene graph node and add to parent
                std::string shortName;
                std::string longName;
                if (fbxNodePtr == m_FbxScene->root_node)
                { // special name in scene graph for root node
                    std::string name = EngineCore::GetFilenameWithoutPathAndExtension(m_Filepath);
                    shortName = name + "::" + std::to_string(m_InstanceIndex) + "::root";
                    longName = m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::root";
                }
                else
                {
                    shortName = "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;
                    longName = m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;
                }
                currentNode = m_SceneGraph.CreateNode(entity, shortName, longName, m_Dictionary);
                m_SceneGraph.GetNode(parentNode).AddChild(currentNode);
            }
        }
        ++hasMeshIndex;

        uint childNodeCount = fbxNodePtr->children.count;
        for (uint childNodeIndex = 0; childNodeIndex < childNodeCount; ++childNodeIndex)
        {
            ProcessNode(fbxNodePtr->children[childNodeIndex], currentNode, hasMeshIndex);
        }
    }

    uint UFbxBuilder::CreateGameObject(const ufbx_node* fbxNodePtr, uint const parentNode)
    {
        std::string nodeName(fbxNodePtr->name.data);

        auto entity = m_Registry.create();
        auto shortName = EngineCore::GetFilenameWithoutPathAndExtension(m_Filepath) +
                         "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;
        auto longName = m_Filepath + "::" + std::to_string(m_InstanceIndex) + "::" + nodeName;

        uint newNode = m_SceneGraph.CreateNode(entity, shortName, longName, m_Dictionary);
        m_SceneGraph.GetNode(parentNode).AddChild(newNode);

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        LoadTransformationMatrix(fbxNodePtr, scale, rotation, translation);
        TransformComponent transform(scale, rotation, translation);
        if (fbxNodePtr->parent == m_FbxScene->root_node)
        {
            // map fbx to gltf
            transform.SetScale({scale.x / 100.0f, scale.y / 100.0f, scale.z / 100.0f});
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
            for (uint meshIndex = 0; meshIndex < fbxNodePtr->mesh->material_parts.count; ++meshIndex)
            {
                std::string materialName = fbxNodePtr->mesh->materials.data[meshIndex]->name.data;
                uint fbxMeshIndex = m_MaterialNameToIndex[materialName];
                AssignMaterial(m_Submeshes[meshIndex], fbxMeshIndex);
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
            if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2Map)
            {
                PbrDiffuseNormalRoughnessMetallic2Tag pbrDiffuseNormalRoughnessMetallic2Tag;
                m_Registry.emplace<PbrDiffuseNormalRoughnessMetallic2Tag>(entity, pbrDiffuseNormalRoughnessMetallic2Tag);
            }
            if (m_MaterialFeatures & MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSA2Map)
            {
                PbrDiffuseNormalRoughnessMetallicSA2Tag pbrDiffuseNormalRoughnessMetallicSA2Tag;
                m_Registry.emplace<PbrDiffuseNormalRoughnessMetallicSA2Tag>(entity, pbrDiffuseNormalRoughnessMetallicSA2Tag);

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

        { // add mesh and transform components to all instances
            MeshComponent mesh{nodeName, m_Model};
            m_Registry.emplace<MeshComponent>(entity, mesh);
            m_Registry.emplace<TransformComponent>(entity, transform);
        }

        return newNode;
    }

    void UFbxBuilder::LoadImage(std::string& filepath, uint& mapIndex, bool useSRGB)
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
            LOG_CORE_CRITICAL("UFbxBuilder::LoadImage(): file '{0}' not found", filepath);
        }

        if (loadSucess)
        {
            mapIndex = m_Images.size();
            m_Images.push_back(texture);
        }
    }

    void UFbxBuilder::LoadMaterial(const ufbx_material* fbxMaterial, ufbx_material_pbr_map materialProperty,
                                   Material& engineMaterial)
    {
        switch (materialProperty)
        {
            case UFBX_MATERIAL_PBR_BASE_COLOR: // aka albedo aka diffuse color
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.base_color;
                if (materialMap.has_value)
                {
                    if (materialMap.texture)
                    {
                        std::string filename(materialMap.texture->filename.data);
                        LoadImage(filename, engineMaterial.m_DiffuseMapIndex, Texture::USE_SRGB);
                        engineMaterial.m_Features |= Material::HAS_DIFFUSE_MAP;
                    }
                    else // constant material property
                    {
                        const ufbx_material_map& baseFactorMaterialMap = fbxMaterial->pbr.base_factor;
                        float baseFactor = baseFactorMaterialMap.has_value ? baseFactorMaterialMap.value_real : 1.0f;
                        engineMaterial.m_DiffuseColor.r = materialMap.value_vec4.x * baseFactor;
                        engineMaterial.m_DiffuseColor.g = materialMap.value_vec4.y * baseFactor;
                        engineMaterial.m_DiffuseColor.b = materialMap.value_vec4.z * baseFactor;
                        engineMaterial.m_DiffuseColor.a = materialMap.value_vec4.w;
                    }
                }
                break;
            }
            case UFBX_MATERIAL_PBR_ROUGHNESS:
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.roughness;
                if (materialMap.has_value)
                {
                    if (materialMap.texture)
                    {
                        std::string filename(materialMap.texture->filename.data);
                        LoadImage(filename, engineMaterial.m_RoughnessMapIndex, Texture::USE_UNORM);
                        engineMaterial.m_Features |= Material::HAS_ROUGHNESS_MAP;
                    }
                    else // constant material property
                    {
                        engineMaterial.m_Roughness = materialMap.value_real;
                    }
                }
                break;
            }
            case UFBX_MATERIAL_PBR_METALNESS:
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.metalness;
                if (materialMap.has_value)
                {
                    if (materialMap.texture)
                    {
                        std::string filename(materialMap.texture->filename.data);
                        LoadImage(filename, engineMaterial.m_MetallicMapIndex, Texture::USE_UNORM);
                        engineMaterial.m_Features |= Material::HAS_METALLIC_MAP;
                    }
                    else // constant material property
                    {
                        engineMaterial.m_Metallic = materialMap.value_real;
                    }
                }
                break;
            }
            case UFBX_MATERIAL_PBR_NORMAL_MAP:
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.normal_map;
                if (materialMap.texture)
                {
                    std::string filename(materialMap.texture->filename.data);
                    LoadImage(filename, engineMaterial.m_NormalMapIndex, Texture::USE_UNORM);
                    engineMaterial.m_Features |= Material::HAS_NORMAL_MAP;
                }
                break;
            }
            case UFBX_MATERIAL_PBR_EMISSION_COLOR:
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.emission_color;
                if (materialMap.texture)
                {
                    std::string filename(materialMap.texture->filename.data);
                    LoadImage(filename, engineMaterial.m_EmissiveMapIndex, Texture::USE_SRGB);
                    engineMaterial.m_Features |= Material::HAS_EMISSIVE_MAP;
                }

                {
                    glm::vec3 emissiveColor(materialMap.value_vec3.x, materialMap.value_vec3.y, materialMap.value_vec3.z);
                    engineMaterial.m_EmissiveColor = emissiveColor;
                }
                break;
            }
            case UFBX_MATERIAL_PBR_EMISSION_FACTOR:
            {
                ufbx_material_map const& materialMap = fbxMaterial->pbr.emission_factor;
                if (materialMap.has_value)
                {
                    engineMaterial.m_EmissiveStrength = materialMap.value_real;
                }
                break;
            }
            default:
            {
                CORE_ASSERT(false, "material property not recognized");
                break;
            }
        }
    }

    void UFbxBuilder::LoadMaterials()
    {
        m_Materials.clear();
        uint numMaterials = m_FbxScene->materials.count;
        for (uint fbxMaterialIndex = 0; fbxMaterialIndex < numMaterials; ++fbxMaterialIndex)
        {
            const ufbx_material* fbxMaterial = m_FbxScene->materials[fbxMaterialIndex];
            // PrintProperties(fbxMaterial);

            Material engineMaterial{};
            engineMaterial.m_Features = m_SkeletalAnimation;

            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_BASE_COLOR, engineMaterial);
            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_ROUGHNESS, engineMaterial);
            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_METALNESS, engineMaterial);
            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_NORMAL_MAP, engineMaterial);
            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_EMISSION_COLOR, engineMaterial);
            LoadMaterial(fbxMaterial, UFBX_MATERIAL_PBR_EMISSION_FACTOR, engineMaterial);

            m_MaterialNameToIndex[fbxMaterial->name.data] = fbxMaterialIndex;
            m_Materials.push_back(engineMaterial);
        }
    }

    void UFbxBuilder::LoadVertexData(const ufbx_node* fbxNodePtr)
    {
        // handle vertex data
        m_Vertices.clear();
        m_Indices.clear();
        m_Submeshes.clear();
        m_MaterialFeatures = 0;
        m_FbxNoBuiltInTangents = false;

        ufbx_mesh& fbxMesh = *fbxNodePtr->mesh; // mesh for this node, contains submeshes
        uint numSubmeshes = fbxMesh.material_parts.count;
        if (numSubmeshes)
        {
            m_Submeshes.resize(numSubmeshes);
            for (uint submeshIndex = 0; submeshIndex < numSubmeshes; ++submeshIndex)
            {
                LoadVertexData(fbxNodePtr, submeshIndex);
            }
            if (m_FbxNoBuiltInTangents) // at least one mesh did not have tangents
            {
                CalculateTangents();
            }
        }
    }

    void UFbxBuilder::LoadVertexData(const ufbx_node* fbxNodePtr, uint const submeshIndex)
    {
        ufbx_mesh& fbxMesh = *fbxNodePtr->mesh; // mesh for this node, contains submeshes
        const ufbx_mesh_part& fbxSubmesh = fbxNodePtr->mesh->material_parts[submeshIndex];
        size_t numFaces = fbxSubmesh.num_faces;

        if (!(fbxSubmesh.num_triangles))
        {
            LOG_CORE_CRITICAL("UFbxBuilder::LoadVertexData: only triangle meshes are supported");
            return;
        }

        size_t numVerticesBefore = m_Vertices.size();
        size_t numIndicesBefore = m_Indices.size();

        ModelSubmesh& submesh = m_Submeshes[submeshIndex];
        submesh.m_FirstVertex = numVerticesBefore;
        submesh.m_FirstIndex = numIndicesBefore;
        // submesh.m_VertexCount = 0;
        submesh.m_IndexCount = 0;
        submesh.m_InstanceCount = m_InstanceCount;

        glm::vec4 diffuseColor;
        {
            ufbx_material_map& baseColorMap = fbxNodePtr->materials[submeshIndex]->pbr.base_color;
            diffuseColor = baseColorMap.has_value ? glm::vec4(baseColorMap.value_vec4.x, baseColorMap.value_vec4.y,
                                                              baseColorMap.value_vec4.z, baseColorMap.value_vec4.w)
                                                  : glm::vec4(1.0f);
        }

        { // vertices
            bool hasTangents = fbxMesh.vertex_tangent.exists;
            bool hasUVs = fbxMesh.uv_sets.count;
            bool hasVertexColors = fbxMesh.vertex_color.exists;

            m_FbxNoBuiltInTangents = m_FbxNoBuiltInTangents || (!hasTangents);
            for (size_t fbxFaceIndex = 0; fbxFaceIndex < numFaces; ++fbxFaceIndex)
            {
                ufbx_face& fbxFace = fbxMesh.faces[fbxSubmesh.face_indices.data[fbxFaceIndex]];
                size_t numTriangleIndices = fbxMesh.max_face_triangles * 3;
                std::vector<uint> verticesPerFaceIndexBuffer(numTriangleIndices);
                size_t numTriangles =
                    ufbx_triangulate_face(verticesPerFaceIndexBuffer.data(), numTriangleIndices, &fbxMesh, fbxFace);
                size_t numVerticesPerFace = 3 * numTriangles;
                for (uint vertexPerFace = 0; vertexPerFace < numVerticesPerFace; ++vertexPerFace)
                {
                    // if the face is a quad, then 2 triangles, numVerticesPerFace = 6
                    uint vertexPerFaceIndex = verticesPerFaceIndexBuffer[vertexPerFace];

                    Vertex vertex{};
                    vertex.m_Amplification = 1.0f;

                    // position
                    uint fbxVertexIndex = fbxMesh.vertex_indices[vertexPerFaceIndex];
                    {
                        ufbx_vec3& positionFbx = fbxMesh.vertices[fbxVertexIndex];
                        vertex.m_Position = glm::vec3(positionFbx.x, positionFbx.y, positionFbx.z);
                    }

                    // normals, always defined if `ufbx_load_opts.generate_missing_normals` is used
                    {
                        uint fbxNormalIndex = fbxMesh.vertex_normal.indices[vertexPerFaceIndex];
                        CORE_ASSERT(fbxNormalIndex < fbxMesh.vertex_normal.values.count,
                                    "LoadVertexData: memory violation normals");
                        ufbx_vec3& normalFbx = fbxMesh.vertex_normal.values.data[fbxNormalIndex];
                        vertex.m_Normal = glm::vec3(normalFbx.x, normalFbx.y, normalFbx.z);
                    }
                    if (hasTangents) // tangents (check `tangent space` in Blender when exporting fbx)
                    {
                        uint fbxTangentIndex = fbxMesh.vertex_tangent.indices[vertexPerFaceIndex];
                        CORE_ASSERT(fbxTangentIndex < fbxMesh.vertex_tangent.values.count,
                                    "LoadVertexData: memory violation tangents");
                        ufbx_vec3& tangentFbx = fbxMesh.vertex_tangent.values.data[fbxTangentIndex];
                        vertex.m_Tangent = glm::vec3(tangentFbx.x, tangentFbx.y, tangentFbx.z);
                    }

                    if (hasUVs) // uv coordinates
                    {
                        uint fbxUVIndex = fbxMesh.vertex_uv.indices[vertexPerFaceIndex];
                        CORE_ASSERT(fbxUVIndex < fbxMesh.vertex_uv.values.count,
                                    "LoadVertexData: memory violation uv coordinates");
                        ufbx_vec2& uvFbx = fbxMesh.vertex_uv.values.data[fbxUVIndex];
                        vertex.m_UV = glm::vec2(uvFbx.x, uvFbx.y);
                    }

                    if (hasVertexColors) // vertex colors
                    {
                        uint fbxColorIndex = fbxMesh.vertex_color.indices[vertexPerFaceIndex];
                        ufbx_vec4& colorFbx = fbxMesh.vertex_color.values.data[fbxColorIndex];

                        // convert from sRGB to linear
                        glm::vec3 linearColor = glm::pow(glm::vec3(colorFbx.x, colorFbx.y, colorFbx.z), glm::vec3(2.2f));
                        glm::vec4 vertexColor(linearColor.x, linearColor.y, linearColor.z, colorFbx.w);
                        vertex.m_Color = vertexColor * diffuseColor;
                    }
                    else
                    {
                        vertex.m_Color = diffuseColor;
                    }
                    m_Vertices.push_back(vertex);
                }
            }
        }

        // resolve indices
        // A face has four vertices, while above loop generates at least six vertices for per face)
        {
            // get number of all vertices created from above (faces * trianglesPerFace * 3)
            uint submeshAllVertices = m_Vertices.size() - numVerticesBefore;

            // create a ufbx vertex stream with data pointing to the first vertex of this submesh
            // (m_vertices is for all submeshes)
            ufbx_vertex_stream streams;
            streams.data = &m_Vertices[numVerticesBefore];
            streams.vertex_count = submeshAllVertices;
            streams.vertex_size = sizeof(Vertex);

            // index buffer: add space for all new vertices from above
            m_Indices.resize(numIndicesBefore + submeshAllVertices);

            // ufbx_generate_indices() will rearrange m_Vertices (via streams.data) and fill m_Indices
            ufbx_error ufbxError;
            size_t numVertices = ufbx_generate_indices(&streams, 1 /*size_t num_streams*/, &m_Indices[numIndicesBefore],
                                                       submeshAllVertices, nullptr, &ufbxError);

            // handle error
            if (ufbxError.type != UFBX_ERROR_NONE)
            {
                char buffer[1024];
                ufbx_format_error(buffer, sizeof(buffer), &ufbxError);
                LOG_CORE_CRITICAL("UFbxBuilder: creation of index buffer failed, file: {0}, error: {1},  node: {2}",
                                  m_Filepath, buffer, fbxNodePtr->name.data);
            }

            // m_Vertices can be downsized now
            m_Vertices.resize(numVerticesBefore + numVertices);
            submesh.m_VertexCount = numVertices;
            submesh.m_IndexCount = submeshAllVertices;
        }
    }

    void UFbxBuilder::LoadTransformationMatrix(const ufbx_node* fbxNodePtr, glm::vec3& scale, glm::quat& rotation,
                                               glm::vec3& translation)
    {
        ufbx_transform t = fbxNodePtr->local_transform;
        translation.x = t.translation.x;
        translation.y = t.translation.y;
        translation.z = t.translation.z;
        rotation.x = t.rotation.x;
        rotation.y = t.rotation.y;
        rotation.z = t.rotation.z;
        rotation.w = t.rotation.w;
        scale.x = t.scale.x;
        scale.y = t.scale.y;
        scale.z = t.scale.z;
    }

    void UFbxBuilder::AssignMaterial(ModelSubmesh& submesh, uint const materialIndex)
    {
        if (!m_FbxScene->materials.count)
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMapInstanced, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrNoMap;
            }
            submesh.m_MaterialProperties.m_Roughness = 0.5f;
            submesh.m_MaterialProperties.m_Metallic = 0.1f;

            LOG_CORE_INFO("material assigned: material index {0}, PbrNoMap (no material found)", materialIndex);
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
        submesh.m_MaterialProperties.m_EmissiveColor = glm::vec4(material.m_EmissiveColor, 1.0f);

        uint pbrFeatures = material.m_Features & (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP |
                                                  Material::HAS_ROUGHNESS_MAP | Material::HAS_METALLIC_MAP |
                                                  Material::HAS_ROUGHNESS_METALLIC_MAP | Material::HAS_SKELETAL_ANIMATION);
        if (pbrFeatures == Material::HAS_DIFFUSE_MAP)
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuse, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseSAMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseSAMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: diffuseSAMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseSAMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseSAMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseSAMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseSA, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial:normalMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseNormalMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormal, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size()");
            CORE_ASSERT(normalMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial:normalMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseNormalSAMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalSAMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalSA, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_MAP |
                                 Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMapIndex = material.m_RoughnessMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size() ");
            CORE_ASSERT(normalMapIndex < m_Images.size(), " UFbxBuilder::AssignMaterial: normalMapIndex <m_Images.size() ");
            CORE_ASSERT(roughnessMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: roughnessMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessSAMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessSAMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalSA, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_MAP |
                                 Material::HAS_METALLIC_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMapIndex = material.m_RoughnessMapIndex;
            uint metallicMapIndex = material.m_MetallicMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial: diffuseMapIndex <m_Images.size() ");
            CORE_ASSERT(normalMapIndex < m_Images.size(), " UFbxBuilder::AssignMaterial: normalMapIndex <m_Images.size() ");
            CORE_ASSERT(roughnessMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: roughnessMapIndex < m_Images.size()");
            CORE_ASSERT(metallicMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: metallicMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMapIndex], m_Images[metallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2MapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2Map;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallic2, features: 0x {1:x} ",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_MAP |
                                 Material::HAS_METALLIC_MAP | Material::HAS_SKELETAL_ANIMATION))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMapIndex = material.m_RoughnessMapIndex;
            uint metallicMapIndex = material.m_MetallicMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size() ");
            CORE_ASSERT(normalMapIndex < m_Images.size(), " UFbxBuilder::AssignMaterial: normalMapIndex <m_Images.size() ");
            CORE_ASSERT(roughnessMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: roughnessMapIndex < m_Images.size()");
            CORE_ASSERT(metallicMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: metallicMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMapIndex], m_Images[metallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_ShaderData, m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSA2MapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallicSA2Map;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallicSA2, features:0x {1:x}",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures == (Material::HAS_DIFFUSE_MAP | Material::HAS_ROUGHNESS_MAP | Material::HAS_METALLIC_MAP))
        {
            LOG_CORE_CRITICAL("material diffuseRoughnessMetallic not supported");
        }
        else if (pbrFeatures & (Material::HAS_DIFFUSE_MAP | Material::HAS_NORMAL_MAP | Material::HAS_ROUGHNESS_MAP |
                                Material::HAS_METALLIC_MAP))
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            uint normalMapIndex = material.m_NormalMapIndex;
            uint roughnessMapIndex = material.m_RoughnessMapIndex;
            uint metallicMapIndex = material.m_MetallicMapIndex;

            CORE_ASSERT(diffuseMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: diffuseMapIndex < m_Images.size() ");
            CORE_ASSERT(normalMapIndex < m_Images.size(), " UFbxBuilder::AssignMaterial: normalMapIndex < m_Images.size() ");
            CORE_ASSERT(roughnessMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: roughnessMapIndex < m_Images.size()");
            CORE_ASSERT(metallicMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: metallicMapIndex < m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex], m_Images[normalMapIndex],
                                                               m_Images[roughnessMapIndex], m_Images[metallicMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(
                    MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2MapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseNormalRoughnessMetallic2Map;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuseNormalRoughnessMetallic2, features:0x {1:x}",
                          materialIndex, material.m_Features);
        }
        else if (pbrFeatures & Material::HAS_DIFFUSE_MAP)
        {
            uint diffuseMapIndex = material.m_DiffuseMapIndex;
            CORE_ASSERT(diffuseMapIndex < m_Images.size(), "UFbxBuilder::AssignMaterial: diffuseMapIndex <m_Images.size()");

            { // create material descriptor
                std::vector<std::shared_ptr<Texture>> textures{m_Images[diffuseMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrDiffuseMapInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrDiffuseMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrDiffuse, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrNoMapInstanced, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrNoMap;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrNoMap, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }

        // emissive materials
        if (material.m_Features & Material::HAS_EMISSIVE_MAP) // emissive texture
        {
            uint emissiveMapIndex = material.m_EmissiveMapIndex;
            CORE_ASSERT(emissiveMapIndex < m_Images.size(),
                        "UFbxBuilder::AssignMaterial: emissiveMapIndex < m_Images.size()");

            {
                std::vector<std::shared_ptr<Texture>> textures{m_Images[emissiveMapIndex]};
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor =
                    MaterialDescriptor::Create(MaterialDescriptor::MtPbrEmissiveTextureInstanced, textures, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrEmissiveTexture;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrEmissiveTexture, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
        else if (material.m_EmissiveColor * material.m_EmissiveStrength != glm::vec3(0.0f, 0.0f, 0.0f)) // emissive color
        {
            { // create material descriptor
                std::vector<std::shared_ptr<Buffer>> buffers{m_InstanceBuffer->GetBuffer()};
                auto materialDescriptor = MaterialDescriptor::Create(MaterialDescriptor::MtPbrEmissiveInstanced, buffers);
                submesh.m_MaterialDescriptors.push_back(materialDescriptor);
                m_MaterialFeatures |= MaterialDescriptor::MtPbrEmissive;
            }

            LOG_CORE_INFO("material assigned: material index {0}, PbrEmissive, features: 0x{1:x}", materialIndex,
                          material.m_Features);
        }
    }

    void UFbxBuilder::CalculateTangents()
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

    void UFbxBuilder::CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices)
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

    void UFbxBuilder::PrintProperties(const ufbx_material* fbxMaterial)
    {
        const char* materialName = fbxMaterial->name.data;
        size_t numberOfTextures = fbxMaterial->textures.count;
        LOG_CORE_WARN("material name: {0}, number of textures: {1}", materialName, numberOfTextures);

        // lambda to print a property
        auto printProperty = [&](std::string const& str, ufbx_material_map const& materialMap)
        {
            bool hasValue = materialMap.has_value;
            bool hasTexture = materialMap.texture;
            std::string message = str + ": ";
            if (hasValue)
            {
                if (hasTexture)
                {
                    std::string filename(materialMap.texture->filename.data);
                    message += "texture = " + filename;
                }
                else
                {
                    message += "constant value found ";
                    switch (materialMap.value_components)
                    {
                        case 0:
                        {
                            message += "component value is zero";
                            break;
                        }
                        case 1:
                        {
                            message += std::to_string(materialMap.value_real);
                            break;
                        }
                        case 2:
                        {
                            message +=
                                std::to_string(materialMap.value_vec2.x) + " " + std::to_string(materialMap.value_vec2.y);
                            break;
                        }
                        case 3:
                        {
                            message += std::to_string(materialMap.value_vec3.x) + " " +
                                       std::to_string(materialMap.value_vec3.y) + " " +
                                       std::to_string(materialMap.value_vec3.z);
                            break;
                        }
                        case 4:
                        {
                            message += std::to_string(materialMap.value_vec4.x) + " " +
                                       std::to_string(materialMap.value_vec4.y) + " " +
                                       std::to_string(materialMap.value_vec4.z) + " " +
                                       std::to_string(materialMap.value_vec4.w);
                            break;
                        }
                        default:
                            message += "component value out of range";
                            break;
                    }
                }
            }
            else
            {
                message += "no value found";
            }
            LOG_CORE_INFO(message);
        };

        printProperty("baseFactor", fbxMaterial->pbr.base_factor);
        printProperty("baseColor", fbxMaterial->pbr.base_color);
        printProperty("roughness", fbxMaterial->pbr.roughness);
        printProperty("metalness", fbxMaterial->pbr.metalness);
        printProperty("diffuseRoughness", fbxMaterial->pbr.diffuse_roughness);
        printProperty("normalMap", fbxMaterial->pbr.normal_map);
        printProperty("emissiveColor", fbxMaterial->pbr.emission_color);
        printProperty("emissiveFactor", fbxMaterial->pbr.emission_factor);
    }
} // namespace GfxRenderEngine
