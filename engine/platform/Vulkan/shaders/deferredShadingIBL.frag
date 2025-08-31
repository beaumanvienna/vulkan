/* Engine Copyright (c) 2025 Engine Development Team 
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

#version 460

#include "engine/platform/Vulkan/pointlights.h"
#include "engine/platform/Vulkan/shadowMapping.h"
#include "engine/platform/Vulkan/shader.h"

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput positionMap;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput normalMap;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput diffuseMap;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput roughnessMetallicMap;

layout(location = 0) out vec4 outColor;

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

layout(set = 2, binding = 0) uniform sampler2DShadow shadowMapTextureHiRes;
layout(set = 2, binding = 1) uniform sampler2DShadow shadowMapTextureLowRes;
layout(set = 2, binding = 2) uniform ShadowUniformBuffer0
{
    mat4 m_Projection;
    mat4 m_View;
} lightUboHiRes;

layout(set = 2, binding = 3) uniform ShadowUniformBuffer1
{
    mat4 m_Projection;
    mat4 m_View;
} lightUboLowRes;

layout(push_constant, std430) uniform VK_PushConstantsIBL
{
    // x: uMaxPrefilterMip, number of mips - 1, use as push.m_Values.x
    // y: float exposure
    // z: reserve
    // w: reserve
    vec4 m_Values0;
    
    // x: shader settings 0 (see shader.h)
    // y: reserve
    // z: reserve
    // w: reserve
    ivec4 m_Values1;
} push;

// IBL - envPrefilteredDiffuse
layout(set = 3, binding = 0) uniform sampler2D uIrradianceMap; // pre-baked diffuse (lat-long)

// IBL - envPrefilteredSpecular
layout(set = 3, binding = 1) uniform sampler2D uPrefilteredEnv; // prefiltered specular as MIP-mapped lat-long

// IBL - BRDFIntegrationMap
layout(set = 3, binding = 2) uniform sampler2D uBRDFLUT;        // 2D BRDF LUT

// =======================================
// Helpers
// =======================================
const float PI = 3.141592653589793;
const float EPSILON = 1e-6;

// Direction -> lat-long UV in [0,1]^2
vec2 DirToLatLongUV(vec3 dir)
{
    dir = normalize(dir);
    float phi   = atan(dir.z, dir.x);            // -PI..PI
    float theta = acos(clamp(dir.y, -1.0, 1.0)); // 0..PI
    float u = phi / (2.0 * PI) + 0.5;            // wrap in U
    float v = theta / PI;                        // clamp in V
    return vec2(u, v);
}

// Fresnel (roughness-aware)
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Sample irradiance from a lat-long 2D texture (already convolved offline)
vec3 SampleIrradiance(vec3 N)
{
    vec2 uv = DirToLatLongUV(N);
    return texture(uIrradianceMap, uv).rgb;
}

// Sample prefiltered specular from a MIP-mapped lat-long 2D texture
vec3 SamplePrefilteredEnv(vec3 R, float roughness)
{
    vec2 uv = DirToLatLongUV(R);
    float maxMip = push.m_Values0.x;               // numMips - 1
    float lod    = roughness * maxMip;            // linear mapping
    return textureLod(uPrefilteredEnv, uv, lod).rgb;
}

// Compute specular IBL using prefiltered env + BRDF LUT (split-sum)
vec3 ComputeSpecularIBL(vec3 N, vec3 V, vec3 F0, float roughness)
{
    float NoV = max(dot(N, V), 0.0);
    vec3  R   = reflect(-V, N);

    vec3 prefiltered = SamplePrefilteredEnv(R, roughness);
    vec2 brdf        = texture(uBRDFLUT, vec2(NoV, roughness)).rg;
    vec3 F           = FresnelSchlickRoughness(NoV, F0, roughness);

    // split-sum: prefiltered * (F * brdf.x + brdf.y)
    return prefiltered * (F * brdf.x + brdf.y);
}

// ACES filmic tonemap
vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

// tone mapping
vec3 ACESFilm(vec3 x)
{
    // Input transform to ACES color space
    const mat3 ACESInputMat = mat3(
         0.59719, 0.35458, 0.04823,
         0.07600, 0.90834, 0.01566,
         0.02840, 0.13383, 0.83777
    );

    // Output transform back to sRGB
    const mat3 ACESOutputMat = mat3(
         1.60475, -0.53108, -0.07367,
        -0.10208,  1.10813, -0.00605,
        -0.00327, -0.07276,  1.07602
    );

    x = ACESInputMat * x;

    // Apply curve
    x = RRTAndODTFit(x);

    x = ACESOutputMat * x;

    // Clamp to displayable range
    return clamp(x, 0.0, 1.0);
}

vec3 ACESFilmFromLukas(vec3 color) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((color*(a*color+b))/(color*(c*color+d)+e), 0.0, 1.0);
}

// =======================================
// Main (deferred lighting resolve)
// =======================================
void main()
{
    // G-buffer fetch
    vec3 fragPosition = subpassLoad(positionMap).rgb;
    vec3 N            = normalize(subpassLoad(normalMap).rgb);
    vec3 albedo       = subpassLoad(diffuseMap).rgb;
    vec4 material     = subpassLoad(roughnessMetallicMap);
    float roughness   = material.g;
    float metallic    = material.b;

    // View
    vec3 camPos = (inverse(ubo.m_View) * vec4(0.0,0.0,0.0,1.0)).xyz;
    vec3 V = normalize(camPos - fragPosition); // viewing vector

    // Base reflectivity
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Diffuse IBL (already convolved)
    vec3 irradiance = SampleIrradiance(N);
    vec3 diffuse    = irradiance * albedo * (1.0 - metallic);

    // Specular IBL
    vec3 specular = ComputeSpecularIBL(N, V, F0, roughness);

    // Combine
    vec3 color = diffuse + specular;
    
    // Apply exposure before tonemapping
    float exposure = push.m_Values0.y;
    if (exposure > EPSILON)
    {
        color *= exposure;
    }

    // Apply ACES tonemapper
    if ((push.m_Values1.x & SHADER_SETTINGS0_USE_NEW_ACES) != 0)
    {
        color = ACESFilm(color);
    }
    else
    {
        color = ACESFilmFromLukas(color);
    }

    // Gamma correction is not required because we are using an SRGB color attachment. 
    // The shader is expected to write the color in linear space. 
    // The color will be converted into gamma space automatically.

    if ((push.m_Values1.x & SHADER_SETTINGS0_DO_NOT_MULTIPLY_COLOR_OUT_WITH_ALBEDO) != 0)
    {
        outColor = vec4(color, 1.0);
    }
    else
    {
        outColor = vec4(albedo, 1.0) * vec4(color, 1.0);
    }
}