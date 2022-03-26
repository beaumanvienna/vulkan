#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in int diffuseTextureSlot;
layout(location = 5) in float amplification;
layout(location = 6) in int unlit;
layout(location = 7) in int normalTextureSlot;
layout(location = 8) in vec3 tangent;
layout(location = 9) in vec3 bitangent;

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
layout(location = 4) out int   fragDiffuseTextureSlot;
layout(location = 5) out float fragAmplification;
layout(location = 6) out int   fragUnlit;
layout(location = 7) out vec3  toCameraDirection;
layout(location = 8) out int   fragNormalTextureSlot;

layout(location = 9) out vec3  fragTangentViewPos;
layout(location = 10) out vec3 fragTangentFragPos;
layout(location = 11) out vec3 fragTangentLightPos[10];

void main()
{
    // lighting
    vec4 positionWorld = push.m_ModelMatrix * vec4(position, 1.0);
    fragPositionWorld = positionWorld.xyz;
    fragMormalWorld = normalize(mat3(push.m_NormalMatrix) * normal);
    fragColor = color;
    fragDiffuseTextureSlot = diffuseTextureSlot;
    fragAmplification = amplification;
    fragUnlit = unlit;
    fragNormalTextureSlot = normalTextureSlot;

    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * push.m_ModelMatrix * vec4(position, 1.0);
    fragUV = uv;

    vec3 cameraPosWorld = (inverse(ubo.m_View) * vec4(0.0,0.0,0.0,1.0)).xyz;
    toCameraDirection = cameraPosWorld - positionWorld.xyz;

    // lights and camera in tangent space
    mat3 normalMatrix = transpose(inverse(mat3(push.m_ModelMatrix)));
    vec3 T = normalize(normalMatrix * tangent);
    vec3 N = normalize(normalMatrix * normal);

    // Gram Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    

    for (int i = 0; i < ubo.m_NumberOfActiveLights; i++)
    {
        PointLight light = ubo.m_PointLights[i];
        fragTangentLightPos[i] = TBN * light.m_Position.xyz;
    }
    fragTangentViewPos  = TBN * cameraPosWorld;
    fragTangentFragPos  = TBN * fragPositionWorld;

}
