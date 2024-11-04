/* Engine Copyright (c) 2024 Engine Development Team 

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

#include "engine/renderer/skeletalAnimation/joints.h"
#include "engine/platform/Vulkan/resource.h"

layout(location = 0) in vec3  position;
layout(location = 5) in ivec4 jointIds;
layout(location = 6) in vec4  weights;

struct InstanceData
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
};

layout(set = 0, binding = 0) uniform ShadowUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;
} ubo;

layout(set = 1, binding = 1) uniform SkeletalAnimationShaderData
{
    mat4 m_FinalJointsMatrices[MAX_JOINTS];
} skeletalAnimation;

layout(set = 1, binding = 0) readonly buffer InstanceUniformBuffer
{
    InstanceData m_InstanceData[MAX_INSTANCE];
} uboInstanced;

void main()
{
    vec4 animatedPosition = vec4(0.0f);
    mat4 jointTransform    = mat4(0.0f);
    for (int i = 0 ; i < MAX_JOINT_INFLUENCE ; i++)
    {
        if (weights[i] == 0)
            continue;
        if (jointIds[i] >=MAX_JOINTS) 
        {
            animatedPosition = vec4(position,1.0f);
            jointTransform   = mat4(1.0f);
            break;
        }
        vec4 localPosition  = skeletalAnimation.m_FinalJointsMatrices[jointIds[i]] * vec4(position,1.0f);
        animatedPosition   += localPosition * weights[i];
        jointTransform     += skeletalAnimation.m_FinalJointsMatrices[jointIds[i]] * weights[i];
    }

    // projection * view * model * position
    mat4 modelMatrix = uboInstanced.m_InstanceData[gl_InstanceIndex].m_ModelMatrix;
    vec4 positionWorld = modelMatrix * animatedPosition;
    gl_Position        = ubo.m_Projection * ubo.m_View * positionWorld;
}
