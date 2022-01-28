#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPositionWorld;
layout(location = 2) in vec3 fragMormalWorld;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;

    // point light
    vec4 m_AmbientLightColor;
    vec3 m_LightPosition;
    vec4 m_LightColor;
} ubo;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

void main()
{

    vec3 directionToLight = ubo.m_LightPosition - fragPositionWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);
    vec3 lightColor = ubo.m_LightColor.xyz * ubo.m_LightColor.w * attenuation;
    vec3 ambientLightColor = ubo.m_AmbientLightColor.xyz * ubo.m_AmbientLightColor.w;
    vec3 diffusedLightColor = lightColor * max(dot(normalize(fragMormalWorld), normalize(directionToLight)), 0);
    vec3 lightColorResult = (ambientLightColor + diffusedLightColor);

    outColor.x = lightColorResult.x * fragColor.x;
    outColor.y = lightColorResult.y * fragColor.y;
    outColor.z = lightColorResult.z * fragColor.z;
    outColor.w = 1.0;

}
