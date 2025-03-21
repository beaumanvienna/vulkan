// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

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
namespace JPH
{
    /// Defines how primitives should be rendered
    class PipelineState
    {
    public:
        /// Describes the input layout of the vertex shader
        enum class EInputDescription
        {
            Position,             ///< 3 float position
            Color,                ///< 4 uint8 color
            Normal,               ///< 3 float normal
            TexCoord,             ///< 2 float texture coordinate
            InstanceColor,        ///< 4 uint8 per instance color
            InstanceTransform,    ///< 4x4 float per instance transform
            InstanceInvTransform, ///< 4x4 float per instance inverse transform
        };

        /// In which draw pass to use this pipeline state
        enum class EDrawPass
        {
            Shadow,
            Normal
        };

        /// The type of topology to emit
        enum class ETopology
        {
            Triangle,
            Line
        };

        /// Fill mode of the triangles
        enum class EFillMode
        {
            Solid,
            Wireframe
        };

        /// If depth write / depth test is on
        enum class EDepthTest
        {
            Off,
            On
        };

        /// How to blend the pixel from the shader in the back buffer
        enum class EBlendMode
        {
            Write,
            AlphaBlend,
        };

        /// How to cull triangles
        enum class ECullMode
        {
            Backface,
            FrontFace,
        };

        /// Destructor
        virtual ~PipelineState() = default;

        /// Make this pipeline state active (any primitives rendered after this will use this state)
        virtual void Activate() = 0;
    };
} // namespace JPH