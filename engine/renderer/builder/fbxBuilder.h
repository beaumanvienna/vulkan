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

#pragma once

#include "assimp/scene.h"

#include "renderer/model.h"
#include "scene/fbx.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class FbxBuilder
    {

    public:
        FbxBuilder() = delete;
        FbxBuilder(const std::string& filepath, Scene& scene);

        bool Load(uint const instanceCount = 1, int const sceneID = Fbx::FBX_NOT_USED);
        void SetDictionaryPrefix(std::string const&);

    public:
        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<Submesh> m_Submeshes{};

    private:
        void LoadVertexData(const aiNode* fbxNodePtr, int vertexColorSet = 0, uint uvSet = 0);
        void LoadVertexData(const aiNode* fbxNodePtr, uint const meshIndex, uint const fbxMeshIndex, int vertexColorSet = 0,
                            uint uvSet = 0);

        void LoadMaterials();
        std::shared_ptr<Texture> LoadTexture(std::string const& filepath, bool useSRGB);
        void LoadProperties(const aiMaterial* fbxMaterial, Material::PbrMaterial& pbrMaterial);
        void LoadMap(const aiMaterial* fbxMaterial, aiTextureType textureType, int materialIndex);
        void AssignMaterial(Submesh& submesh, int const materialIndex);
        glm::mat4 LoadTransformationMatrix(const aiNode* fbxNodePtr);
        void PrintMaps(const aiMaterial* fbxMaterial);

        bool MarkNode(const aiNode* fbxNodePtr);
        void ProcessNode(const aiNode* fbxNodePtr, uint const parentNode, uint& hasMeshIndex);
        uint CreateGameObject(const aiNode* fbxNodePtr, uint const parentNode);

        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);
        void CalculateTangents();

    private:
        std::string m_Filepath;
        std::string m_Basepath;
        std::string m_DictionaryPrefix;
        const aiScene* m_FbxScene;
        std::vector<Material> m_Materials;
        std::vector<std::shared_ptr<Texture>> m_Textures;
        std::vector<Material::MaterialTextures> m_MaterialTextures{};
        bool m_FbxNoBuiltInTangents;
        std::shared_ptr<Model> m_Model;
        std::shared_ptr<InstanceBuffer> m_InstanceBuffer;
        std::vector<entt::entity> m_InstancedObjects;
        uint m_RenderObject;

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
