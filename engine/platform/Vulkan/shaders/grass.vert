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

struct BaseModelData
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
};

struct GrassShaderData
{
    int m_Height;
    int m_Index;
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

layout(set = 2, binding = 3) uniform ParameterBuffer
{
    int m_Width;
    int m_Height; // not used, 
    float m_ScaleXZ;
    float m_ScaleY;
} parameters;

layout(set = 2, binding = 0) readonly buffer InstanceUniformBuffer
{
    BaseModelData m_BaseModelData;
} baseTransform;

layout(set = 2, binding = 2) readonly buffer HeightMap
{
    GrassShaderData m_GrassShaderData[1]; // actual array size is larger than 1
} heightMap;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec4 fragColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out vec3 fragTangent;

void main()
{
    mat4 baseModelMatrix = baseTransform.m_BaseModelData.m_ModelMatrix;
    mat4 normalMatrix = baseTransform.m_BaseModelData.m_NormalMatrix;

    int index = heightMap.m_GrassShaderData[gl_InstanceIndex].m_Index;
    float hgt = heightMap.m_GrassShaderData[gl_InstanceIndex].m_Height;
    float row = floor(index / parameters.m_Width);
    float col = floor((index - parameters.m_Width * row));

    float theta = sin(hgt+gl_InstanceIndex); // random
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
        vec4(                   col,
                                hgt,
                                row,
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

    fragPosition = positionWorld.xyz;

    mat3 normalMatrixTransformed = transpose(inverse(mat3(baseModelMatrix) * mat3(localTransform)));
    fragNormal = normalize(normalMatrixTransformed * normal);
    fragTangent = normalize(normalMatrixTransformed * tangent);

    fragUV = uv;
    fragColor = color;
}
