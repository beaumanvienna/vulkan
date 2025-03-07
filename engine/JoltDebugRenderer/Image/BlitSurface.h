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

#include <Image/ZoomImage.h>
#include <Jolt/Core/Color.h>
namespace JPH
{
    /// Settings for blitting one surface to another with possibly different formats and dimensions. The blit
    /// routine can use filtering or blurring on the fly. Also it can perform some other
    /// basic operations like converting an image to grayscale or alpha only surfaces.
    class BlitSettings
    {
    public:
        /// Constructor
        BlitSettings();

        /// Comparison operators
        bool operator==(const BlitSettings& inRHS) const;

        /// Default settings
        static const BlitSettings sDefault;

        /// Special operations that can be applied during the blit
        bool mConvertRGBToAlpha;  ///< Convert RGB values to alpha values (RGB values remain untouched)
        bool mConvertAlphaToRGB;  ///< Convert alpha values to grayscale RGB values (Alpha values remain untouched)
        bool mConvertToGrayScale; ///< Convert RGB values to grayscale values (Alpha values remain untouched)
        bool mInvertAlpha;        ///< Invert alpha values
        bool mColorKeyAlpha; ///< If true, colors in the range mColorKeyStart..mColorKeyEnd will get an alpha of 0, other
                             ///< colors will get an alpha of 255
        Color mColorKeyStart;
        Color mColorKeyEnd;
        ZoomSettings mZoomSettings; ///< Settings for resizing the image
    };

    /// Copies an image from inSrc to inDst, converting it on the fly as defined by inBlitSettings
    bool BlitSurface(RefConst<Surface> inSrc, Ref<Surface> ioDst,
                     const BlitSettings& inBlitSettings = BlitSettings::sDefault);
} // namespace JPH