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

layout(location = 0)       in vec3  fragColor;
layout(location = 1)       in vec3  fragPositionWorld;
layout(location = 2)       in vec3  fragNormalWorld;
layout(location = 3)       in vec2  fragUV;
layout(location = 4)       in float fragAmplification;
layout(location = 5)  flat in int   fragUnlit;
layout(location = 6)       in vec3  fragTangentViewPos;
layout(location = 7)       in vec3  fragTangentFragPos;
layout(location = 8)       in vec3  fragTangentLightPos[10];

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

layout(set = 1, binding = 0) uniform sampler2D diffuseMap; // diffuse map
layout(set = 1, binding = 1) uniform sampler2D normalMap;  // normal map

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    float roughness           = push.m_NormalMatrix[3].x;
    float metallic            = push.m_NormalMatrix[3].y;
    float normalMapIntensity  = push.m_NormalMatrix[3].z;

    vec3 ambientLightColor = ubo.m_AmbientLightColor.xyz * ubo.m_AmbientLightColor.w;

    // ---------- lighting ----------
    vec3 diffusedLightColor = vec3(0.0);
    vec3 surfaceNormal;

    vec3 toCameraDirection = fragTangentViewPos - fragTangentFragPos;

    // blinn phong: theta between N and H
    vec3 specularLightColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < ubo.m_NumberOfActiveLights; i++)
    {
        PointLight light = ubo.m_PointLights[i];

        // normal in tangent space
        vec3 surfaceNormalfromMap = normalize(texture(normalMap,fragUV).xyz * 2 - vec3(1.0, 1.0, 1.0));
        surfaceNormal             = mix(vec3(0.0, 0.0, 1.0), surfaceNormalfromMap, normalMapIntensity);
        vec3 directionToLight     = fragTangentLightPos[i] - fragTangentFragPos;
        float distanceToLight     = length(directionToLight);
        float attenuation = 1.0 / (distanceToLight * distanceToLight);
        
        // ---------- diffused ----------
        float cosAngleOfIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0.0);
        vec3 intensity = light.m_Color.xyz * light.m_Color.w * attenuation;
        diffusedLightColor += intensity * cosAngleOfIncidence;
        
        // ---------- specular ----------
        if (cosAngleOfIncidence != 0.0)
        {
            vec3 incidenceVector      = - normalize(directionToLight);
            vec3 directionToCamera    = normalize(toCameraDirection);
            vec3 reflectedLightDir    = reflect(incidenceVector, surfaceNormal);
        
            // phong
            //float specularFactor      = max(dot(reflectedLightDir, directionToCamera),0.0);
            // blinn phong
            vec3 halfwayDirection     = normalize(-incidenceVector + directionToCamera);
            float specularFactor      = max(dot(surfaceNormal, halfwayDirection),0.0);
        
            float specularReflection  = pow(specularFactor, roughness);
            vec3  intensity = light.m_Color.xyz * light.m_Color.w * attenuation;
            specularLightColor += intensity * specularReflection;
        }
    }
    // ------------------------------

    vec3 pixelColor;
    float alpha = 1.0;

    alpha = texture(diffuseMap,fragUV).w;
    pixelColor = texture(diffuseMap,fragUV).xyz;
    if (alpha < 0.0001) discard;
    pixelColor *= fragAmplification;

    outColor.xyz = ambientLightColor*pixelColor.xyz + (diffusedLightColor  * pixelColor.xyz) + specularLightColor;

    // reinhard tone mapping
//    outColor.xyz = outColor.xyz / (outColor.xyz + vec3(1.0));
outColor.xyz = vec3(1.0);
    outColor.w = alpha;

}
