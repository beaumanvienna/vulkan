#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;

    // point light
    vec4 m_AmbientLightColor;
    vec3 m_LightPosition;
    vec4 m_LightColor;
} ubo;

layout(location = 0) out vec2 fragOffset;

const float LIGHT_RADIUS = 0.1;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.m_View[0][0], ubo.m_View[1][0], ubo.m_View[2][0]};
    vec3 cameraUpWorld = {ubo.m_View[0][1], ubo.m_View[1][1], ubo.m_View[2][1]};

  vec3 positionWorld = ubo.m_LightPosition
    + LIGHT_RADIUS * fragOffset.x * cameraRightWorld
    + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

  gl_Position = ubo.m_Projection * ubo.m_View * vec4(positionWorld, 1.0);
}
