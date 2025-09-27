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
#include "engine/platform/Vulkan/shaders/pbr.h"

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

struct GrassShaderData2
{
    vec4 m_Translation;
    vec4 m_Rotation;
};

layout(buffer_reference, scalar) readonly buffer GrassBuffer
{
    GrassShaderData2 m_GrassShaderData[]; 
};

struct GrassParameters
{
    // byte 0 to 15
    int m_Width;
    int m_Height; // not used, 
    float m_ScaleXZ;
    float m_ScaleY;

    // byte 16 to 23
    BDA m_GrassBufferDeviceAddress;
}; // 24 bytes

struct DrawCallInfoGrass
{
    // Per mesh (never changes after mesh upload)
    // byte 0 to 7
    BDA m_MeshBufferDeviceAddress; // BDA to MeshBufferData struct

    // Per renderpass (water or main 3D pass)
    // byte 8 to 31
    VertexCtrl m_VertexCtrl;

    // Per submesh
    // byte 32 to 39
    BDA m_MaterialBuffer; // BDA to MaterialBuffer struct
    // byte 40 to 47
    SubmeshInfo m_SubmeshInfo;
    
    // byte 48 to 71
    GrassParameters m_GrassParameters;
};

layout(push_constant, scalar) uniform Push
{
    layout(offset = 0) DrawCallInfoGrass m_Constants;
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
    mat4 baseModelMatrix;
    mat4 normalMatrix;

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

        // Index into it using gl_InstanceIndex
        instanceData = instanceBuffer.m_Data[gl_InstanceIndex];

        baseModelMatrix = instanceData.m_ModelMatrix;
        normalMatrix    = instanceData.m_NormalMatrix;
    }

    GrassParameters parameters = push.m_Constants.m_GrassParameters;
    GrassBuffer grassMask = GrassBuffer(push.m_Constants.m_GrassParameters.m_GrassBufferDeviceAddress);

    vec3 rotVec3 = grassMask.m_GrassShaderData[gl_InstanceIndex].m_Rotation.xyz;
    vec3 translVec3 = grassMask.m_GrassShaderData[gl_InstanceIndex].m_Translation.xyz;

    float theta = sin(gl_InstanceIndex); // random
    float s = sin(theta); // sine
    float c = cos(theta); // cosine
    float sclXZ = parameters.m_ScaleXZ;
    float sclY = parameters.m_ScaleY;

    mat4 translation = mat4
    (
        vec4(    1.0,
                 0.0,   
                 0.0,
                 0.0                ), // first column
        vec4(         0.0,
                      1.0,
                      0.0,
                      0.0           ), // second column
        vec4(              0.0,
                           0.0,
                           1.0,
                           0.0      ), // third column
        vec4(                   translVec3.x,
                                translVec3.y,
                                translVec3.z,
                                1.0 )  // fourth column
    );
    
    mat4 rotation = mat4               // rotation around y-axsis
    (
        vec4(      c,
                 0.0,   
                  -s,
                 0.0                ), // first column
        vec4(         0.0,
                      1.0,
                      0.0,
                      0.0           ), // second column
        vec4(                s,
                           0.0,
                             c,
                           0.0      ), // third column
        vec4(                   0.0,
                                0.0,
                                0.0,
                                1.0 )  // fourth column
    );

    mat4 scale = mat4
    (
        vec4(  sclXZ,
                 0.0,
                 0.0,
                 0.0                ), // first column
        vec4(         0.0,
                     sclY,
                      0.0,
                      0.0           ), // second column
        vec4(              0.0,
                           0.0,
                         sclXZ,
                           0.0      ), // third column
        vec4(                   0.0,
                                0.0,
                                0.0,
                                1.0 )  // fourth column
    );

    mat4 localTransform = translation * rotation * scale;

    vec4 positionWorld = baseModelMatrix * localTransform * vec4(position, 1.0);
    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * positionWorld;
    if (bool(push.m_Constants.m_VertexCtrl.m_Features & GLSL_ENABLE_CLIPPING_PLANE))
    {
        gl_ClipDistance[0] = dot(positionWorld, push.m_Constants.m_VertexCtrl.m_ClippingPlane);
    }

    fragPosition = positionWorld.xyz;

    mat3 normalMatrixTransformed = transpose(inverse(mat3(baseModelMatrix) * mat3(localTransform)));
    fragNormal = normalize(normalMatrixTransformed * normal);
    fragTangent = normalize(normalMatrixTransformed * tangent);

    fragUV = uv;
    fragColor = color;
}
