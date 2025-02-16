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

#include "scene/grass.h"

namespace GfxRenderEngine
{
    class Scene;
    struct Vertex;
    class GrassBuilder
    {

    public:
        GrassBuilder() = delete;
        GrassBuilder(Grass::GrassSpec& grassSpec, Scene& scene);

        bool LoadMask();
        bool LoadVertexData();
        bool Build();

    private:
        struct MaskData
        {
            struct Quad
            {
                void Print()
                {
                    std::cout << m_Indices[0] << ", " << m_Indices[1] << ", " << m_Indices[2] << ", " << m_Indices[3]
                              << std::endl;
                };
                int m_Indices[4];
            };
            std::vector<uint> m_Indices{};
            std::vector<Vertex> m_Vertices{};
            std::vector<Quad> m_Quads{};
        };

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
        Grass::GrassSpec& m_GrassSpec;
        Scene& m_Scene;
        bool LoadVertexData(uint const meshIndex);
        bool ExtractQuads();
        bool CreateInstances();
        void PrintAssetError(fastgltf::Error assetErrorCode);
        fastgltf::Asset m_GltfAsset;
        std::vector<MaskData> m_MaskData;
    };
} // namespace GfxRenderEngine
