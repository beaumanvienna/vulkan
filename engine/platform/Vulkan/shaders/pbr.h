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

#define BDA uint64_t // buffer device address

struct Vertex
{
    vec3 m_Position;
    vec4 m_Color;
    vec3 m_Normal;
    vec2 m_UV;
    vec3 m_Tangent;
    ivec4 m_JointIds;
    vec4 m_Weights;
};

struct PointLight
{
    vec4 m_Position; // ignore w
    vec4 m_Color;    // w is intensity
};

struct DirectionalLight
{
    vec4 m_Direction; // ignore w
    vec4 m_Color;     // w is intensity
};

struct InstanceData
{
    mat4 m_ModelMatrix;
    mat4 m_NormalMatrix;
};

struct VertexCtrl
{
    // byte 0 to 15
    vec4 m_ClippingPlane;

    // byte 16 to 23
    uint m_Features;
    int m_Reserve0;
};

struct  SubmeshInfo
{
    // byte 0 yo 7
    uint m_FirstIndex;
    int  m_VertexOffset;
};

struct MeshBufferData
{
    // byte 0 to 31
    BDA m_VertexBufferDeviceAddress;
    BDA m_IndexBufferDeviceAddress;
    BDA m_InstanceBufferDeviceAddress;
    BDA m_SkeletalAnimationBufferDeviceAddress;
};

struct PbrMaterialProperties
{ 
    int m_Features;
    float m_Roughness;
    float m_Metallic;
    float m_NormalMapIntensity;

    // byte 16 to 31
    vec4 m_DiffuseColor;

    // byte 32 to 47
    vec3 m_EmissiveColor;
    float m_EmissiveStrength;

    // byte 48 to 63
    float m_ClearcoatFactor;
    float m_ClearcoatRoughnessFactor;
    uint m_DiffuseMap;
    uint m_NormalMap;

    // byte 64 to 79
    uint m_RoughnessMap;
    uint m_MetallicMap;
    uint m_RoughnessMetallicMap;
    uint m_EmissiveMap;
    
    // byte 80 to 87
    uint m_ClearcoatMap;
    uint m_Reserve0;
};

struct DrawCallInfo
{
    // Per mesh (never changes after mesh upload)
    // byte 0 to 7
    BDA m_MeshBufferDeviceAddress; // BDA to MeshBufferData struct

    // Per renderpass (water or main 3D pass)
    // byte 8 to 31
    VertexCtrl m_VertexCtrl;

    // Per submesh
    // byte 32 to 39
    BDA m_MaterialBuffer; // BDA to MaterialBuffer struct
    // byte 40 to 47
    SubmeshInfo m_SubmeshInfo;
};

layout(buffer_reference, scalar) readonly buffer VertexBuffer
{
    Vertex m_Vertices[];
};

layout(buffer_reference, scalar) readonly buffer IndexBuffer
{
    uint m_Indices[];
};

layout(buffer_reference, scalar) readonly buffer InstanceBuffer
{
    InstanceData m_Data[];
};

layout(buffer_reference, scalar) readonly buffer MeshBuffer
{
    MeshBufferData m_Data;
};

layout(buffer_reference, scalar) readonly buffer MaterialBuffer
{
    PbrMaterialProperties m_PbrMaterialProperties;
};

layout(set = 0, binding = 0) uniform GlobalUniformBuffer
{
    mat4 m_Projection;
    mat4 m_View;

    // point light
    vec4 m_AmbientLightColor;
    PointLight m_PointLights[MAX_LIGHTS];
    DirectionalLight m_DirectionalLight;
    int m_NumberOfActivePointLights;
    int m_NumberOfActiveDirectionalLights;
} ubo;

