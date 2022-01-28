#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_ProjectionView;

    // point light
    vec4 m_AmbientLightColor;
    vec3 m_LightPosition;
    vec4 m_LightColor;
} ubo;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

layout(location = 0) out vec3 fragColor;

void main()
{
    // lighting
    vec4 positionWorld = push.m_ModelMatrix * vec4(position, 1.0);
    vec3 directionToLight = ubo.m_LightPosition - positionWorld.xyz;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);
    vec3 lightColor = ubo.m_LightColor.xyz * ubo.m_LightColor.w * attenuation;
    vec3 ambientLightColor = ubo.m_AmbientLightColor.xyz * ubo.m_AmbientLightColor.w;
    vec3 normalWorldSpace = normalize(mat3(push.m_NormalMatrix) * normal);
    vec3 diffusedLightColor = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);
    vec3 lightColorResult = (ambientLightColor + diffusedLightColor);

    // projection * view * model * position
    gl_Position = ubo.m_ProjectionView * push.m_ModelMatrix * vec4(position, 1.0);

    fragColor.x = lightColorResult.x * color.x;
    fragColor.y = lightColorResult.y * color.y;
    fragColor.z = lightColorResult.z * color.z;
}
