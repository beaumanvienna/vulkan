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

#include <memory>

#include "engine.h"
#include "renderer/buffer.h"
#include "renderer/texture.h"
#include "renderer/cubemap.h"
// #include "renderer/renderer.h"

namespace GfxRenderEngine
{

    class MaterialDescriptor
    {
    public:
        enum MaterialType
        {
            MtPbrNoMap = 0x1 << 0x00,                                         // 1
            MtPbrEmissive = 0x1 << 0x01,                                      // 2
            MtPbrDiffuseMap = 0x1 << 0x02,                                    // 4
            MtPbrDiffuseSAMap = 0x1 << 0x03,                                  // 8
            MtPbrNoMapInstanced = 0x1 << 0x04,                                // 16
            MtPbrEmissiveTexture = 0x1 << 0x05,                               // 32
            MtPbrDiffuseNormalMap = 0x1 << 0x06,                              // 64
            MtPbrEmissiveInstanced = 0x1 << 0x07,                             // 128
            MtPbrDiffuseNormalSAMap = 0x1 << 0x08,                            // 256
            MtPbrDiffuseMapInstanced = 0x1 << 0x09,                           // 512
            MtPbrDiffuseSAMapInstanced = 0x1 << 0x0a,                         // 1024
            MtPbrEmissiveTextureInstanced = 0x1 << 0x0b,                      // 2048
            MtPbrDiffuseNormalMapInstanced = 0x1 << 0x0c,                     // 4096
            MtPbrDiffuseNormalRoughnessSAMap = 0x1 << 0x0d,                   // 8192
            MtPbrDiffuseNormalSAMapInstanced = 0x1 << 0x0e,                   // 16384
            MtPbrDiffuseNormalRoughnessMetallicMap = 0x1 << 0x0f,             // 32768
            MtPbrDiffuseNormalRoughnessMetallic2Map = 0x1 << 0x10,            // 65536
            MtPbrDiffuseNormalRoughnessMetallicSAMap = 0x1 << 0x11,           // 131072
            MtPbrDiffuseNormalRoughnessSAMapInstanced = 0x1 << 0x12,          // 262144
            MtPbrDiffuseNormalRoughnessMetallicSA2Map = 0x1 << 0x13,          // 524288
            MtPbrDiffuseNormalRoughnessMetallicMapInstanced = 0x1 << 0x14,    // 1048576
            MtPbrDiffuseNormalRoughnessMetallic2MapInstanced = 0x1 << 0x15,   // 2097152
            MtPbrDiffuseNormalRoughnessMetallicSAMapInstanced = 0x1 << 0x16,  // 4194304
            MtPbrDiffuseNormalRoughnessMetallicSA2MapInstanced = 0x1 << 0x17, // 8388608
            MtCubemap = 0x1 << 0x18                                           // 16777216
        };

        static constexpr uint ALL_PBR_MATERIALS =
            MaterialType::MtPbrNoMap + MaterialType::MtPbrEmissive + MaterialType::MtPbrDiffuseMap +
            MaterialType::MtPbrDiffuseSAMap + MaterialType::MtPbrNoMapInstanced + MaterialType::MtPbrEmissiveTexture +
            MaterialType::MtPbrDiffuseNormalMap + MaterialType::MtPbrEmissiveInstanced +
            MaterialType::MtPbrDiffuseNormalSAMap + MaterialType::MtPbrDiffuseMapInstanced +
            MaterialType::MtPbrDiffuseSAMapInstanced + MaterialType::MtPbrEmissiveTextureInstanced +
            MaterialType::MtPbrDiffuseNormalMapInstanced + MaterialType::MtPbrDiffuseNormalSAMapInstanced +
            MaterialType::MtPbrDiffuseNormalRoughnessSAMap + MaterialType::MtPbrDiffuseNormalRoughnessSAMapInstanced +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicMap + MaterialType::MtPbrDiffuseNormalRoughnessMetallic2Map +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicSAMap +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicSA2Map +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicMapInstanced;

    public:
        virtual ~MaterialDescriptor() = default;

        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType,
                                                          std::vector<std::shared_ptr<Buffer>>& buffers);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType,
                                                          std::vector<std::shared_ptr<Texture>>& textures);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType,
                                                          std::vector<std::shared_ptr<Texture>>& textures,
                                                          std::vector<std::shared_ptr<Buffer>>& buffers);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType,
                                                          std::shared_ptr<Cubemap> const& cubemap);

    public:
        virtual MaterialType GetMaterialType() const = 0;
    };
} // namespace GfxRenderEngine
