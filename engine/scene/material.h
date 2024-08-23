/* Engine Copyright (c) 2022 Engine Development Team
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

#include "engine/platform/Vulkan/material.h"
#include "engine.h"

namespace GfxRenderEngine
{
    class Texture;
    class Buffer;
    class MaterialDescriptor;
    class Material
    {
    public:
        enum TextureIndices
        {
            DIFFUSE_MAP_INDEX = 0,
            NORMAL_MAP_INDEX,
            ROUGHNESS_MAP_INDEX,
            METALLIC_MAP_INDEX,
            ROUGHNESS_METALLIC_MAP_INDEX,
            EMISSIVE_MAP_INDEX,
            NUM_TEXTURES
        };

        // fixed-size array for material textures
        typedef std::array<std::shared_ptr<Texture>, Material::NUM_TEXTURES> MaterialTextures;

        enum MaterialFeatures // bitset
        {
            HAS_DIFFUSE_MAP = GLSL_HAS_DIFFUSE_MAP,
            HAS_NORMAL_MAP = GLSL_HAS_NORMAL_MAP,
            HAS_ROUGHNESS_MAP = GLSL_HAS_ROUGHNESS_MAP,
            HAS_METALLIC_MAP = GLSL_HAS_METALLIC_MAP,
            HAS_ROUGHNESS_METALLIC_MAP = GLSL_HAS_ROUGHNESS_METALLIC_MAP,
            HAS_EMISSIVE_COLOR = GLSL_HAS_EMISSIVE_COLOR,
            HAS_EMISSIVE_MAP = GLSL_HAS_EMISSIVE_MAP
        };

        struct PbrMaterial
        { // align data to blocks of 16 bytes
            // byte 0 to 15
            uint m_Features{0};
            float m_Roughness{0.0f};
            float m_Metallic{0.0f};
            float m_Spare0{0.0f}; // padding

            // byte 16 to 31
            glm::vec4 m_DiffuseColor{1.0f, 1.0f, 1.0f, 1.0f};

            // byte 32 to 47
            glm::vec3 m_EmissiveColor{0.0f, 0.0f, 0.0f};
            float m_EmissiveStrength{1.0f};

            // byte 48 to 63
            float m_NormalMapIntensity{1.0f};
            float m_Spare1{0.0f}; // padding
            float m_Spare2{0.0f}; // padding
            float m_Spare3{0.0f}; // padding

            // byte 64 to 128
            glm::vec4 m_Spare4[4];
        };

    public:
        PbrMaterial m_PbrMaterial;
        std::shared_ptr<MaterialDescriptor> m_MaterialDescriptor;
        MaterialTextures m_MaterialTextures;
    };

} // namespace GfxRenderEngine
