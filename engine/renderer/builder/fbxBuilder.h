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
#include "renderer/fbx.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class FbxBuilder
    {

    public:

        FbxBuilder() = delete;
        FbxBuilder(const std::string& filepath, Scene& scene);

        bool LoadFbx(uint const instanceCount = 1, int const sceneID = Fbx::FBX_NOT_USED);

    public:

        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<std::shared_ptr<Texture>> m_Images;
        std::vector<PrimitiveNoMap> m_PrimitivesNoMap{};
        std::vector<PrimitiveEmissive> m_PrimitivesEmissive{};
        std::vector<PrimitiveDiffuseMap> m_PrimitivesDiffuseMap{};
        std::vector<PrimitiveDiffuseSAMap> m_PrimitivesDiffuseSAMap{};
        std::vector<PrimitiveEmissiveTexture> m_PrimitivesEmissiveTexture{};
        std::vector<PrimitiveDiffuseNormalMap> m_PrimitivesDiffuseNormalMap{};
        std::vector<PrimitiveDiffuseNormalSAMap> m_PrimitivesDiffuseNormalSAMap{};
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicMap> m_PrimitivesDiffuseNormalRoughnessMetallicMap{};
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicSAMap> m_PrimitivesDiffuseNormalRoughnessMetallicSAMap{};

    private:

        bool LoadImageFbx(std::string& filepath, uint& mapIndex, bool useSRGB);
        bool LoadMap(const aiMaterial* fbxMaterial, aiTextureType textureType, Material& engineMaterial);
        void LoadMaterialsFbx();
        void LoadVertexDataFbx(const aiNode* fbxNodePtr, uint const meshIndex, int vertexColorSet = 0, uint uvSet = 0);
        void AssignMaterial(const PrimitiveTmp& primitiveTmp, uint const materialIndex);
        void LoadTransformationMatrix(TransformComponent& transform, const aiNode* fbxNodePtr);

        bool MarkNode(const aiNode* fbxNodePtr);
        void ProcessNode(const aiNode* fbxNodePtr, uint const parentNode, int hasMeshIndex);
        uint CreateGameObject(const aiNode* fbxNodePtr, uint const parentNode);
        
    private:

        std::string m_Filepath;
        std::string m_Basepath;
        const aiScene* m_FbxScene;
        std::vector<Material> m_Materials;

        uint m_ImageOffset;

        // scene graph
        uint m_InstanceCount;
        uint m_InstanceIndex;
        std::vector<bool> m_HasMesh;
        entt::entity m_GameObject;

        entt::registry& m_Registry;
        SceneGraph& m_SceneGraph;
        Dictionary& m_Dictionary;

    // skeletal animation
    private:

        void LoadSkeletonsFbx();
        void LoadJoint(int globalFbxNodeIndex, int parentJoint);
        uint m_SkeletalAnimation;

    public:

        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderData;
        std::shared_ptr<SkeletalAnimations> m_Animations;
    };
}
