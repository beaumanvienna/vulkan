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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Encapsulates a vulkan buffer to hold the model and normal matrix for instanced rendering
*/

#pragma once

#include <vulkan/vulkan.h>

#include "engine.h"
#include "renderer/instanceBuffer.h"

#include "VKbuffer.h"

namespace GfxRenderEngine
{
    class VK_InstanceBuffer : public InstanceBuffer
    {

    public:
        VK_InstanceBuffer(uint numInstances);
        virtual ~VK_InstanceBuffer();

        VK_InstanceBuffer(const VK_InstanceBuffer&) = delete;
        VK_InstanceBuffer& operator=(const VK_InstanceBuffer&) = delete;

        virtual void SetInstanceData(uint index, glm::mat4 const& mat4Global, glm::mat4 const& normalMatrix) override;
        virtual const glm::mat4& GetModelMatrix(uint index) override;
        virtual const glm::mat4& GetNormalMatrix(uint index) override;
        virtual std::shared_ptr<Buffer> GetBuffer() override;
        void Update();

    private:
        struct InstanceData
        {
            glm::mat4 m_ModelMatrix;
            glm::mat4 m_NormalMatrix;
        };

        uint m_NumInstances;
        bool m_Dirty;
        std::vector<InstanceData> m_DataInstances;
        std::shared_ptr<VK_Buffer> m_Ubo;
    };
} // namespace GfxRenderEngine
