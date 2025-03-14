#version 450

#include "engine/JoltDebugRenderer/Shaders/VK/VertexConstants.h"

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec4 iColor;

layout(location = 0) out vec4 oColor;

layout(push_constant, std430) uniform Push
{
    mat4 m_Projection;
    mat4 m_View;
} push;

void main() 
{
    gl_Position = push.m_Projection * push.m_View * vec4(iPosition, 1.0);
    oColor = iColor;
}
