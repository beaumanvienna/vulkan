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
#include "renderer/buffer.h"

namespace GfxRenderEngine
{
    namespace Grass
    {
        struct GrassSpec
        {
            std::string m_FilepathGrassModel;
            std::string m_FilepathGrassMask;
            // base transform
            glm::vec3 m_Rotation;
            glm::vec3 m_Translation;
            glm::vec3 m_Scale;
            float m_ScaleXZ;
            float m_ScaleY;
        };

#pragma pack(push, 1)
        struct GrassShaderData1
        {
            int m_Height;
            int m_Index;
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct GrassShaderData2
        {
            glm::vec4 m_Translation;
            glm::vec4 m_Rotation;
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct GrassParameters
        {
            // byte 0 to 15
            int m_Width;
            int m_Height;
            float m_ScaleXZ;
            float m_ScaleY;

            // byte 16 to 23
            Buffer::BufferDeviceAddress m_GrassBufferDeviceAddress;
        }; // 24 bytes
#pragma pack(pop)
    } // namespace Grass
} // namespace GfxRenderEngine