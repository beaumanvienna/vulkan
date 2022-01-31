#version 450

layout(location = 0) in vec2 fragOffset;

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

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    vec4 m_Position;
    vec4 m_Color;
    float m_Radius;
} push;

void main()
{
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis > 1.0)
    {
        discard;
    }
    outColor = vec4(push.m_Color.xyz, 1.0);
}
