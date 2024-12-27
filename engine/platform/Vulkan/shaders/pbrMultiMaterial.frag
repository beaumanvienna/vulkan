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

layout(set = 1, binding = 0) uniform sampler2D diffuseMap[GLSL_NUM_MULTI_MATERIAL];
layout(set = 1, binding = 1) uniform sampler2D normalMap[GLSL_NUM_MULTI_MATERIAL];
layout(set = 1, binding = 2) uniform sampler2D roughnessMetallicMap[GLSL_NUM_MULTI_MATERIAL];
layout(set = 1, binding = 3) uniform sampler2D emissiveMap[GLSL_NUM_MULTI_MATERIAL];
layout(set = 1, binding = 4) uniform sampler2D roughnessMap[GLSL_NUM_MULTI_MATERIAL];
layout(set = 1, binding = 5) uniform sampler2D metallicMap[GLSL_NUM_MULTI_MATERIAL];

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

struct PbrMaterial
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
}

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
    PbrMaterial m_PbrMaterial[GLSL_NUM_MULTI_MATERIAL];
} push;

void main()
{
    float contribution[4];
    contribution[0] = controlMap.r;
    contribution[1] = controlMap.g;
    contribution[2] = controlMap.b;
    contribution[3] = controlMap.a;
    
    // position
    outPosition = vec4(fragPosition, 1.0);
    outColor = vec4(0.0, 0.0, 0.0, 0.0);
    outNormal = vec4(0.0, 0.0, 1.0, 1.0);
    
    // normal
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    
    for (int i = 0; i < GLSL_NUM_MULTI_MATERIAL; ++i)
    {
        // color
        vec4 col;
        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_DIFFUSE_MAP))
        {
            col = texture(diffuseMap[i], fragUV) * push.m_PbrMaterial[i].m_DiffuseColor;
        }
        else
        {
            col = vec4(fragColor.r, fragColor.g, fragColor.b, fragColor.a);
        }
        outColor = outColor + col * contribution[i];

        // normal
        vec4 normal;
        float normalMapIntensity = push.m_PbrMaterial[i].m_NormalMapIntensity;
        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_NORMAL_MAP))
        {
            vec3 normalTangentSpace = texture(normalMap[i],fragUV).xyz * 2 - vec3(1.0, 1.0, 1.0);
            normalTangentSpace = mix(vec3(0.0, 0.0, 1.0), normalTangentSpace, normalMapIntensity);
            normal = vec4(normalize(TBN * normalTangentSpace), 1.0);
        }
        else
        {
            normal = vec4(N, 1.0);
        }
        outNormal = mix(outNormal, normal, contribution[i];

        // roughness, metallic
        float roughness;
        float metallic;

        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP))
        {
            roughness = texture(roughnessMetallicMap, fragUV).g;
            metallic = texture(roughnessMetallicMap, fragUV).b;
        }
        else
        {
            if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_ROUGHNESS_MAP))
            {
                roughness = texture(roughnessMap, fragUV).r; // gray scale
            }
            else
            {
                roughness = push.m_PbrMaterial[i].m_Roughness;
            }
            if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_METALLIC_MAP))
            {
                metallic = texture(metallicMap, fragUV).r; // gray scale
            }
            else
            {
                metallic = push.m_PbrMaterial[i].m_Metallic;
            }
        }
        outMaterial = vec4(normalMapIntensity, roughness, metallic, 0.0);
    }

    
    
    
    

    

    // emissive material
    vec4 emissiveColor = vec4(push.m_PbrMaterial[i].m_EmissiveColor.r, push.m_PbrMaterial[i].m_EmissiveColor.g, push.m_PbrMaterial[i].m_EmissiveColor.b, 1.0);
    if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_EMISSIVE_MAP))
    {
        vec4 fragEmissiveColor = texture(emissiveMap, fragUV);        
        outEmissive = fragEmissiveColor * emissiveColor * push.m_PbrMaterial[i].m_EmissiveStrength;
    }
    else
    {
        outEmissive = emissiveColor * push.m_PbrMaterial[i].m_EmissiveStrength;
    }
}
