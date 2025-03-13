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

#include <Image/Surface.h>
#include <Renderer/Frustum.h>
#include <Renderer/PipelineState.h>
#include <Renderer/VertexShader.h>
#include <Renderer/PixelShader.h>
#include <Renderer/RenderPrimitive.h>
#include <Renderer/RenderInstances.h>
#include <memory>

#include "renderer/camera.h"

namespace JPH
{

    // Forward declares
    class Texture;

    /// Camera setup
    struct CameraState
    {
        CameraState();
        CameraState(GfxRenderEngine::Camera const& cam0);

        RVec3 mPos;    ///< Camera position
        Vec3 mForward; ///< Camera forward vector
        Vec3 mUp;      ///< Camera up vector
        float mFOVY;   ///< Field of view in radians in up direction

        void Print();
    };

    /// Responsible for rendering primitives to the screen
    class Renderer
    {
    public:
        /// Destructor
        virtual ~Renderer();

        /// Initialize renderer
        virtual void Initialize() = 0;

        /// Start / end drawing a frame
        virtual void BeginFrame(const CameraState& inCamera, float inWorldScale);
        virtual void EndFrame();

        /// Switch between orthographic and 3D projection mode
        virtual void SetProjectionMode() = 0;
        virtual void SetOrthoMode() = 0;

        /// Create texture from an image surface
        virtual Ref<Texture> CreateTexture(const Surface* inSurface) = 0;

        /// Compile a vertex shader
        virtual Ref<VertexShader> CreateVertexShader(const char* inName) = 0;

        /// Compile a pixel shader
        virtual Ref<PixelShader> CreatePixelShader(const char* inName) = 0;

        /// Create pipeline state object that defines the complete state of how primitives should be rendered
        virtual unique_ptr<PipelineState> CreatePipelineState(
            const VertexShader* inVertexShader, const PipelineState::EInputDescription* inInputDescription,
            uint inInputDescriptionCount, const PixelShader* inPixelShader, PipelineState::EDrawPass inDrawPass,
            PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest,
            PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode, std::string const& debugName) = 0;

        /// Create a render primitive
        virtual RenderPrimitive* CreateRenderPrimitive(PipelineState::ETopology inType) = 0;

        /// Create render instances object to allow drawing batches of objects
        virtual RenderInstances* CreateRenderInstances() = 0;

        /// Get the shadow map texture
        virtual Texture* GetShadowMap() const = 0;

        /// Get the camera state / frustum (only valid between BeginFrame() / EndFrame())
        const CameraState& GetCameraState() const
        {
            JPH_ASSERT(mInFrame);
            return mCameraState;
        }
        const Frustum& GetCameraFrustum() const
        {
            JPH_ASSERT(mInFrame);
            return mCameraFrustum;
        }

        /// Offset relative to which the world is rendered, helps avoiding rendering artifacts at big distances
        RVec3 GetBaseOffset() const { return mBaseOffset; }
        void SetBaseOffset(RVec3 inOffset) { mBaseOffset = inOffset; }

        /// Get the light frustum (only valid between BeginFrame() / EndFrame())
        const Frustum& GetLightFrustum() const
        {
            JPH_ASSERT(mInFrame);
            return mLightFrustum;
        }

        /// Size of the shadow map will be cShadowMapSize x cShadowMapSize pixels
        static const uint cShadowMapSize = 4096;

        /// Which frame is currently rendering (to keep track of which buffers are free to overwrite)
        uint GetCurrentFrameIndex() const
        {
            JPH_ASSERT(mInFrame);
            return mFrameIndex;
        }

        /// Create a platform specific Renderer instance
        static Renderer* sCreate();

    protected:
        struct VertexShaderConstantBuffer
        {
            Mat44 mView;
            Mat44 mProjection;
            Mat44 mLightView;
            Mat44 mLightProjection;
        };

        struct PixelShaderConstantBuffer
        {
            Vec4 mCameraPos;
            Vec4 mLightPos;
        };

        float mPerspectiveYSign = 1.0f; ///< Sign for the Y coordinate in the projection matrix (1 for DX, -1 for Vulkan)
        bool mInFrame = false;          ///< If we're within a BeginFrame() / EndFrame() pair
        CameraState mCameraState;
        RVec3 mBaseOffset{RVec3::sZero()}; ///< Offset to subtract from the camera position to deal with large worlds
        Frustum mCameraFrustum;
        Frustum mLightFrustum;
        uint mFrameIndex = 0; ///< Current frame index (0 or 1)
        VertexShaderConstantBuffer mVSBuffer;
        VertexShaderConstantBuffer mVSBufferOrtho;
        PixelShaderConstantBuffer mPSBuffer;
    };
} // namespace JPH