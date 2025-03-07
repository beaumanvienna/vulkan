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

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include "core.h"
namespace JPH
{

    CameraState::CameraState()
        : mPos(RVec3::sZero()), mForward(0, 0, -1), mUp(0, 1, 0), //
          mFOVY(DegreesToRadians(70.0f))
    {
    }

    CameraState::CameraState(GfxRenderEngine::Camera const& cam0)
    {
        { // RVec3 mPos;    ///< Camera position
            const glm::vec3& pos = cam0.GetPosition();
            mPos = RVec3{pos.x, pos.y, pos.z};
        }
        { // Vec3 mForward; ///< Camera forward vector
            const glm::vec3& rot = cam0.GetRotation();
            mForward = Vec3{rot.x, rot.y, rot.z};
        }
        { // Vec3 mUp;      ///< Camera up vector
            mUp = Vec3{0.0f, -1.0f, 0.0f};
        }
        { // float mFOVY;   ///< Field of view in radians in up direction
            mFOVY = DegreesToRadians(70.0f);
        }
    }

    Renderer::~Renderer() {}

    static Mat44 sPerspectiveInfiniteReverseZ(float inFovY, float inAspect, float inNear, float inYSign)
    {
        float height = 1.0f / Tan(0.5f * inFovY);
        float width = height / inAspect;

        return Mat44(Vec4(width, 0.0f, 0.0f, 0.0f), Vec4(0.0f, inYSign * height, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, -1.0f),
                     Vec4(0.0f, 0.0f, inNear, 0.0f));
    }

    void Renderer::BeginFrame(const CameraState& inCamera, float inWorldScale)
    {
        // Mark that we're in the frame
        JPH_ASSERT(!mInFrame);
        mInFrame = true;

        // Store state
        mCameraState = inCamera;

        // Light properties
        Vec3 light_pos = inWorldScale * Vec3(250, 250, 250);
        Vec3 light_tgt = Vec3::sZero();
        Vec3 light_up = Vec3(0, 1, 0);
        Vec3 light_fwd = (light_tgt - light_pos).Normalized();
        float light_fov = DegreesToRadians(20.0f);
        float light_near = 1.0f;

        // Camera properties
        Vec3 cam_pos = Vec3(inCamera.mPos - mBaseOffset);
        float camera_fovy = inCamera.mFOVY;
        float camera_aspect = Engine::m_Engine->GetWindowAspectRatio();
        float camera_fovx = 2.0f * ATan(camera_aspect * Tan(0.5f * camera_fovy));
        float camera_near = 0.01f * inWorldScale;

        // Calculate camera frustum
        mCameraFrustum = Frustum(cam_pos, inCamera.mForward, inCamera.mUp, camera_fovx, camera_fovy, camera_near);

        // Calculate light frustum
        mLightFrustum = Frustum(light_pos, light_fwd, light_up, light_fov, light_fov, light_near);

        // Camera projection and view
        mVSBuffer.mProjection = sPerspectiveInfiniteReverseZ(camera_fovy, camera_aspect, camera_near, mPerspectiveYSign);
        Vec3 tgt = cam_pos + inCamera.mForward;
        mVSBuffer.mView = Mat44::sLookAt(cam_pos, tgt, inCamera.mUp);

        // Light projection and view
        mVSBuffer.mLightProjection = sPerspectiveInfiniteReverseZ(light_fov, 1.0f, light_near, mPerspectiveYSign);
        mVSBuffer.mLightView = Mat44::sLookAt(light_pos, light_tgt, light_up);

        // Camera ortho projection and view
        mVSBufferOrtho.mProjection =
            Mat44(Vec4(2.0f / Engine::m_Engine->GetWindowWidth(), 0.0f, 0.0f, 0.0f),
                  Vec4(0.0f, -mPerspectiveYSign * 2.0f / Engine::m_Engine->GetWindowHeight(), 0.0f, 0.0f),
                  Vec4(0.0f, 0.0f, -1.0f, 0.0f), Vec4(-1.0f, mPerspectiveYSign * 1.0f, 0.0f, 1.0f));
        mVSBufferOrtho.mView = Mat44::sIdentity();

        // Light projection and view are unused in ortho mode
        mVSBufferOrtho.mLightView = Mat44::sIdentity();
        mVSBufferOrtho.mLightProjection = Mat44::sIdentity();

        // Set constants for pixel shader
        mPSBuffer.mCameraPos = Vec4(cam_pos, 0);
        mPSBuffer.mLightPos = Vec4(light_pos, 0);
    }

    void Renderer::EndFrame()
    {
        // Mark that we're no longer in the frame
        JPH_ASSERT(mInFrame);
        mInFrame = false;
    }
} // namespace JPH