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

#include <future>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "scene/gltf.h"
#include "scene/material.h"
#include "scene/registry.h"
#include "renderer/model.h"
#include "renderer/resourceDescriptor.h"
#include "auxiliary/queue.h"

namespace GfxRenderEngine
{
    namespace Armature
    {
        struct Skeleton;
    }
    class Buffer;
    class Dictionary;
    class InstanceBuffer;
    class Material;
    class Scene;
    class SceneGraph;
    class SkeletalAnimations;
    struct Submesh;
    class Texture;
    class TransformComponent;
    struct Vertex;

    class FastgltfBuilder
    {

    public:
        FastgltfBuilder() = delete;
        FastgltfBuilder(const std::string& filepath, Scene& scene, Resources::ResourceBuffers* resourceBuffers = nullptr);
        FastgltfBuilder(const std::string& filepath, Scene& scene, int groupNode);

        bool Load(uint const instanceCount = 1, int const sceneID = Gltf::GLTF_NOT_USED);
        bool Load(uint const instanceCount, std::vector<entt::entity>& firstInstances, bool useSceneGraph = true);
        void SetDictionaryPrefix(std::string const&);

    private:
        void LoadTextures();
        void LoadMaterials();
        void LoadVertexData(uint const, Model::ModelData&);
        bool GetImageFormat(uint const imageIndex);
        void AssignMaterial(Submesh& submesh, int const materialIndex, InstanceBuffer* instanceBuffer);
        void LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex);
        void CalculateTangents(Model::ModelData&);
        void CalculateTangentsFromIndexBuffer(Model::ModelData&, const std::vector<uint>& indices);

        bool MarkNode(int const gltfNodeIndex);
        void ProcessScene(fastgltf::Scene& scene, uint const parentNode, uint instanceIndex);
        void ProcessNode(fastgltf::Scene* scene, int const gltfNodeIndex, uint const parentNode, uint instanceIndex);
        uint CreateGameObject(fastgltf::Scene* scene, int const gltfNodeIndex, uint const parentNode, uint instanceIndex);
        int GetMinFilter(uint index);
        int GetMagFilter(uint index);

        void PrintAssetError(fastgltf::Error assetErrorCode);

    private:
        template <typename T> fastgltf::ComponentType LoadAccessor(const fastgltf::Accessor& accessor, const T*& pointer,
                                                                   size_t* count = nullptr,
                                                                   fastgltf::AccessorType* type = nullptr)
        {
            CORE_ASSERT(accessor.bufferViewIndex.has_value(), "Loadaccessor: no buffer view index provided");

            const fastgltf::BufferView& bufferView = m_GltfAsset.bufferViews[accessor.bufferViewIndex.value()];
            auto& buffer = m_GltfAsset.buffers[bufferView.bufferIndex];

            const fastgltf::sources::Array* vector = std::get_if<fastgltf::sources::Array>(&buffer.data);
            CORE_ASSERT(vector, "FastgltfBuilder::LoadAccessor: unsupported data type");

            size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
            pointer = reinterpret_cast<const T*>(vector->bytes.data() + dataOffset);

            if (count)
            {
                *count = accessor.count;
            }
            if (type)
            {
                *type = accessor.type;
            }
            return accessor.componentType;
        }

    private:
        std::string m_Filepath;
        std::string m_Basepath;
        std::string m_DictionaryPrefix;
        fastgltf::Asset m_GltfAsset;
        std::vector<std::shared_ptr<Model>> m_Models;
        std::vector<Material> m_Materials;
        std::vector<Material::MaterialTextures> m_MaterialTextures{};
        std::vector<std::shared_ptr<Texture>> m_Textures{};

        // scene graph
        uint m_InstanceCount{0};
        std::vector<entt::entity> m_FirstInstances;
        int m_GroupNode;
        bool m_UseSceneGraph{true};
        std::vector<bool> m_HasMesh;
        Atomic::Queue<std::future<bool>> m_NodeFuturesQueue;
        std::unordered_map<int, entt::entity> m_InstancedObjects;
        Resources::ResourceBuffers m_ResourceBuffersPre;

        Registry& m_Registry;
        SceneGraph& m_SceneGraph;
        Dictionary& m_Dictionary;

        std::mutex m_Mutex;

        // skeletal animation
    private:
        void LoadSkeletonsGltf();
        void LoadJoint(int globalGltfNodeIndex, int parentJoint);
        bool m_SkeletalAnimation;

    public:
        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderData;
        std::shared_ptr<SkeletalAnimations> m_Animations;
    };
} // namespace GfxRenderEngine
