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

#include <memory>
#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#else
// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
#define JPH_DEBUG_RENDERER
// Make sure the debug renderer symbols don't get imported or exported
#define JPH_DEBUG_RENDERER_EXPORT
#include <Jolt/Renderer/DebugRenderer.h>
#undef JPH_DEBUG_RENDERER
#undef JPH_DEBUG_RENDERER_EXPORT
#endif

#include <Jolt/Jolt.h>
#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/UnorderedMap.h>

#include "TestFramework.h"
#include <Renderer/Renderer.h>
#include "engine/platform/Vulkan/VKswapChain.h"

namespace JPH
{

    class Renderer;
    class Font;

    /// Implementation of DebugRenderer
    class DebugRendererImp final : public DebugRenderer
    {
    public:
        JPH_OVERRIDE_NEW_DELETE

        DebugRendererImp(Renderer* inRenderer, const Font* inFont);
        void CompileShaders();
        /// Implementation of DebugRenderer interface
        virtual void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
        virtual void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor,
                                  ECastShadow inCastShadow = ECastShadow::Off) override;
        virtual Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
        virtual Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices,
                                          int inIndexCount) override;
        virtual void DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq,
                                  ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode,
                                  ECastShadow inCastShadow, EDrawMode inDrawMode) override;
        virtual void DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight) override;

        void DrawPass();

        /// Draw all primitives that were added
        void Draw();

        /// Clear all primitives (to be called after drawing)
        void Clear();

    private:
        /// Helper functions to draw sub parts
        void DrawLines();
        void DrawTriangles();
        void DrawTexts();

        /// Helper functions to clear sub parts
        void ClearLines();
        void ClearTriangles();
        void ClearTexts();

        /// Finalize the current locked primitive and add it to the primitives to draw
        void FinalizePrimitive();

        /// Ensure that the current locked primitive has space for a primitive consisting inVtxSize vertices
        void EnsurePrimitiveSpace(int inVtxSize);

        Renderer* mRenderer;

        /// Shaders for triangles
        unique_ptr<PipelineState> mTriangleStateBF;
        unique_ptr<PipelineState> mTriangleStateFF;
        unique_ptr<PipelineState> mTriangleStateWire;

        /// Lock that protects the triangle batches from being accessed from multiple threads
        Mutex mPrimitivesLock;

        Batch mEmptyBatch;

        /// Properties for a single rendered instance
        struct Instance
        {
            /// Constructor
            Instance(Mat44Arg inModelMatrix, Mat44Arg inModelMatrixInvTrans, ColorArg inModelColor)
                : mModelMatrix(inModelMatrix), mModelMatrixInvTrans(inModelMatrixInvTrans), mModelColor(inModelColor)
            {
            }

            Mat44 mModelMatrix;
            Mat44 mModelMatrixInvTrans;
            Color mModelColor;
        };

        /// Rendered instance with added information for lodding
        struct InstanceWithLODInfo : public Instance
        {
            /// Constructor
            InstanceWithLODInfo(Mat44Arg inModelMatrix, Mat44Arg inModelMatrixInvTrans, ColorArg inModelColor,
                                const AABox& inWorldSpaceBounds, float inLODScaleSq)
                : Instance(inModelMatrix, inModelMatrixInvTrans, inModelColor), mWorldSpaceBounds(inWorldSpaceBounds),
                  mLODScaleSq(inLODScaleSq)
            {
            }

            /// Bounding box for culling
            AABox mWorldSpaceBounds;

            /// Square of scale factor for LODding (1 = original, > 1 = lod out further, < 1 = lod out earlier)
            float mLODScaleSq;
        };

        /// Properties for a batch of instances that have the same primitive
        struct Instances
        {
            Array<InstanceWithLODInfo> mInstances;

            /// Start index in mInstancesBuffer for each of the LOD in the geometry pass. Length is one longer than the
            /// number of LODs to indicate how many instances the last lod has.
            Array<int> mGeometryStartIdx;

            /// Start index in mInstancesBuffer for each of the LOD in the light pass. Length is one longer than the number
            /// of LODs to indicate how many instances the last lod has.
            Array<int> mLightStartIdx;
        };

        using InstanceMap = UnorderedMap<GeometryRef, Instances>;

        /// Clear map of instances and make it ready for the next frame
        void ClearMap(InstanceMap& ioInstances);

        /// Helper function to draw instances
        inline void DrawInstances(const Geometry* inGeometry, const Array<int>& inStartIdx);

        /// List of primitives that are finished and ready for drawing
        InstanceMap mWireframePrimitives;
        InstanceMap mPrimitives;
        InstanceMap mTempPrimitives;
        InstanceMap mPrimitivesBackFacing;
        int mNumInstances = 0;
        Ref<RenderInstances> mInstancesBuffer[VK_SwapChain::MAX_FRAMES_IN_FLIGHT];

        /// Primitive that is being built + its properties
        Ref<RenderPrimitive> mLockedPrimitive;
        Vertex* mLockedVerticesStart = nullptr;
        Vertex* mLockedVertices = nullptr;
        Vertex* mLockedVerticesEnd = nullptr;
        AABox mLockedPrimitiveBounds;

        /// A single text string
        struct Text
        {
            Text(Vec3Arg inPosition, const string_view& inText, ColorArg inColor, float inHeight)
                : mPosition(inPosition), mText(inText), mColor(inColor), mHeight(inHeight)
            {
            }

            Vec3 mPosition;
            String mText;
            Color mColor;
            float mHeight;
        };

        /// All text strings that are to be drawn on screen
        Array<Text> mTexts;
        Mutex mTextsLock;

        /// Font with which to draw the texts
        RefConst<Font> mFont;

        /// A single line segment
        struct Line
        {
            Float3 mFrom;
            Color mFromColor;
            Float3 mTo;
            Color mToColor;
        };

        /// The list of line segments
        Array<Line> mLines;
        Mutex mLinesLock;

        /// The shaders for the line segments
        std::unique_ptr<PipelineState> mLineState;
    };
} // namespace JPH