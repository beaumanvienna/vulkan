/* Engine Copyright (c) 2022 Engine Development Team
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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#pragma once

#include "engine.h"
#include "renderer/renderer.h"
#include "renderer/builder/builder.h"
#include "renderer/builder/gltfBuilder.h"
#include "renderer/builder/terrainBuilder.h"
#include "renderer/builder/fastgltfBuilder.h"
#include "renderer/builder/ufbxBuilder.h"
#include "renderer/builder/fbxBuilder.h"
#include "auxiliary/threadPool.h"

namespace GfxRenderEngine
{
    namespace Chrono
    {
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using Duration = std::chrono::duration<float, std::chrono::seconds::period>;
    } // namespace Chrono

    class GraphicsContext
    {

    public:
        virtual ~GraphicsContext() = default;

        virtual bool Init() = 0;
        virtual void SetVSync(int interval) = 0;
        virtual void LimitFrameRate(Chrono::TimePoint) = 0;
        virtual bool IsInitialized() const = 0;

        virtual Renderer* GetRenderer() const = 0;
        virtual std::shared_ptr<Model> LoadModel(const Builder& builder) = 0;
        virtual std::shared_ptr<Model> LoadModel(const TerrainBuilder& builder) = 0;
        virtual std::shared_ptr<Model> LoadModel(const GltfBuilder& builder) = 0;
        virtual std::shared_ptr<Model> LoadModel(const Model::ModelData& modelData) = 0;
        virtual std::shared_ptr<Model> LoadModel(const FbxBuilder& builder) = 0;
        virtual std::shared_ptr<Model> LoadModel(const UFbxBuilder& builder) = 0;
        virtual void ToggleDebugWindow(const GenericCallback& callback = nullptr) = 0;

        virtual uint GetContextWidth() const = 0;
        virtual uint GetContextHeight() const = 0;
        virtual bool MultiThreadingSupport() const = 0;
        virtual void WaitIdle() const = 0;
        virtual void ResetDescriptorPool(ThreadPool& threadPool) = 0;

        static std::shared_ptr<GraphicsContext> Create(void* window, ThreadPool& threadPoolPrimary,
                                                       ThreadPool& threadPoolSecondary);

    private:
    };
} // namespace GfxRenderEngine
