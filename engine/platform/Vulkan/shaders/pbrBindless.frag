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
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/material.h"

// pbrBindless.h contains the declartion of the types
// and the definition of buffers and push constants
#include "engine/platform/Vulkan/shaders/pbrBindless.h"

layout(set = 1, binding = 0) uniform sampler2D bindlessTextures[];

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

void main()
{
    MaterialBuffer material = MaterialBuffer(push.m_Constants.m_MaterialBuffer);

    // position
    outPosition = vec4(fragPosition, 1.0);

    // color
    vec4 col;
    if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_DIFFUSE_MAP)) 
    {
        col = texture(bindlessTextures[material.m_PbrMaterialProperties.m_DiffuseMap], fragUV) *
          material.m_PbrMaterialProperties.m_DiffuseColor;
    }
    else
    {
        col = fragColor;
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

    float normalMapIntensity  = material.m_PbrMaterialProperties.m_NormalMapIntensity;
    vec3 normalTangentSpace;
    if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_NORMAL_MAP))
    {
        normalTangentSpace = texture(bindlessTextures[material.m_PbrMaterialProperties.m_NormalMap],fragUV).xyz * 2 - vec3(1.0, 1.0, 1.0);
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
    if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP))
    {
        roughness = texture(bindlessTextures[material.m_PbrMaterialProperties.m_RoughnessMetallicMap], fragUV).g;
        metallic = texture(bindlessTextures[material.m_PbrMaterialProperties.m_RoughnessMetallicMap], fragUV).b;
    }
    else
    {
        if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_ROUGHNESS_MAP))
        {
            roughness = texture(bindlessTextures[material.m_PbrMaterialProperties.m_RoughnessMap], fragUV).r; // gray scale
        }
        else
        {
            roughness = material.m_PbrMaterialProperties.m_Roughness;
        }
        if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_METALLIC_MAP))
        {
            metallic = texture(bindlessTextures[material.m_PbrMaterialProperties.m_MetallicMap], fragUV).r; // gray scale
        }
        else
        {
            metallic = material.m_PbrMaterialProperties.m_Metallic;
        }
    }

    // clearcoat
    float clearcoatFactor;
    if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_EMISSIVE_MAP))
    {
        clearcoatFactor = texture(bindlessTextures[material.m_PbrMaterialProperties.m_ClearcoatMap], fragUV).r;
    }
    else
    {
        clearcoatFactor = material.m_PbrMaterialProperties.m_ClearcoatFactor;
    }
    float clearcoatRoughnessFactor = material.m_PbrMaterialProperties.m_ClearcoatRoughnessFactor;

    // material output to g-buffer
    outMaterial = vec4(clearcoatFactor, roughness, metallic, clearcoatRoughnessFactor);

    // emissive material
    vec4 emissiveColor = vec4(material.m_PbrMaterialProperties.m_EmissiveColor.r, material.m_PbrMaterialProperties.m_EmissiveColor.g, material.m_PbrMaterialProperties.m_EmissiveColor.b, 1.0);
    if (bool(material.m_PbrMaterialProperties.m_Features & GLSL_HAS_EMISSIVE_MAP))
    {
        vec4 fragEmissiveColor = texture(bindlessTextures[material.m_PbrMaterialProperties.m_EmissiveMap], fragUV);
        outEmissive = fragEmissiveColor * emissiveColor * material.m_PbrMaterialProperties.m_EmissiveStrength;
    }
    else
    {
        outEmissive = emissiveColor * material.m_PbrMaterialProperties.m_EmissiveStrength;
    }
}
