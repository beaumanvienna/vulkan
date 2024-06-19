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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#version 450
#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/resource.h"

layout(location = 0) in vec3  position;
layout(location = 1) in vec4  color;
layout(location = 2) in vec3  normal;
layout(location = 3) in vec2  uv;
layout(location = 4) in vec3  tangent;

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

layout(set = 2, binding = 0) uniform InstanceUniformBuffer
{
    InstanceData m_InstanceData;
} baseTransform;

#define WIDTH 512 // row
#define HEIGHT 375 // col
#define NUM_HEIGHT_VALUES WIDTH*HEIGHT // 768000
#define INSTANCE_COUNT NUM_HEIGHT_VALUES

layout(set = 2, binding = 2) readonly buffer HeightMap
{
    int m_HeightMapData[INSTANCE_COUNT]; 
} heightMap;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

void main()
{
    mat4 baseModelMatrix = baseTransform.m_InstanceData.m_ModelMatrix;
    mat4 normalMatrix = baseTransform.m_InstanceData.m_NormalMatrix;

    int index = gl_InstanceIndex;
    int hgt = heightMap.m_HeightMapData[index];
    float row = floor(index / WIDTH);
    float col = floor((index - WIDTH * row));

    float theta = sin(hgt+gl_InstanceIndex); // random
    float s = sin(theta); // sine
    float c = cos(theta); // cosine
    mat4 localTransform = mat4
    (
        vec4(5.0 * c,       s,   0.0, 0.0), // first column
        vec4(     -s, 5.0 * c,   0.0, 0.0), // second column
        vec4(    0.0,     0.0,   5.0, 0.0), // third column
        vec4(    col,      row, -hgt, 1.0)  // fourth column
    );

    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * baseModelMatrix * localTransform * vec4(position, 1.0);

    vec4 positionWorld = baseModelMatrix * vec4(position, 1.0);
    fragPosition = positionWorld.xyz;
    fragNormal = mat3(normalMatrix) * normal;
    fragTangent = mat3(normalMatrix) * tangent;

    fragUV = uv;
    fragColor = color;
}
