#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in int textureSlot;
layout(location = 5) in float amplification;
layout(location = 6) in int unlit;

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

layout(push_constant) uniform Push
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
} push;

layout(location = 0) out vec3  fragColor;
layout(location = 1) out vec3  fragPositionWorld;
layout(location = 2) out vec3  fragMormalWorld;
layout(location = 3) out vec2  fragUV;
layout(location = 4) out int   fragTextureSlot;
layout(location = 5) out float fragAmplification;
layout(location = 6) out int   fragUnlit;

void main()
{
    // lighting
    vec4 positionWorld = push.m_ModelMatrix * vec4(position, 1.0);
    fragPositionWorld = positionWorld.xyz;
    fragMormalWorld = normalize(mat3(push.m_NormalMatrix) * normal);
    fragColor = color;
    fragTextureSlot = textureSlot;
    fragAmplification = amplification;
    fragUnlit = unlit;

    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * push.m_ModelMatrix * vec4(position, 1.0);
    fragUV = uv;
}
