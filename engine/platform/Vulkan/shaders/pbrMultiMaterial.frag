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
#extension GL_EXT_scalar_block_layout : require
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
    float m_NormalMapIntensity;

    // byte 16 to 31
    vec4 m_DiffuseColor;

    // byte 32 to 47
    vec3 m_EmissiveColor;
    float m_EmissiveStrength;
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

layout(push_constant, scalar) uniform PushFragment
{
    layout(offset = 24) PbrMaterial m_PbrMaterial[GLSL_NUM_MULTI_MATERIAL];
} push;

void main()
{
    // position
    outPosition = vec4(fragPosition, 1.0);

    // normal
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec4 col[GLSL_NUM_MULTI_MATERIAL];
    vec4 normal[GLSL_NUM_MULTI_MATERIAL];
    vec4 material[GLSL_NUM_MULTI_MATERIAL];
    vec4 emissive[GLSL_NUM_MULTI_MATERIAL];

    float uvScale[GLSL_NUM_MULTI_MATERIAL] = {4.0, 1.0, 1.0, 4.0};

    for (int i = 0; i < GLSL_NUM_MULTI_MATERIAL; ++i)
    {
        vec2 uv = uvScale[i] * fragUV;
        // color
        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_DIFFUSE_MAP))
        {
            col[i] = texture(diffuseMap[i], uv) * push.m_PbrMaterial[i].m_DiffuseColor;
        }
        else
        {
            col[i] = vec4(fragColor.r, fragColor.g, fragColor.b, fragColor.a);
        }

        // normal
        float normalMapIntensity = push.m_PbrMaterial[i].m_NormalMapIntensity;
        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_NORMAL_MAP))
        {
            vec3 normalTangentSpace = texture(normalMap[i],uv).xyz * 2 - vec3(1.0, 1.0, 1.0);
            normalTangentSpace = mix(vec3(0.0, 0.0, 1.0), normalTangentSpace, normalMapIntensity);
            normal[i] = vec4(normalize(TBN * normalTangentSpace), 1.0);
        }
        else
        {
            normal[i] = vec4(N, 1.0);
        }

        // roughness, metallic
        float roughness;
        float metallic;

        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP))
        {
            roughness = texture(roughnessMetallicMap[i], uv).g;
            metallic = texture(roughnessMetallicMap[i], uv).b;
        }
        else
        {
            if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_ROUGHNESS_MAP))
            {
                roughness = texture(roughnessMap[i], uv).r; // gray scale
            }
            else
            {
                roughness = push.m_PbrMaterial[i].m_Roughness;
            }
            if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_METALLIC_MAP))
            {
                metallic = texture(metallicMap[i], uv).r; // gray scale
            }
            else
            {
                metallic = push.m_PbrMaterial[i].m_Metallic;
            }
        }
        material[i] = vec4(normalMapIntensity, roughness, metallic, 0.0);

        // emissive material
        vec4 emissiveColor = vec4(push.m_PbrMaterial[i].m_EmissiveColor.r, push.m_PbrMaterial[i].m_EmissiveColor.g, push.m_PbrMaterial[i].m_EmissiveColor.b, 1.0);
        if (bool(push.m_PbrMaterial[i].m_Features & GLSL_HAS_EMISSIVE_MAP))
        {
            vec4 fragEmissiveColor = texture(emissiveMap[i], uv);
            emissive[i] = fragEmissiveColor * emissiveColor * push.m_PbrMaterial[i].m_EmissiveStrength;
        }
        else
        {
            emissive[i] = emissiveColor * push.m_PbrMaterial[i].m_EmissiveStrength;
        }
    }

    float vertical;
    vertical = smoothstep(0.5, 0.6, N.y);

    float altitude;
    altitude = smoothstep(8.7, 9.0, fragPosition.y);
    
    
    float lowness;
    lowness = smoothstep(1.3, 1.6, fragPosition.y);

    // col
    outColor = mix(col[0], col[3], altitude);
    outColor = mix(col[2], outColor, lowness);
    outColor = mix(col[1], outColor, vertical);

    // normal
    outNormal = mix(normal[1], normal[0], vertical);
    outNormal = mix(outNormal, normal[3], altitude);

    // material
    outMaterial = mix(material[1], material[0], vertical);
    outMaterial = mix(outMaterial, material[3], altitude);

    // emissive
    outEmissive = mix(emissive[1], emissive[0], vertical);
    outEmissive = mix(outEmissive, emissive[3], altitude);
}
