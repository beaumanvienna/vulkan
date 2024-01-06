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
//#include "renderer/renderer.h"

namespace GfxRenderEngine
{

    class MaterialDescriptor
    {

    public:

        enum MaterialType
        {
            MtPbrNoMap                                        = 0x1 << 0x00, // 1
            MtPbrEmissive                                     = 0x1 << 0x01, // 2
            MtPbrDiffuseMap                                   = 0x1 << 0x02, // 4
            MtPbrDiffuseSAMap                                 = 0x1 << 0x03, // 8
            MtPbrEmissiveTexture                              = 0x1 << 0x04, // 16
            MtPbrDiffuseNormalMap                             = 0x1 << 0x05, // 32
            MtPbrDiffuseNormalSAMap                           = 0x1 << 0x06, // 64
            MtPbrDiffuseNormalRoughnessMetallicMap            = 0x1 << 0x07, // 128
            MtPbrDiffuseNormalRoughnessMetallic2Map           = 0x1 << 0x08, // 256
            MtPbrDiffuseNormalRoughnessMetallicSAMap          = 0x1 << 0x09, // 512
            MtPbrDiffuseNormalRoughnessMetallicSA2Map         = 0x1 << 0x0a, // 1024
            MtCubemap                                         = 0x1 << 0x0b  // 2048
        };

        static constexpr uint ALL_PBR_MATERIALS = 
            MaterialType::MtPbrNoMap +
            MaterialType::MtPbrEmissive +
            MaterialType::MtPbrDiffuseMap +
            MaterialType::MtPbrDiffuseSAMap +
            MaterialType::MtPbrEmissiveTexture +
            MaterialType::MtPbrDiffuseNormalMap +
            MaterialType::MtPbrDiffuseNormalSAMap +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicMap +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallic2Map +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicSAMap +
            MaterialType::MtPbrDiffuseNormalRoughnessMetallicSA2Map;

    public:

        virtual ~MaterialDescriptor() = default;

        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType, std::vector<std::shared_ptr<Texture>>& textures);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType, std::vector<std::shared_ptr<Texture>>& textures, std::vector<std::shared_ptr<Buffer>>& buffers);
        static std::shared_ptr<MaterialDescriptor> Create(MaterialType materialType, std::shared_ptr<Cubemap> const& cubemap);

    public:

        virtual MaterialType GetMaterialType() const = 0;

    };
}