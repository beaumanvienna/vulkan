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

#include <Jolt/Geometry/Plane.h>
#include <Jolt/Geometry/AABox.h>

namespace JPH
{

    /// A camera frustum containing of 6 planes (left, right, top, bottom, near, far) pointing inwards
    class [[nodiscard]] Frustum
    {
    public:
        /// Empty constructor
        Frustum() = default;

        /// Construct frustum from position, forward, up, field of view x and y and near plane.
        /// Note that inUp does not need to be perpendicular to inForward but cannot be collinear.
        inline Frustum(Vec3Arg inPosition, Vec3Arg inForward, Vec3Arg inUp, float inFOVX, float inFOVY, float inNear)
        {
            Vec3 right = inForward.Cross(inUp).Normalized();
            Vec3 up =
                right.Cross(inForward)
                    .Normalized(); // Calculate the real up vector (inUp does not need to be perpendicular to inForward)

            // Near plane
            mPlanes[0] = Plane::sFromPointAndNormal(inPosition + inNear * inForward, inForward);

            // Top and bottom planes
            mPlanes[1] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(right, 0.5f * inFOVY) * -up);
            mPlanes[2] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(right, -0.5f * inFOVY) * up);

            // Left and right planes
            mPlanes[3] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(up, 0.5f * inFOVX) * right);
            mPlanes[4] = Plane::sFromPointAndNormal(inPosition, Mat44::sRotation(up, -0.5f * inFOVX) * -right);
        }

        /// Test if frustum overlaps with axis aligned box. Note that this is a conservative estimate and can return true if
        /// the frustum doesn't actually overlap with the box. This is because we only test the plane axis as separating axis
        /// and skip checking the cross products of the edges of the frustum
        inline bool Overlaps(const AABox& inBox) const
        {
            // Loop over all frustum planes
            for (const Plane& p : mPlanes)
            {
                // Get support point (the maximum extent) in the direction of our normal
                Vec3 support = inBox.GetSupport(p.GetNormal());

                // If this is behind our plane, the box is not inside the frustum
                if (p.SignedDistance(support) < 0.0f)
                    return false;
            }

            return true;
        }

    private:
        Plane mPlanes[5]; ///< Planes forming the frustum
    };
} // namespace JPH