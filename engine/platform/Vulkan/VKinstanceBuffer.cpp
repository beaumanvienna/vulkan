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

   Encapsulates a vulkan buffer
   Based on https://github.com/blurrypiano/littleVulkanEngine/blob/main/src/lve_buffer.cpp
   Initially based off VulkanBuffer by Sascha Willems -
   https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h */

#include "VKinstanceBuffer.h"

namespace GfxRenderEngine
{
    VK_InstanceBuffer::VK_InstanceBuffer(uint numInstances) : m_NumInstances(numInstances), m_Dirty{true}
    {
        m_Ubo = std::make_shared<VK_Buffer>(numInstances * sizeof(InstanceData),
                                            Buffer::BufferUsage::STORAGE_BUFFER_VISIBLE_TO_CPU);
        m_Ubo->MapBuffer();
        m_DataInstances.resize(numInstances);
    }

    VK_InstanceBuffer::~VK_InstanceBuffer() {}

    void VK_InstanceBuffer::SetInstanceData(uint index, glm::mat4 const& mat4Global, glm::mat4 const& normalMatrix)
    {
        CORE_ASSERT(index < m_NumInstances, "out of bounds");

        m_DataInstances[index].m_ModelMatrix = mat4Global;
        m_DataInstances[index].m_NormalMatrix = normalMatrix;

        m_Dirty = true;
    }

    void VK_InstanceBuffer::Update()
    {
        if (m_Dirty)
        {
            // update ubo
            m_Ubo->WriteToBuffer(m_DataInstances.data());
            m_Ubo->Flush();
            m_Dirty = false;
        }
    }

    std::shared_ptr<Buffer> VK_InstanceBuffer::GetBuffer() { return m_Ubo; }

    const glm::mat4& VK_InstanceBuffer::GetModelMatrix(uint index) { return m_DataInstances[index].m_ModelMatrix; }

    const glm::mat4& VK_InstanceBuffer::GetNormalMatrix(uint index) { return m_DataInstances[index].m_NormalMatrix; }
} // namespace GfxRenderEngine
