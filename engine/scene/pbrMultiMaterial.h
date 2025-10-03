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

#include <memory>

#include "engine/scene/material.h"
#include "engine/scene/pbrMaterial.h"

namespace GfxRenderEngine
{
    class PbrMultiMaterial : public Material
    {
    public:
        static constexpr uint NUM_MULTI_MATERIAL = GLSL_NUM_MULTI_MATERIAL;

        struct PbrMultiMaterialTextures
        {
            PbrMaterial::MaterialTextures m_MaterialTextures[NUM_MULTI_MATERIAL];
        };
#pragma pack(push, 1)
        struct Parameters
        {
            glm::vec2 m_Vertical{};
            glm::vec2 m_Altitude{};
            glm::vec2 m_Lowness{};
        };
#pragma pack(pop)

    public:
        virtual ~PbrMultiMaterial() {}

        [[nodiscard]] virtual MaterialType GetType() const override { return MaterialType::MtPbrMulti; }

        [[nodiscard]] virtual Buffer::BufferDeviceAddress GetMaterialBufferDeviceAddress(uint index = 0) const override
        {
            return m_MaterialArray[index]->GetMaterialBufferDeviceAddress(index);
        }

        virtual std::shared_ptr<Buffer>& GetMaterialBuffer(uint index = 0) override
        {
            return m_MaterialArray[index]->GetMaterialBuffer();
        }

        virtual void SetMaterialDescriptor(std::shared_ptr<MaterialDescriptor> materialDescriptor, uint index = 0) override
        {
            m_MaterialArray[index]->SetMaterialDescriptor(materialDescriptor);
        }

        virtual std::shared_ptr<MaterialDescriptor>& GetMaterialDescriptor(uint index = 0) override
        {
            return m_MaterialArray[index]->GetMaterialDescriptor(index);
        }

        std::shared_ptr<PbrMaterial>& GetMaterial(uint index) { return m_MaterialArray[index]; };

    public:
        PbrMultiMaterialTextures m_PbrMultiMaterialTextures;

    private:
        std::array<std::shared_ptr<PbrMaterial>, NUM_MULTI_MATERIAL> m_MaterialArray;
    };

} // namespace GfxRenderEngine
