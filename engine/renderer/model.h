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

#define GL_BYTE 0x1400           // 5120
#define GL_UNSIGNED_BYTE 0x1401  // 5121
#define GL_SHORT 0x1402          // 5122
#define GL_UNSIGNED_SHORT 0x1403 // 5123
#define GL_INT 0x1404            // 5124
#define GL_UNSIGNED_INT 0x1405   // 5125
#define GL_FLOAT 0x1406          // 5126
#define GL_2_BYTES 0x1407        // 5127
#define GL_3_BYTES 0x1408        // 5128
#define GL_4_BYTES 0x1409        // 5129
#define GL_DOUBLE 0x140A         // 5130

#include <memory>

#include "tinygltf/tiny_gltf.h"

#include "engine.h"
#include "scene/material.h"
#include "scene/sceneGraph.h"
#include "scene/components.h"
#include "scene/dictionary.h"
#include "renderer/skeletalAnimation/skeleton.h"
#include "renderer/skeletalAnimation/skeletalAnimations.h"
#include "renderer/materialDescriptor.h"
#include "renderer/texture.h"
#include "renderer/cubemap.h"
#include "sprite/sprite.h"
#include "entt.hpp"

namespace GfxRenderEngine
{

    struct Vertex
    {
        glm::vec3 m_Position;
        glm::vec4 m_Color;
        glm::vec3 m_Normal;
        glm::vec2 m_UV;
        float m_Amplification;
        int m_Unlit;
        glm::vec3 m_Tangent;
        glm::ivec4 m_JointIds;
        glm::vec4 m_Weights;

        bool operator==(const Vertex& other) const;
    };

    struct Material
    {
        enum Bitfield
        {
            NO_MAP = 0x0,
            HAS_DIFFUSE_MAP = 0x1 << 0x0,
            HAS_NORMAL_MAP = 0x1 << 0x1,
            HAS_ROUGHNESS_MAP = 0x1 << 0x2,
            HAS_METALLIC_MAP = 0x1 << 0x3,
            HAS_ROUGHNESS_METALLIC_MAP = 0x1 << 0x4,
            HAS_EMISSIVE_MAP = 0x1 << 0x5,
            HAS_SKELETAL_ANIMATION = 0x1 << 0x6
        };
        // constant material properties
        glm::vec4 m_DiffuseColor{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec3 m_EmissiveColor{0.0f, 0.0f, 0.0f};
        float m_NormalMapIntensity{1.0f};
        float m_Roughness{0.1};
        float m_Metallic{0.5};
        float m_EmissiveStrength{1.0f};
        // map indices
        uint m_DiffuseMapIndex;
        uint m_NormalMapIndex;
        uint m_RoughnessMapIndex;
        uint m_MetallicMapIndex;
        uint m_RoughnessMetallicMapIndex;
        uint m_EmissiveMapIndex;
        // feature bits
        uint m_Features;
    };

    struct MaterialProperties
    {
        float m_NormalMapIntensity;
        float m_Roughness;
        float m_Metallic;
        float m_EmissiveStrength;
        glm::vec4 m_EmissiveColor;
        glm::vec4 m_BaseColorFactor;
    };

    struct Submesh
    {
        uint m_FirstIndex;
        uint m_FirstVertex;
        uint m_IndexCount;
        uint m_VertexCount;
        uint m_InstanceCount;
        MaterialProperties m_MaterialProperties;
    };

    struct ModelSubmesh : public Submesh
    {
        std::vector<std::shared_ptr<MaterialDescriptor>> m_MaterialDescriptors;
    };

    class Model
    {

    public:
        Model() {}
        virtual ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        virtual void CreateVertexBuffers(const std::vector<Vertex>& vertices) = 0;
        virtual void CreateIndexBuffers(const std::vector<uint>& indices) = 0;

        SkeletalAnimations& GetAnimations();

        static float m_NormalMapIntensity;

    protected:
        std::vector<std::shared_ptr<Texture>> m_Textures;
        std::vector<std::shared_ptr<Cubemap>> m_Cubemaps;

        // skeletal animation
        std::shared_ptr<SkeletalAnimations> m_Animations;
        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderDataUbo;
    };
} // namespace GfxRenderEngine
