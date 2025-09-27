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
#include "engine/platform/Vulkan/shaders/pbr.h"

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

struct DrawCallInfoMultiMaterial
{
    // Per mesh (never changes after mesh upload)
    // byte 0 to 7
    BDA m_MeshBufferDeviceAddress; // BDA to MeshBufferData struct

    // Per renderpass (water or main 3D pass)
    // byte 8 to 31
    VertexCtrl m_VertexCtrl;

    // Per submesh
    // byte 32 to 63
    BDA m_MaterialBuffer[GLSL_NUM_MULTI_MATERIAL /*4*/]; // BDAs to MaterialBuffer structs
    // byte 64 to 71
    SubmeshInfo m_SubmeshInfo;
};

layout(push_constant, scalar) uniform Push
{
    layout(offset = 0) DrawCallInfoMultiMaterial m_Constants;
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
        MaterialBuffer inMaterial = MaterialBuffer(push.m_Constants.m_MaterialBuffer[i]);
        PbrMaterialProperties pbrMaterialProperties = inMaterial.m_PbrMaterialProperties;

        vec2 uv = uvScale[i] * fragUV;
        // color
        if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_DIFFUSE_MAP))
        {
            col[i] = texture(bindlessTextures[pbrMaterialProperties.m_DiffuseMap], uv) * pbrMaterialProperties.m_DiffuseColor;
        }
        else
        {
            col[i] = fragColor;
        }

        // normal
        float normalMapIntensity = pbrMaterialProperties.m_NormalMapIntensity;
        if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_NORMAL_MAP))
        {
            vec3 normalTangentSpace = texture(bindlessTextures[pbrMaterialProperties.m_NormalMap],uv).xyz * 2 - vec3(1.0, 1.0, 1.0);
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

        if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_ROUGHNESS_METALLIC_MAP))
        {
            roughness = texture(bindlessTextures[pbrMaterialProperties.m_RoughnessMetallicMap], uv).g;
            metallic = texture(bindlessTextures[pbrMaterialProperties.m_RoughnessMetallicMap], uv).b;
        }
        else
        {
            if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_ROUGHNESS_MAP))
            {
                roughness = texture(bindlessTextures[pbrMaterialProperties.m_RoughnessMap], uv).r; // gray scale
            }
            else
            {
                roughness = pbrMaterialProperties.m_Roughness;
            }
            if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_METALLIC_MAP))
            {
                metallic = texture(bindlessTextures[pbrMaterialProperties.m_MetallicMap], uv).r; // gray scale
            }
            else
            {
                metallic = pbrMaterialProperties.m_Metallic;
            }
        }
        material[i] = vec4(normalMapIntensity, roughness, metallic, 0.0);

        // emissive material
        vec4 emissiveColor = vec4(pbrMaterialProperties.m_EmissiveColor, 1.0);
        if (bool(pbrMaterialProperties.m_Features & GLSL_HAS_EMISSIVE_MAP))
        {
            vec4 fragEmissiveColor = texture(bindlessTextures[pbrMaterialProperties.m_EmissiveMap], uv);
            emissive[i] = fragEmissiveColor * emissiveColor * pbrMaterialProperties.m_EmissiveStrength;
        }
        else
        {
            emissive[i] = emissiveColor * pbrMaterialProperties.m_EmissiveStrength;
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
