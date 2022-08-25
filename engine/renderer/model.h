/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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
#include "renderer/texture.h"
#include "sprite/sprite.h"
#include "entt.hpp"

namespace GfxRenderEngine
{

    struct Vertex
    {
        glm::vec3 m_Position;
        glm::vec3 m_Color;
        glm::vec3 m_Normal;
        glm::vec2 m_UV;
        int m_DiffuseMapTextureSlot;
        float m_Amplification;
        int m_Unlit;
        glm::vec3 m_Tangent;

        bool operator==(const Vertex& other) const;

    };

    struct Material
    {
        enum Bitfield
        {
            HAS_DIFFUSE_MAP             = 0x01 << 0,
            HAS_NORMAL_MAP              = 0x01 << 1,
            HAS_ROUGHNESS_METALLIC_MAP  = 0x01 << 2
        };
        glm::vec3 m_DiffuseColor;
        uint m_DiffuseMapIndex;
        uint m_NormalMapIndex;
        uint m_RoughnessMettalicMapIndex;
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
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrNoMapMaterial m_PbrNoMapMaterial{};
    };

    struct PrimitiveDiffuseMap
    {
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseMaterial m_PbrDiffuseMaterial;
    };

    struct PrimitiveDiffuseNormalMap
    {
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseNormalMaterial m_PbrDiffuseNormalMaterial;
    };

    struct PrimitiveDiffuseNormalRoughnessMetallicMap
    {
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        PbrDiffuseNormalRoughnessMetallicMaterial m_PbrDiffuseNormalRoughnessMetallicMaterial;
    };

    class Builder
    {

    public:

        Builder() {}
        Builder(const std::string& filepath);

        void LoadModel(const std::string& filepath, int diffuseMapTextureSlot = 0, int fragAmplification = 1.0, int normalTextureSlot = 0);
        entt::entity LoadGLTF(entt::registry& registry, TreeNode& sceneHierarchy, Dictionary& dictionary, TransformComponent* transform = nullptr);
        void LoadSprite(const Sprite& sprite, float amplification = 0.0f, int unlit = 0, const glm::vec4& color = glm::vec4(1.0f));
        void LoadParticle(const glm::vec4& color);

    public:

        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<PrimitiveNoMap> m_PrimitivesNoMap{};
        std::vector<PrimitiveDiffuseMap> m_PrimitivesDiffuseMap{};
        std::vector<PrimitiveDiffuseNormalMap> m_PrimitivesDiffuseNormalMap{};
        std::vector<PrimitiveDiffuseNormalRoughnessMetallicMap> m_PrimitivesDiffuseNormalRoughnessMetallicMap{};

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

    };

    class Model
    {

    public:

        Model() {}
        virtual ~Model() {}

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        virtual void CreateVertexBuffers(const std::vector<Vertex>& vertices) = 0;
        virtual void CreateIndexBuffers(const std::vector<uint>& indices) = 0;

        static float m_NormalMapIntensity;
    };
}
