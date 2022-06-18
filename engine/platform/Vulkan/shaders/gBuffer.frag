/* Engine Copyright (c) 2022 Engine Development Team 
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
#define LIGHT_COUNT 10

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;

layout(location = 0)       in  vec3  fragColor;
layout(location = 1)       in  vec3  fragPositionWorld;
layout(location = 2)       in  vec3  fragNormalWorld;
layout(location = 3)       in  vec2  fragUV;
layout(location = 4)       in  float fragAmplification;
layout(location = 5)  flat in int    fragUnlit;
layout(location = 6)       in  vec3  fragTangentWorld;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outColor;

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
    PointLight m_PointLights[LIGHT_COUNT];
    int m_NumberOfActiveLights;
} ubo;

void main() 
{
    outPosition = vec4(fragPositionWorld, 1.0);

    // Calculate normal in tangent space
    vec3 N = normalize(fragNormalWorld);
    vec3 T = normalize(fragTangentWorld);

    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec3 surfaceNormalfromMap = TBN * normalize(texture(normalMap, fragUV).xyz * 2.0 - vec3(1.0));
    outNormal = vec4(surfaceNormalfromMap, 1.0);

    outColor = texture(diffuseMap, fragUV);
}