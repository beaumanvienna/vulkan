/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/vulkan

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

// inputs 
layout(push_constant) uniform Push
{
    mat4 m_Mat4;
    vec2 m_UV[2];
} push;

// outputs
layout(location = 0) out vec2  fragUV;
layout(location = 1) out vec4  fragColor;
layout(location = 2) out float textureID;

// 0 - 1
// | / |
// 3 - 2

// Mat4
// x1, y1, red,       green,
// x2, y1, blue,      alpha,
// x2, y2, cntxtWdth, cntxtHght,
// x1, y2, 1.0f,      1.0f

//positions
// 0
// 1
// 3

// 1
// 2
// 3

void main()
{
    float red   = push.m_Mat4[2][0];
    float green = push.m_Mat4[3][0];
    float blue  = push.m_Mat4[2][1];
    float alpha = push.m_Mat4[3][1];
    fragColor = vec4(red, green, blue, alpha);

    float contextWidthHalf  = push.m_Mat4[2][2]/2.0;
    float contextHeightHalf = push.m_Mat4[2][3]/2.0;
    float x,y;

    switch (gl_VertexIndex)
    {
        case 0:
            fragUV = vec2(push.m_UV[0].x, push.m_UV[0].y);
            x = push.m_Mat4[0][0];
            y = push.m_Mat4[1][0];
            break;
        case 1:
            fragUV = vec2(push.m_UV[1].x, push.m_UV[0].y);
            x = push.m_Mat4[0][1];
            y = push.m_Mat4[1][1];
            break;
        case 2:
            fragUV = vec2(push.m_UV[0].x, push.m_UV[1].y);
            x = push.m_Mat4[0][3];
            y = push.m_Mat4[1][3];
            break;
        case 3:
            fragUV = vec2(push.m_UV[1].x, push.m_UV[0].y);
            x = push.m_Mat4[0][1];
            y = push.m_Mat4[1][1];
            break;
        case 4:
            fragUV = vec2(push.m_UV[1].x, push.m_UV[1].y);
            x = push.m_Mat4[0][2];
            y = push.m_Mat4[1][2];
            break;
        case 5:
            fragUV = vec2(push.m_UV[0].x, push.m_UV[1].y);
            x = push.m_Mat4[0][3];
            y = push.m_Mat4[1][3];
            break;
    }
    float xNorm = (x - contextWidthHalf) / contextWidthHalf;
    float yNorm = (y - contextHeightHalf) / contextHeightHalf;
    gl_Position = vec4(xNorm, yNorm, 0.0, 1.0);

    textureID = push.m_Mat4[3][2];
}
