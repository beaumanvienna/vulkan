/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/vulkan
   *
   * PBR rendering; parts of this code are based on https://learnopengl.com/PBR/Lighting
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
#define LIGHT_COUNT 10
#define AMBIENT 0.9

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput positionMap;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput normalMap;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput diffuseMap;

layout (location = 0) out vec4 outColor;

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
    PointLight m_PointLights[LIGHT_COUNT];
    int m_NumberOfActiveLights;
} ubo;



void main() 
{
    // retrieve G buffer data
    vec3 fragPos = subpassLoad(positionMap).rgb;
    vec3 normal = subpassLoad(normalMap).rgb;
    vec4 diffuseColor = subpassLoad(diffuseMap);
    
    // Ambient part
    vec3 fragcolor  = diffuseColor.rgb * AMBIENT;
    
    for(int i = 0; i < LIGHT_COUNT; ++i)
    {
        // vector to light
        vec3 L = ubo.m_PointLights[i].m_Position.xyz - fragPos;
        // distance from light to fragment position
        float dist = length(L);

        // Viewer to fragment
        //vec3 V = ubo..xyz - fragPos;
        //V = normalize(V);
        //
        ////if(dist < ubo.lights[i].radius)
        //{
        //    // Light to fragment
        //    L = normalize(L);
        //
        //    // Attenuation
        //    float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);
        //
        //    // Diffuse part
        //    vec3 N = normalize(normal);
        //    float NdotL = max(0.0, dot(N, L));
        //    vec3 diff = ubo.lights[i].color * diffuseColor.rgb * NdotL * atten;
        //
        //    // Specular part
        //    // Specular map values are stored in alpha of albedo mrt
        //    vec3 R = reflect(-L, N);
        //    float NdotR = max(0.0, dot(R, V));
        //    vec3 spec = ubo.lights[i].color * diffuseColor.a * pow(NdotR, 16.0) * atten;
        //
        //    fragcolor += diff + spec;    
        //}    
    }        
//    outColor = vec4(fragcolor, 1.0);

//outColor = vec4(0.1, 0.8, 0.2, 1.0);
//outColor = vec4(fragPos, 1.0);
//outColor = vec4(normal, 1.0);
outColor = vec4(diffuseColor);
}