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

layout(location = 0) in vec2 fragOffset;

struct PointLight
{
    vec4 m_Position;  // ignore w
    vec4 m_Color;     // w is intensity
};

struct DirectionalLight
{
    vec4 m_Direction;  // ignore w
    vec4 m_Color;     // w is intensity
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

layout (location = 0) out vec4 outColor;

layout(push_constant, std430) uniform Push
{
    vec4 m_Position;
    vec4 m_ColorRadius; // w is radius
} push;

void main()
{
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis > 1.0)
    {
        discard;
    }
    float oneMinusDisSqr = (1 - dis) * (1 - dis);
    vec3 bloom = vec3(oneMinusDisSqr, oneMinusDisSqr, oneMinusDisSqr);
    float alpha = smoothstep(0.1, 1.0, 1-dis);
    outColor = vec4(push.m_ColorRadius.xyz + bloom, alpha);
}
