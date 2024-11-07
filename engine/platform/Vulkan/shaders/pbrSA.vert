/* Engine Copyright (c) 2024 Engine Development Team 
   https://github.com/beaumanvienna/vulkan
   *
   * PBR rendering; parts of this code are based on https://learnopengl.com/PBR/Lighting
   *

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
#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/resource.h"

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 jointIds;
layout(location = 6) in vec4 weights;

struct PointLight
{
    vec4 m_Position; // ignore w
    vec4 m_Color;    // w is intensity
};

struct DirectionalLight
{
    vec4 m_Direction; // ignore w
    vec4 m_Color;     // w is intensity
};

struct InstanceData
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;

    // point light
    vec4 m_AmbientLightColor;
    PointLight m_PointLights[MAX_LIGHTS];
    DirectionalLight m_DirectionalLight;
    int m_NumberOfActivePointLights;
    int m_NumberOfActiveDirectionalLights;
} ubo;

layout(set = 2, binding = 0) readonly buffer InstanceUniformBuffer
{
    InstanceData m_InstanceData[MAX_INSTANCE];
} uboInstanced;

layout(set = 2, binding = 1) uniform SkeletalAnimationShaderData
{
    mat4 m_FinalJointsMatrices[MAX_JOINTS];
} skeletalAnimation;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

void main()
{
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

    mat4 modelMatrix = uboInstanced.m_InstanceData[gl_InstanceIndex].m_ModelMatrix;
    mat4 instanceNormalMatrix = uboInstanced.m_InstanceData[gl_InstanceIndex].m_NormalMatrix;

    // projection * view * model * position
    vec4 positionWorld = modelMatrix * animatedPosition;
    gl_Position = ubo.m_Projection * ubo.m_View * positionWorld;
    fragPosition = positionWorld.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix) * mat3(jointTransform)));
    fragNormal = normalize(normalMatrix * normal);
    fragTangent = normalize(normalMatrix * tangent);

    fragUV = uv;
    fragColor = color;
}
