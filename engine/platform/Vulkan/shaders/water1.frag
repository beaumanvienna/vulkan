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

// in 
layout(location = 0)      in  vec4 clipSpace;
layout(location = 1)      in  vec2  fragUV;

// out
layout(location = 0)      out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D refractionTexture;
layout(set = 1, binding = 1) uniform sampler2D reflectionTexture;
layout(set = 2, binding = 0) uniform sampler2D duDvMap;

layout(push_constant, std430) uniform Push
{
    mat4 m_ModelMatrix;
    vec4 m_Values;
} push;

const float waveStrength = 0.02;

void main()
{
    // UVs for refraction and reflection textures
    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
    vec2 ndc_flipped = vec2(ndc.x, 1.0 - ndc.y);

    // du dv map
    float moveFactor = push.m_Values.x;
    vec2 distortion1 = (texture(duDvMap, vec2(fragUV.x + moveFactor, fragUV.y)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion2 = (texture(duDvMap, vec2(-fragUV.x + moveFactor, fragUV.y + moveFactor)).rg * 2.0 - 1.0) * waveStrength;
    vec2 distortion = distortion1 + distortion2;
    ndc += distortion1;
    ndc = clamp(ndc, 0.001, 0.999);
    ndc_flipped += distortion1;
    ndc_flipped = clamp(ndc_flipped, 0.001, 0.999);

    vec4 refraction = texture(refractionTexture, ndc);
    vec4 reflection = texture(reflectionTexture, ndc_flipped);
    outColor = mix(refraction, reflection, 0.2);
    vec4 blue = vec4(0.0, 0.3, 0.5, 1.0);
    outColor = mix(outColor, blue, 0.05);
}
