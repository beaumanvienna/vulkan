#version 450

layout(location = 0) in vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_ProjectionView;

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
    outColor = vec4(fragColor, 1.0f);
}
