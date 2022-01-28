#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_ProjectionView;
    vec3 m_LightDirection;
} ubo;

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

layout(location = 0) out vec3 fragColor;

const float AMBIENT_LIGHT = 0.02;

void main()
{                  // projection * view * model * position
    gl_Position = ubo.m_ProjectionView * push.m_ModelMatrix * vec4(position, 1.0);

    vec3 normalWorldSpace = normalize(mat3(push.m_NormalMatrix) * normal);
    float lightIntensity = AMBIENT_LIGHT + max(dot(normalWorldSpace, ubo.m_LightDirection), 0);

    fragColor = color * lightIntensity;
}
