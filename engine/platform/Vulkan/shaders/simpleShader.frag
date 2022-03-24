#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) flat in int fragTextureSlot;
layout(location = 5) in float fragAmplification;
layout(location = 6) flat in int fragUnlit;
layout(location = 7) in vec3  toCameraDirection;

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

layout(set = 0, binding = 1) uniform sampler2D tex1;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

void main()
{

    // ---------- diffused ----------
    vec3 diffusedLightColor = ubo.m_AmbientLightColor.xyz * ubo.m_AmbientLightColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for (int i = 0; i < ubo.m_NumberOfActiveLights; i++)
    {
        PointLight light = ubo.m_PointLights[i];
        vec3 directionToLight = light.m_Position.xyz - fragPositionWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        float cosAngleOfIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.m_Color.xyz * light.m_Color.w * attenuation;
        diffusedLightColor += intensity * cosAngleOfIncidence;
    }

    // ---------- specular ----------
    // blinn phong: theta between N and H
    vec3 specularLightColor = vec3(0.0, 0.0, 0.0);

    //for (int i = 0; i < ubo.m_NumberOfActiveLights; i++)
    for (int i = 5; i < 6; i++)
    {
        PointLight light = ubo.m_PointLights[i];
        vec3 directionToLight     = light.m_Position.xyz - fragPositionWorld;
        float distanceToLight     = length(directionToLight);
        vec3 incidenceVector      = - normalize(directionToLight);
        vec3 directionToCamera    = normalize(toCameraDirection);
        vec3 reflectedLightDir    = reflect(incidenceVector, surfaceNormal);
        float specularFactor      = max(dot(reflectedLightDir, directionToCamera),0.0);
        float specularReflection  = pow(specularFactor, 8);
        float attenuation = 1.0 / (distanceToLight);
        vec3  intensity = light.m_Color.xyz * light.m_Color.w * attenuation;
        specularLightColor = intensity * specularReflection;
    }
    // ------------------------------

    vec3 pixelColor;
    float alpha = 1.0;
    vec3 lightColor = diffusedLightColor + specularLightColor;
    if (fragTextureSlot > 0)
    {
        // {0.0, 1.0} - {1.0, 1.0}
        // |        /            |
        // {0.0, 0.0} - {1.0, 0.0}

        if (fragTextureSlot == 1)
        {
            alpha = texture(tex1,fragUV).w;
            pixelColor = texture(tex1,fragUV).xyz;
        }
        if (alpha == 0) discard;
        if (fragUnlit != 0)
        {
            lightColor.xyz = vec3(1.0, 1.0, 1.0);
        }
        pixelColor *= fragAmplification;
    }
    else
    {
        pixelColor = fragColor.xyz;
    }

    outColor.xyz = lightColor.xyz  * pixelColor.xyz;
    outColor.w = alpha;

}
