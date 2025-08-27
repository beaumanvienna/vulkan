/* Engine Copyright (c) 2025 Engine Development Team 
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

layout (location = 0) in vec3 vDirection;   // inPosition from cube vertices becomes view direction
layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2D uEquirectangularMap;

void main()
{
    // Normalize direction
    vec3 dir = normalize(vDirection);

    // Convert direction to spherical coords
    float phi = atan(dir.z, dir.x);          // longitude
    float theta = acos(clamp(dir.y, -1.0, 1.0)); // latitude

    // Map phi [-PI, PI] -> [0,1]
    float u = (phi / (2.0 * 3.14159265)) + 0.5;

    // Map theta [0, PI] -> [0,1]
    float v = theta / 3.14159265;

    vec3 color = texture(uEquirectangularMap, vec2(u, v)).rgb;

    outColor = vec4(color, 1.0);
}

