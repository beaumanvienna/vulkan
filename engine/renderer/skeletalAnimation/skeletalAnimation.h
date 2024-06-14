/* Engine Copyright (c) 2023 Engine Development Team
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

#include "engine.h"
#include "renderer/skeletalAnimation/skeleton.h"

namespace GfxRenderEngine
{
    class Timestep;
    class SkeletalAnimation
    {

    public:
        enum class Path
        {
            TRANSLATION,
            ROTATION,
            SCALE
        };

        enum class InterpolationMethod
        {
            LINEAR,
            STEP,
            CUBICSPLINE
        };

        struct Channel
        {
            Path m_Path;
            int m_SamplerIndex;
            int m_Node;
        };

        struct Sampler
        {
            std::vector<float> m_Timestamps;
            std::vector<glm::vec4> m_TRSoutputValuesToBeInterpolated;
            InterpolationMethod m_Interpolation;
        };

    public:
        SkeletalAnimation(std::string const& name);

        void Start();
        void Stop();
        bool IsRunning() const;
        bool WillExpire(const Timestep& timestep) const;
        std::string const& GetName() const { return m_Name; }
        void SetRepeat(bool repeat) { m_Repeat = repeat; }
        void Update(const Timestep& timestep, Armature::Skeleton& skeleton);
        float GetDuration() const { return m_LastKeyFrameTime - m_FirstKeyFrameTime; }
        float GetCurrentTime() const { return m_CurrentKeyFrameTime - m_FirstKeyFrameTime; }

        std::vector<SkeletalAnimation::Sampler> m_Samplers;
        std::vector<SkeletalAnimation::Channel> m_Channels;

        void SetFirstKeyFrameTime(float firstKeyFrameTime) { m_FirstKeyFrameTime = firstKeyFrameTime; }
        void SetLastKeyFrameTime(float lastKeyFrameTime) { m_LastKeyFrameTime = lastKeyFrameTime; }

    private:
        std::string m_Name;
        bool m_Repeat;

        // relative animation time
        float m_FirstKeyFrameTime;
        float m_LastKeyFrameTime;
        float m_CurrentKeyFrameTime = 0.0f;
    };
} // namespace GfxRenderEngine
