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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "engine.h"
#include "engine/platform/Vulkan/shader.h"
#include "scene/grass.h"

namespace GfxRenderEngine
{

    enum ControlFeatures // bitset
    {
        ENABLE_CLIPPING_PLANE = GLSL_ENABLE_CLIPPING_PLANE
    };
#pragma pack(push, 1)
    struct VertexCtrl
    {
        // byte 0 to 15
        glm::vec4 m_ClippingPlane{0.0f};

        // byte 16 to 23
        uint m_Features{0};
        int m_Reserve0{0};
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct SubmeshInfo
    {
        // byte 0 to 7
        uint m_FirstIndex{0};
        int m_VertexOffset{0};
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct MeshBufferData
    {
        // byte 0 to 31
        Buffer::BufferDeviceAddress m_VertexBufferDeviceAddress{0};
        Buffer::BufferDeviceAddress m_IndexBufferDeviceAddress{0};
        Buffer::BufferDeviceAddress m_InstanceBufferDeviceAddress{0};
        Buffer::BufferDeviceAddress m_SkeletalAnimationBufferDeviceAddress{0};
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct DrawCallInfo
    {
        // per mesh (never changes after mesh upload)
        // byte 0 to 7
        Buffer::BufferDeviceAddress m_MeshBufferDeviceAddress;

        // byte 8 to 31
        VertexCtrl m_VertexCtrl; // per render pass (water or main 3D pass)

        // per submesh
        // byte 32 to 39
        Buffer::BufferDeviceAddress m_MaterialBufferDeviceAddress;
        // byte 40 to 47
        SubmeshInfo m_SubmeshInfo;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct DrawCallInfoMultiMaterial
    {
        // per mesh (never changes after mesh upload)
        // byte 0 to 7
        Buffer::BufferDeviceAddress m_MeshBufferDeviceAddress;

        // byte 8 to 31
        VertexCtrl m_VertexCtrl; // per render pass (water or main 3D pass)

        // per submesh
        // byte 32 to 63
        std::array<Buffer::BufferDeviceAddress, PbrMultiMaterial::NUM_MULTI_MATERIAL> m_MaterialBufferDeviceAddresses;
        // byte 64 to 71
        SubmeshInfo m_SubmeshInfo;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct DrawCallInfoGrass
    {
        // per mesh (never changes after mesh upload)
        // byte 0 to 7
        Buffer::BufferDeviceAddress m_MeshBufferDeviceAddress;

        // byte 8 to 31
        VertexCtrl m_VertexCtrl; // per render pass (water or main 3D pass)

        // per submesh
        // byte 32 to 39
        Buffer::BufferDeviceAddress m_MaterialBufferDeviceAddress;
        // byte 40 to 47
        SubmeshInfo m_SubmeshInfo;

        // byte 48 to 71
        Grass::GrassParameters m_GrassParameters;
    };
#pragma pack(pop)
} // namespace GfxRenderEngine
