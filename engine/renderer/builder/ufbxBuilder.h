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

#pragma once

#include "ufbx/ufbx.h"

#include "renderer/model.h"
#include "scene/fbx.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class UFbxBuilder
    {

    public:
        UFbxBuilder() = delete;
        UFbxBuilder(const std::string& filepath, Scene& scene);

        bool Load(uint const instanceCount = 1, int const sceneID = Fbx::FBX_NOT_USED);
        void SetDictionaryPrefix(std::string const&);

    public:
        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<Submesh> m_Submeshes{};

    private:
        void LoadVertexData(const ufbx_node* fbxNodePtr);
        void LoadVertexData(const ufbx_node* fbxNodePtr, uint const submeshIndex);

        void LoadMaterials();
        void LoadMaterial(const ufbx_material* fbxMaterial, ufbx_material_pbr_map materialProperty, int materialIndex);
        std::shared_ptr<Texture> LoadTexture(ufbx_material_map const& materialMap, bool useSRGB);

        void AssignMaterial(Submesh& submesh, int const materialIndex);
        void LoadTransformationMatrix(const ufbx_node* fbxNodePtr, glm::vec3& scale, glm::quat& rotation,
                                      glm::vec3& translation);
        void PrintProperties(const ufbx_material* fbxMaterial);

        bool MarkNode(const ufbx_node* fbxNodePtr);
        void ProcessNode(const ufbx_node* fbxNodePtr, uint parentNode, uint& hasMeshIndex);
        uint CreateGameObject(const ufbx_node* fbxNodePtr, uint const parentNode);

        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);
        void CalculateTangents();

    private:
        std::string m_Filepath;
        std::string m_Basepath;
        std::string m_DictionaryPrefix;
        ufbx_scene* m_FbxScene;
        std::vector<Material> m_Materials;
        std::unordered_map<std::string, uint> m_MaterialNameToIndex;
        bool m_FbxNoBuiltInTangents;
        std::shared_ptr<Model> m_Model;
        std::shared_ptr<InstanceBuffer> m_InstanceBuffer;
        std::vector<entt::entity> m_InstancedObjects;
        uint m_RenderObject;
        std::vector<std::shared_ptr<Texture>> m_Textures;
        std::vector<Material::MaterialTextures> m_MaterialTextures{};

        // scene graph
        uint m_InstanceCount;
        uint m_InstanceIndex;
        std::vector<bool> m_HasMesh;
        entt::entity m_GameObject;

        Registry& m_Registry;
        SceneGraph& m_SceneGraph;
        Dictionary& m_Dictionary;

        // skeletal animation
    private:
        void LoadSkeletonsFbx();
        bool m_SkeletalAnimation;

    public:
        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderData;
        std::shared_ptr<SkeletalAnimations> m_Animations;
    };
} // namespace GfxRenderEngine
