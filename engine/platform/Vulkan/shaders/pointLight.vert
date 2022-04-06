#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

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

layout(location = 0) out vec2 fragOffset;

layout(push_constant) uniform Push
{
    vec4 m_Position;
    vec4 m_Color;
    float m_Radius;
} push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.m_View[0][0], ubo.m_View[1][0], ubo.m_View[2][0]};
    vec3 cameraUpWorld = {ubo.m_View[0][1], ubo.m_View[1][1], ubo.m_View[2][1]};

  vec3 positionWorld = push.m_Position.xyz
    + push.m_Radius * fragOffset.x * cameraRightWorld
    + push.m_Radius * fragOffset.y * cameraUpWorld;

  gl_Position = ubo.m_Projection * ubo.m_View * vec4(positionWorld, 1.0);
}
