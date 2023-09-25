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

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v)
{
    return PowVec3(v, invGamma);
}

float sRGBToLuma(vec3 col)
{
    //return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
    return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

void main()
{
    vec2 srcTexelSize = 1.0 / push.m_SrcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = textureLod(emissiveMap, vec2(fragUV.x - 2*x, fragUV.y + 2*y),push.m_ImageViewID).rgb;
    vec3 b = textureLod(emissiveMap, vec2(fragUV.x,       fragUV.y + 2*y),push.m_ImageViewID).rgb;
    vec3 c = textureLod(emissiveMap, vec2(fragUV.x + 2*x, fragUV.y + 2*y),push.m_ImageViewID).rgb;
    
    vec3 d = textureLod(emissiveMap, vec2(fragUV.x - 2*x, fragUV.y),push.m_ImageViewID).rgb;
    vec3 e = textureLod(emissiveMap, vec2(fragUV.x,       fragUV.y),push.m_ImageViewID).rgb;
    vec3 f = textureLod(emissiveMap, vec2(fragUV.x + 2*x, fragUV.y),push.m_ImageViewID).rgb;
    
    vec3 g = textureLod(emissiveMap, vec2(fragUV.x - 2*x, fragUV.y - 2*y),push.m_ImageViewID).rgb;
    vec3 h = textureLod(emissiveMap, vec2(fragUV.x,       fragUV.y - 2*y),push.m_ImageViewID).rgb;
    vec3 i = textureLod(emissiveMap, vec2(fragUV.x + 2*x, fragUV.y - 2*y),push.m_ImageViewID).rgb;
    
    vec3 j = textureLod(emissiveMap, vec2(fragUV.x - x, fragUV.y + y),push.m_ImageViewID).rgb;
    vec3 k = textureLod(emissiveMap, vec2(fragUV.x + x, fragUV.y + y),push.m_ImageViewID).rgb;
    vec3 l = textureLod(emissiveMap, vec2(fragUV.x - x, fragUV.y - y),push.m_ImageViewID).rgb;
    vec3 m = textureLod(emissiveMap, vec2(fragUV.x + x, fragUV.y - y),push.m_ImageViewID).rgb;
    
    outEmissive.rgb = e*0.125;
    outEmissive.rgb += (a+c+g+i)*0.03125;
    outEmissive.rgb += (b+d+f+h)*0.0625;
    outEmissive.rgb += (j+k+l+m)*0.125;
    outEmissive = vec4(outEmissive.r, outEmissive.g, outEmissive.b, 1.0);
    //if (push.m_ImageViewID == 0) outEmissive = vec4(fragUV.x, 1.0, 1.0, 1.0);
    //if (push.m_ImageViewID == 1) outEmissive = vec4(fragUV.y, fragUV.y, fragUV.y, 1.0);
    //if (push.m_ImageViewID == 2) outEmissive = vec4(fragUV.x, fragUV.x, fragUV.x, 1.0);
    //outEmissive = textureLod(emissiveMap, fragUV, push.m_ImageViewID);
}