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
#include "engine/renderer/skeletalAnimation/joints.h"
#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/resource.h"
#include "engine/platform/Vulkan/shader.h"

// pbrBindless.h contains the declartion of the types
// and the definition of buffers and push constants
#include "engine/platform/Vulkan/shaders/pbr.h"

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

layout(buffer_reference, scalar) readonly buffer SkeletalAnimationShaderData
{
    mat4 m_FinalJointsMatrices[];
};

layout(push_constant, scalar) uniform Push
{
    layout(offset = 0) DrawCallInfo m_Constants;
} push;

void main()
{
    
    vec3  position;
    vec4  color;
    vec3  normal;
    vec2  uv;
    vec3  tangent;
    ivec4 jointIds;
    vec4  weights;
    InstanceBuffer instanceBuffer;
    InstanceData instanceData;
    SkeletalAnimationShaderData skeletalAnimation;
    mat4 modelMatrix;

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

        position = vertex.m_Position;
        color    = vertex.m_Color;
        normal   = vertex.m_Normal;
        uv       = vertex.m_UV;
        tangent  = vertex.m_Tangent;
        jointIds = vertex.m_JointIds;
        weights  = vertex.m_Weights;

        // Create a reference to the buffer from the BDA
        instanceBuffer = InstanceBuffer(mesh.m_Data.m_InstanceBufferDeviceAddress);
        skeletalAnimation = SkeletalAnimationShaderData(mesh.m_Data.m_SkeletalAnimationBufferDeviceAddress);

        // Index into it using gl_InstanceIndex
        instanceData = instanceBuffer.m_Data[gl_InstanceIndex];

        modelMatrix  = instanceData.m_ModelMatrix;
    }
    
    vec4 animatedPosition = vec4(0.0f);
    mat4 jointTransform = mat4(0.0f);
    for (int i = 0; i < MAX_JOINT_INFLUENCE; ++i)
    {
        if (weights[i] == 0)
            continue;
        if (jointIds[i] >=MAX_JOINTS) 
        {
            animatedPosition = vec4(position,1.0f);
            jointTransform = mat4(1.0f);
            break;
        }
        vec4 localPosition  = skeletalAnimation.m_FinalJointsMatrices[jointIds[i]] * vec4(position,1.0f);
        animatedPosition += localPosition * weights[i];
        jointTransform += skeletalAnimation.m_FinalJointsMatrices[jointIds[i]] * weights[i];
    }

    // projection * view * model * position
    vec4 positionWorld = modelMatrix * animatedPosition;
    gl_Position = ubo.m_Projection * ubo.m_View * positionWorld;
    if (bool(push.m_Constants.m_VertexCtrl.m_Features & GLSL_ENABLE_CLIPPING_PLANE))
    {
        gl_ClipDistance[0] = dot(positionWorld, push.m_Constants.m_VertexCtrl.m_ClippingPlane);
    }

    fragPosition = positionWorld.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix) * mat3(jointTransform)));
    fragNormal = normalize(normalMatrix * normal);
    fragTangent = normalize(normalMatrix * tangent);

    fragUV = uv;
    fragColor = color;
}
