/* Engine Copyright (c) 2025 Engine Development Team
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

#include "scene/gltf.h"
#include "renderer/model.h"

// Jolt includes
#include <Jolt/Jolt.h>

// debug rendering
#include <Renderer/DebugRendererImp.h>

namespace GfxRenderEngine
{

    class FastgltfVertexLoader
    {
    public:
        struct Vertex
        {
            glm::vec3 m_Position;
        };

    public:
        FastgltfVertexLoader() = delete;
        FastgltfVertexLoader(const std::string& filepath, TriangleList& triangles);

        bool Load(int const sceneID = Gltf::GLTF_NOT_USED);

    private:
        void LoadVertexData(uint const meshIndex);
        void ProcessScene(fastgltf::Scene& scene);
        void ProcessNode(fastgltf::Scene* scene, int const gltfNodeIndex);
        void PrintAssetError(fastgltf::Error assetErrorCode);
        inline JPH::Float3& ConvertToFloat3(glm::vec3& vec3GLM) { return *reinterpret_cast<JPH::Float3*>(&vec3GLM); }

    private:
        template <typename T> fastgltf::ComponentType LoadAccessor(const fastgltf::Accessor& accessor, const T*& pointer,
                                                                   size_t* count = nullptr,
                                                                   fastgltf::AccessorType* type = nullptr)
        {
            CORE_ASSERT(accessor.bufferViewIndex.has_value(), "Loadaccessor: no buffer view index provided");

            const fastgltf::BufferView& bufferView = m_GltfAsset.bufferViews[accessor.bufferViewIndex.value()];
            auto& buffer = m_GltfAsset.buffers[bufferView.bufferIndex];

            const fastgltf::sources::Array* vector = std::get_if<fastgltf::sources::Array>(&buffer.data);
            CORE_ASSERT(vector, "FastgltfVertexLoader::LoadAccessor: unsupported data type");

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
        std::vector<FastgltfVertexLoader::Vertex> m_Vertices;
        std::vector<uint> m_Indicies;
        TriangleList& m_Triangles;
        fastgltf::Asset m_GltfAsset;
    };
} // namespace GfxRenderEngine
