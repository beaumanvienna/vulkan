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

#include "renderer/model.h"
#include "scene/gltf.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class GltfBuilder
    {

    public:
        GltfBuilder() = delete;
        GltfBuilder(const std::string& filepath, Scene& scene);

        bool Load(uint const instanceCount = 1, int const sceneID = Gltf::GLTF_NOT_USED);
        void SetDictionaryPrefix(std::string const&);

    public:
        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<Submesh> m_Submeshes{};

    private:
        void LoadTextures();
        void LoadMaterials();
        void LoadVertexData(uint const meshIndex);
        bool GetImageFormat(uint const imageIndex);
        void AssignMaterial(Submesh& submesh, int const materialIndex);
        void LoadTransformationMatrix(TransformComponent& transform, int const gltfNodeIndex);
        void CalculateTangents();
        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);

        bool MarkNode(tinygltf::Scene& scene, int const gltfNodeIndex);
        void ProcessScene(tinygltf::Scene& scene, uint const parentNode);
        void ProcessNode(tinygltf::Scene& scene, int const gltfNodeIndex, uint const parentNode);
        uint CreateGameObject(tinygltf::Scene& scene, int const gltfNodeIndex, uint const parentNode);
        int GetMinFilter(uint index);
        int GetMagFilter(uint index);

    private:
        template <typename T>
        int LoadAccessor(const tinygltf::Accessor& accessor, const T*& pointer, uint* count = nullptr, int* type = nullptr)
        {
            const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
            pointer =
                reinterpret_cast<const T*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            if (count)
            {
                *count = static_cast<uint>(accessor.count);
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
        tinygltf::Model m_GltfModel;
        tinygltf::TinyGLTF m_GltfLoader;
        std::shared_ptr<Model> m_Model;
        std::vector<Material> m_Materials;
        uint m_TextureOffset;
        std::vector<std::shared_ptr<Texture>> m_Textures{};
        std::vector<Material::MaterialTextures> m_MaterialTextures{};

        // scene graph
        uint m_InstanceCount;
        uint m_InstanceIndex;
        std::vector<bool> m_HasMesh;
        entt::entity m_GameObject;

        std::vector<entt::entity> m_InstancedObjects;
        std::shared_ptr<InstanceBuffer> m_InstanceBuffer;
        uint m_RenderObject;

        Registry& m_Registry;
        SceneGraph& m_SceneGraph;
        Dictionary& m_Dictionary;

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
