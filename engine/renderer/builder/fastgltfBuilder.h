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

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "renderer/model.h"
#include "renderer/gltf.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class FastgltfBuilder
    {

    public:
        FastgltfBuilder() = delete;
        FastgltfBuilder(const std::string& filepath, Scene& scene);

        bool LoadGltf(uint const instanceCount = 1, int const sceneID = Gltf::GLTF_NOT_USED);

    public:
        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<std::shared_ptr<Texture>> m_Images{};
        std::vector<ModelSubmesh> m_Submeshes{};

    private:
        void LoadImagesGltf();
        void LoadMaterialsGltf();
        void LoadVertexDataGltf(uint const meshIndex);
        bool GetImageFormatGltf(uint const imageIndex);
        void AssignMaterial(ModelSubmesh& submesh, int const materialIndex);
        void LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex);
        void CalculateTangents();
        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);

        bool MarkNode(fastgltf::Scene& scene, int const gltfNodeIndex);
        void ProcessScene(fastgltf::Scene& scene, uint const parentNode);
        void ProcessNode(fastgltf::Scene& scene, int const gltfNodeIndex, uint const parentNode);
        uint CreateGameObject(fastgltf::Scene& scene, int const gltfNodeIndex, uint const parentNode);
        int GetMinFilter(uint index);
        int GetMagFilter(uint index);

        void PrintAssetError(fastgltf::Error assetErrorCode);

    private:
        template <typename T> fastgltf::ComponentType LoadAccessor(const fastgltf::Accessor& accessor, const T*& pointer,
                                                                   size_t* count = nullptr,
                                                                   fastgltf::AccessorType* type = nullptr)
        {
            CORE_ASSERT(accessor.bufferViewIndex.has_value(), "Loadaccessor: no buffer view index provided");
            const fastgltf::BufferView& bufferView = m_GltfModel.bufferViews[accessor.bufferViewIndex.value()];
            auto& buffer = m_GltfModel.buffers[bufferView.bufferIndex];
            std::visit(fastgltf::visitor{[&](auto& arg) // default branch if image data is not supported
                                         { LOG_CORE_CRITICAL("not supported default branch (LoadAccessor) "); },
                                         [&](fastgltf::sources::Array& vector) {
                                             pointer =
                                                 reinterpret_cast<const T*>(vector.bytes.data() + bufferView.byteOffset);
                                         }},
                       buffer.data);
            if (count)
            {
                count[0] = accessor.count;
            }
            if (type)
            {
                type[0] = accessor.type;
            }
            return accessor.componentType;
        }

    private:
        std::string m_Filepath;
        std::string m_Basepath;
        fastgltf::Asset m_GltfModel;
        std::shared_ptr<Model> m_Model;
        std::vector<Material> m_Materials;
        uint m_MaterialFeatures;

        // scene graph
        uint m_InstanceCount;
        uint m_InstanceIndex;
        std::vector<bool> m_HasMesh;
        entt::entity m_GameObject;

        std::vector<entt::entity> m_InstancedObjects;
        std::shared_ptr<InstanceBuffer> m_InstanceBuffer;
        uint m_RenderObject;

        entt::registry& m_Registry;
        SceneGraph& m_SceneGraph;
        Dictionary& m_Dictionary;

        // skeletal animation
    private:
        void LoadSkeletonsGltf();
        void LoadJoint(int globalGltfNodeIndex, int parentJoint);
        uint m_SkeletalAnimation;

    public:
        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderData;
        std::shared_ptr<SkeletalAnimations> m_Animations;
    };
} // namespace GfxRenderEngine
