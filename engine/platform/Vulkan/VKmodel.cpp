/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include "VKmodel.h"

// Vertex
std::vector<VkVertexInputBindingDescription> VK_Model::VK_Vertex::GetBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride  = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VK_Model::VK_Vertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription>  attributeDescriptions(2);

    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);//sizeof(Vertex::position);

    return attributeDescriptions;
}

// VK_Model
VK_Model::VK_Model(std::shared_ptr<VK_Device> device, const std::vector<Vertex>& vertices)
    : m_Device(device)
{
    CreateVertexBuffers(vertices);
}

VK_Model::~VK_Model()
{
    vkDestroyBuffer(m_Device->Device(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device->Device(), m_VertexBufferMemory, nullptr);
}

void VK_Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
    m_VertexCount = static_cast<uint>(vertices.size());
    VkDeviceSize bufferSize = sizeof(Vertex) * m_VertexCount;

    m_Device->CreateBuffer
    (
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_VertexBuffer,
        m_VertexBufferMemory
    );

    void* data;
    vkMapMemory
    (
        m_Device->Device(),
        m_VertexBufferMemory,
        0,
        bufferSize,
        0,
        &data
    );
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device->Device(), m_VertexBufferMemory);
}

void VK_Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_VertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VK_Model::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw
    (
        commandBuffer,
        m_VertexCount,
        1,
        0,
        0
    );
}
