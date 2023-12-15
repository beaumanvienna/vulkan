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
#include "renderer/gltf.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{

    class GltfBuilder
    {

    public:

        GltfBuilder() = delete;
        GltfBuilder(const std::string& filepath, Scene& scene);

        bool LoadGltf(uint const instanceCount = 1, int const sceneID = Gltf::GLTF_NOT_USED);

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

        void LoadImagesGltf();
        void LoadMaterialsGltf();
        void LoadVertexDataGltf(uint const meshIndex);
        bool GetImageFormatGltf(uint const imageIndex);
        void AssignMaterial(const PrimitiveTmp& primitiveTmp, int const materialIndex);
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

        template<typename T>
        int LoadAccessor(const tinygltf::Accessor& accessor, const T*& pointer, uint* count = nullptr, int* type = nullptr)
        {
            const tinygltf::BufferView& view = m_GltfModel.bufferViews[accessor.bufferView];
            pointer = reinterpret_cast<const T*>(&(m_GltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            if (count)
            {
                count[0] = static_cast<uint>(accessor.count);
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
        tinygltf::Model m_GltfModel;
        tinygltf::TinyGLTF m_GltfLoader;
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

        void LoadSkeletonsGltf();
        void LoadJoint(int globalGltfNodeIndex, int parentJoint);
        uint m_SkeletalAnimation;

    public:

        std::shared_ptr<Armature::Skeleton> m_Skeleton;
        std::shared_ptr<Buffer> m_ShaderData;
        std::shared_ptr<SkeletalAnimations> m_Animations;
    };
}
