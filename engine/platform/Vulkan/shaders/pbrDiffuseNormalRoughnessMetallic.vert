/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/vulkan
   *
   * normalMapping: Blinn Phong lighting (ambient, diffuse, and specular with a texture map 
   *                and with a normal map
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

layout(location = 0) in vec3  position;
layout(location = 1) in vec3  color;
layout(location = 2) in vec3  normal;
layout(location = 3) in vec2  uv;
layout(location = 5) in float amplification;
layout(location = 6) in int   unlit;
layout(location = 7) in vec3  tangent;

struct PointLight
{
    vec4 m_Position;  // ignore w
    vec4 m_Color;     // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;

    // point light
    vec4 m_AmbientLightColor;
    PointLight m_PointLights[10];
    int m_NumberOfActiveLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMetallicMap;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

layout(location = 0)  out  vec3  fragColor;
layout(location = 1)  out  vec3  fragPositionWorld;
layout(location = 2)  out  vec3  fragNormalWorld;
layout(location = 3)  out  vec2  fragUV;
layout(location = 4)  out  float fragAmplification;
layout(location = 5)  out  int   fragUnlit;
layout(location = 6)  out  vec3  fragTangentViewPos;
layout(location = 7)  out  vec3  fragTangentFragPos;
layout(location = 8)  out  vec3  fragTangentLightPos[10];

void main()
{
    // lighting
    vec4 positionWorld = push.m_ModelMatrix * vec4(position, 1.0);
    fragPositionWorld = positionWorld.xyz;
    fragNormalWorld = normalize(mat3(push.m_NormalMatrix) * normal);
    fragColor = color;
    fragAmplification = amplification;
    fragUnlit = unlit;

    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * push.m_ModelMatrix * vec4(position, 1.0);
    fragUV = uv;

    vec3 cameraPosWorld = (inverse(ubo.m_View) * vec4(0.0,0.0,0.0,1.0)).xyz;

    // lights and camera in tangent space
    mat3 normalMatrix = transpose(inverse(mat3(push.m_ModelMatrix)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);

    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));

    for (int i = 0; i < ubo.m_NumberOfActiveLights; i++)
    {
        PointLight light = ubo.m_PointLights[i];
        fragTangentLightPos[i] = TBN * light.m_Position.xyz;
    }
    fragTangentViewPos  = TBN * cameraPosWorld;
    fragTangentFragPos  = TBN * fragPositionWorld;
}
