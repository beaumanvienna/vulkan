/* Engine Copyright (c) 2023 Engine Development Team 
   https://github.com/beaumanvienna/vulkan

   Credits: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

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

layout(set = 1, binding = 0) uniform sampler2D emissiveMap;

layout(location = 0)  in  vec2  fragUV;
layout(location = 0) out  vec4  outEmissive;

layout(push_constant) uniform VK_PushConstantDataBloom
{
    vec2 m_SrcResolution;
    float m_FilterRadius;
    int m_ImageViewID;
} push;

void main()
{
    float x = push.m_FilterRadius;
    float y = push.m_FilterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = textureLod(emissiveMap, vec2(fragUV.x - x, fragUV.y + y),push.m_ImageViewID).rgb;
    vec3 b = textureLod(emissiveMap, vec2(fragUV.x,     fragUV.y + y),push.m_ImageViewID).rgb;
    vec3 c = textureLod(emissiveMap, vec2(fragUV.x + x, fragUV.y + y),push.m_ImageViewID).rgb;

    vec3 d = textureLod(emissiveMap, vec2(fragUV.x - x, fragUV.y),push.m_ImageViewID).rgb;
    vec3 e = textureLod(emissiveMap, vec2(fragUV.x,     fragUV.y),push.m_ImageViewID).rgb;
    vec3 f = textureLod(emissiveMap, vec2(fragUV.x + x, fragUV.y),push.m_ImageViewID).rgb;

    vec3 g = textureLod(emissiveMap, vec2(fragUV.x - x, fragUV.y - y),push.m_ImageViewID).rgb;
    vec3 h = textureLod(emissiveMap, vec2(fragUV.x,     fragUV.y - y),push.m_ImageViewID).rgb;
    vec3 i = textureLod(emissiveMap, vec2(fragUV.x + x, fragUV.y - y),push.m_ImageViewID).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    outEmissive.rgb = e*4.0;
    outEmissive.rgb += (b+d+f+h)*2.0;
    outEmissive.rgb += (a+c+g+i);
    outEmissive.rgb *= 1.0 / 16.0;

}