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
#include "scene/treeNode.h"
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

    struct TransformComponent;

    class Builder
    {

    public:

        Builder() {}
        Builder(const std::string& filepath);

        void LoadModel(const std::string& filepath, int diffuseMapTextureSlot = 0, int fragAmplification = 1.0, int normalTextureSlot = 0);
        void LoadGLTF(entt::registry& registry, TreeNode& sceneHierarchy, TransformComponent* transform = nullptr);
        void LoadSprite(Sprite* sprite, const glm::mat4& position, float amplification, int unlit = 0, const glm::vec4& color = glm::vec4(1.0f));
        void LoadParticle(const glm::vec4& color);

    public:

        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};

    private:

        void LoadImagesGLTF();
        void LoadMaterialsGLTF();
        void LoadVertexDataGLTF(uint meshIndex);
        void LoadTransformationMatrix(TransformComponent& transform, int meshIndex);
        void AssignMaterial(entt::registry& registry, entt::entity entity, int materialIndex);
        void CalculateTangents();

    private:

        std::string m_Filepath;
        std::string m_Basepath;
        tinygltf::Model m_GltfModel;
        tinygltf::TinyGLTF m_GltfLoader;
        std::vector<Material> m_Materials;
        TransformComponent* m_Transform;

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

    };
}
