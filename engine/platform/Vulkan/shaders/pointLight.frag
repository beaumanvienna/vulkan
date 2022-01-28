#version 450

layout(location = 0) in vec2 fragOffset;

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

void main()
{
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis > 1.0)
    {
        discard;
    }
    outColor = vec4(ubo.m_LightColor.xyz, 1.0);
}
