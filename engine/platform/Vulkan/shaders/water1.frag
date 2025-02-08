/* Engine Copyright (c) 2025 Engine Development Team 
   *

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

layout(location = 0)      in  vec4 clipSpace;
layout (location = 0)     out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D refractionTexture;
layout(set = 1, binding = 1) uniform sampler2D reflectionTexture;

void main()
{
    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
    vec2 ndc_flipped = vec2(ndc.x, 1.0 - ndc.y);
    vec4 refraction = texture(refractionTexture, ndc);
    vec4 reflection = texture(reflectionTexture, ndc_flipped);
    outColor = mix(refraction, reflection, 0.2);
    //outColor = refraction;
    //outColor = reflection;
}
