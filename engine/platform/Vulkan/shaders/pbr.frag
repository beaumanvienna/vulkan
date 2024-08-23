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
#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/material.h"

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMetallicMap;
layout(set = 1, binding = 3) uniform sampler2D emissiveMap;
layout(set = 1, binding = 4) uniform sampler2D roughnessMap;
layout(set = 1, binding = 5) uniform sampler2D metallicMap;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec3 fragTangent;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outColor;
layout (location = 3) out vec4 outMaterial;
layout (location = 4) out vec4 outEmissive;

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

layout(push_constant) uniform Push
{
    int m_Features;
    float m_Roughness;
    float m_Metallic;
    float m_Spare0; // padding

    // byte 16 to 31
    vec4 m_DiffuseColor;

    // byte 32 to 47
    vec3 m_EmissiveColor;
    float m_EmissiveStrength;

    // byte 48 to 63
    float m_NormalMapIntensity;
    float m_Spare1; // padding
    float m_Spare2; // padding
    float m_Spare3; // padding

    // byte 64 to 128
    vec4 m_Spare4[4];
} push;

void main()
{
    // position
    outPosition = vec4(fragPosition, 1.0);

    // color
    vec4 col;
    if (bool(push.m_Features & GLSL_HAS_DIFFUSE_MAP))
    {
        col = texture(diffuseMap, fragUV) * push.m_DiffuseColor;
    }
    else
    {
        col = vec4(fragColor.r, fragColor.g, fragColor.b, fragColor.a);
    }
    if (col.a < 0.5)
    {
        discard;
    }
    outColor = col;
    
    // normal
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    float normalMapIntensity  = push.m_NormalMapIntensity;
    vec3 normalTangentSpace;
    if (bool(push.m_Features & GLSL_HAS_NORMAL_MAP))
    {
        normalTangentSpace = texture(normalMap,fragUV).xyz * 2 - vec3(1.0, 1.0, 1.0);
        normalTangentSpace = mix(vec3(0.0, 0.0, 1.0), normalTangentSpace, normalMapIntensity);
        outNormal = vec4(normalize(TBN * normalTangentSpace), 1.0);
    }
    else
    {
        outNormal = vec4(N, 1.0);
    }
    
    // roughness, metallic
    float roughness;
    float metallic;
    if (bool(push.m_Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP))
    {
        roughness = texture(roughnessMetallicMap, fragUV).g;
        metallic = texture(roughnessMetallicMap, fragUV).b;
    }
    else
    {
        if (bool(push.m_Features & GLSL_HAS_ROUGHNESS_MAP))
        {
            roughness = texture(roughnessMap, fragUV).r; // gray scale
        }
        else
        {
            roughness = push.m_Roughness;
        }
        if (bool(push.m_Features & GLSL_HAS_METALLIC_MAP))
        {
            metallic = texture(metallicMap, fragUV).r; // gray scale
        }
        else
        {
            metallic = push.m_Metallic;
        }
    }
    outMaterial = vec4(normalMapIntensity, roughness, metallic, 0.0);

    // emissive material
    vec4 emissiveColor = vec4(push.m_EmissiveColor.r, push.m_EmissiveColor.g, push.m_EmissiveColor.b, 1.0);
    if (bool(push.m_Features & GLSL_HAS_EMISSIVE_MAP))
    {
        vec4 fragEmissiveColor = texture(emissiveMap, fragUV);        
        outEmissive = fragEmissiveColor * emissiveColor * push.m_EmissiveStrength;
    }
    else
    {
        outEmissive = emissiveColor * push.m_EmissiveStrength;
    }
}
