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

#include <memory>

#include "tinygltf/tiny_gltf.h"

#include "engine.h"
#include "scene/material.h"
#include "scene/treeNode.h"
#include "scene/components.h"
#include "scene/dictionary.h"
#include "renderer/skeletalAnimation/skeleton.h"
#include "renderer/texture.h"
#include "renderer/cubemap.h"
#include "sprite/sprite.h"
#include "entt.hpp"

namespace GfxRenderEngine
{

    struct Vertex
    {
        glm::vec3   m_Position;
        glm::vec3   m_Color;
        glm::vec3   m_Normal;
        glm::vec2   m_UV;
        float       m_Amplification;
        int         m_Unlit;
        glm::vec3   m_Tangent;
        glm::ivec4  m_JointIds; 
        glm::vec4   m_Weights;

        bool operator==(const Vertex& other) const;

    };

    struct Material
    {
        enum Bitfield
        {
            HAS_DIFFUSE_MAP             = 0x01 << 0,
            HAS_NORMAL_MAP              = 0x01 << 1,
            HAS_ROUGHNESS_METALLIC_MAP  = 0x01 << 2,
            HAS_EMISSIVE_MAP            = 0x01 << 3,
            HAS_SKELETAL_ANIMATION      = 0x01 << 4
        };
        glm::vec3 m_DiffuseColor;
        glm::vec3 m_EmissiveFactor;
        float m_EmissiveStrength;
        uint m_DiffuseMapIndex;
        uint m_DiffuseSAMapIndex;
        uint m_NormalMapIndex;
        uint m_RoughnessMettalicMapIndex;
        uint m_EmissiveMapIndex;
        uint m_Features;
        float m_Roughness;
        float m_Metallic;
        float m_NormalMapIntensity;
    };

    struct PrimitiveTmp
    {
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
    };

    struct PrimitiveNoMap
    {
        ~PrimitiveNoMap();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrNoMapMaterial m_PbrNoMapMaterial{};
    };

    struct PrimitiveEmissive
    {
        ~PrimitiveEmissive();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrEmissiveMaterial m_PbrEmissiveMaterial{};
    };

    struct PrimitiveEmissiveTexture
    {
        ~PrimitiveEmissiveTexture();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrEmissiveTextureMaterial m_PbrEmissiveTextureMaterial{};
    };

    struct PrimitiveDiffuseMap
    {
        ~PrimitiveDiffuseMap();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseMaterial m_PbrDiffuseMaterial;
    };

    struct PrimitiveDiffuseSAMap
    {
        ~PrimitiveDiffuseSAMap();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseSAMaterial m_PbrDiffuseSAMaterial;
    };

    struct PrimitiveDiffuseNormalMap
    {
        ~PrimitiveDiffuseNormalMap();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseNormalMaterial m_PbrDiffuseNormalMaterial;
    };

    struct PrimitiveDiffuseNormalRoughnessMetallicMap
    {
        ~PrimitiveDiffuseNormalRoughnessMetallicMap();
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseNormalRoughnessMetallicMaterial m_PbrDiffuseNormalRoughnessMetallicMaterial;
    };

    struct PrimitiveCubemap
    {
        ~PrimitiveCubemap();
        uint m_FirstVertex;
        uint m_VertexCount;
        CubemapMaterial m_CubemapMaterial;
    };

    class Builder
    {

    public:

        static constexpr int GLTF_NOT_USED = -1;

    public:

        Builder() {}
        Builder(const std::string& filepath);

        void LoadModel(const std::string& filepath, int fragAmplification = 1.0);
        entt::entity LoadGLTF(entt::registry& registry, TreeNode& sceneHierarchy, Dictionary& dictionary, TransformComponent* transform = nullptr);
        void LoadSprite(const Sprite& sprite, float amplification = 0.0f, int unlit = 0, const glm::vec4& color = glm::vec4(1.0f));
        entt::entity LoadCubemap(const std::vector<std::string>& faces, entt::registry& registry);
        void LoadParticle(const glm::vec4& color);

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
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicMap> m_PrimitivesDiffuseNormalRoughnessMetallicMap{};

        std::vector<std::shared_ptr<Cubemap>> m_Cubemaps;
        std::vector<PrimitiveCubemap> m_PrimitivesCubemap{};

    private:

        void LoadImagesGLTF();
        void LoadMaterialsGLTF();
        void LoadVertexDataGLTF(uint meshIndex);
        bool GetImageFormatGLTF(uint imageIndex);
        void LoadTransformationMatrix(TransformComponent& transform, int nodeIndex);
        void AssignMaterial(const PrimitiveTmp& primitiveTmp, int materialIndex);
        void ProcessNode(tinygltf::Scene& scene, uint nodeIndex, entt::registry& registry, Dictionary& dictionary, TreeNode* currentNode);
        TreeNode* CreateGameObject(tinygltf::Scene& scene, uint nodeIndex, entt::registry& registry, Dictionary& dictionary, TreeNode* currentNode);
        void CalculateTangents();
        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);

    private:

        std::string m_Filepath;
        std::string m_Basepath;
        tinygltf::Model m_GltfModel;
        tinygltf::TinyGLTF m_GltfLoader;
        std::vector<Material> m_Materials;
        TransformComponent* m_Transform;
        uint m_ImageOffset;
        entt::entity m_GameObject;

    // skeletal animtion
    private:

        void LoadSkeletons(Material& material);
        void LoadJoint(SkeletalAnimation::Skeleton& skeleton, int globalGltfNodeIndex, int parentJoint);

    public:

        std::vector<SkeletalAnimation::Skeleton> m_Skeletons;
        std::shared_ptr<Buffer> m_ShaderData;
    };

    class Model
    {

    public:

        Model() {}
        virtual ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        virtual void CreateVertexBuffers(const std::vector<Vertex>& vertices) = 0;
        virtual void CreateIndexBuffers(const std::vector<uint>& indices) = 0;

        static float m_NormalMapIntensity;

    protected:

        std::vector<std::shared_ptr<Texture>> m_Images;
        std::vector<std::shared_ptr<Cubemap>> m_Cubemaps;

    };
}
