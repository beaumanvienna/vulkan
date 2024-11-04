/* Engine Copyright (c) 2024 Engine Development Team 

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#version 450
#include "engine/platform/Vulkan/resource.h"

layout(location = 0) in vec3  position;

layout(set = 0, binding = 0) uniform ShadowUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;
} ubo;

struct InstanceData
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
};

layout(set = 1, binding = 0) readonly buffer InstanceUniformBuffer
{
    InstanceData m_InstanceData[MAX_INSTANCE];
} uboInstanced;

void main()
{
    mat4 modelMatrix = uboInstanced.m_InstanceData[gl_InstanceIndex].m_ModelMatrix;

    // projection * view * model * position
    gl_Position = ubo.m_Projection * ubo.m_View * modelMatrix * vec4(position, 1.0);
}
