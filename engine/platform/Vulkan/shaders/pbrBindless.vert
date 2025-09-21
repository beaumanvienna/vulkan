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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#version 450
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/resource.h"
#include "engine/platform/Vulkan/shader.h"

// pbrBindless.h contains the declartion of the types
// and the definition of buffers and push constants
#include "engine/platform/Vulkan/shaders/pbrBindless.h"

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

void main()
{
    // mesh buffer has BDAs for vertex, index, and instance buffers
    MeshBuffer mesh = MeshBuffer(push.m_Constants.m_MeshBufferDeviceAddress);

    // Index lookup
    IndexBuffer indexBuffer = IndexBuffer(mesh.m_Data.m_IndexBufferDeviceAddress);
    uint index = indexBuffer.m_Indices[push.m_Constants.m_SubmeshInfo.m_FirstIndex + gl_VertexIndex];
    index += push.m_Constants.m_SubmeshInfo.m_VertexOffset;

    // Vertex fetch
    VertexBuffer vertexBuffer = VertexBuffer(mesh.m_Data.m_VertexBufferDeviceAddress);
    Vertex vertex = vertexBuffer.m_Vertices[index];

    vec3  position = vertex.m_Position;
    vec4  color    = vertex.m_Color;
    vec3  normal   = vertex.m_Normal;
    vec2  uv       = vertex.m_UV;
    vec3  tangent  = vertex.m_Tangent;
    ivec4 jointIds = vertex.m_JointIds;
    vec4  weights  = vertex.m_Weights;

    // Create a reference to the buffer from the BDA
    InstanceBuffer instanceBuffer = InstanceBuffer(mesh.m_Data.m_InstanceBufferDeviceAddress);

    // Index into it using gl_InstanceIndex
    InstanceData instanceData = instanceBuffer.m_Data[gl_InstanceIndex];

    mat4 modelMatrix  = instanceData.m_ModelMatrix;
    mat4 normalMatrix = instanceData.m_NormalMatrix;

    // projection * view * model * position
    vec4 positionWorld = modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.m_Projection * ubo.m_View * positionWorld;
    if (bool(push.m_Constants.m_VertexCtrl.m_Features & GLSL_ENABLE_CLIPPING_PLANE))
    {
        gl_ClipDistance[0] = dot(positionWorld, push.m_Constants.m_VertexCtrl.m_ClippingPlane);
    }

    fragPosition = positionWorld.xyz;
    fragNormal = mat3(normalMatrix) * normal;
    fragTangent = mat3(normalMatrix) * tangent;

    fragUV = uv;
    fragColor = color;
}
