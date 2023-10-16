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
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{


    class SkeletalAnimation
    {

    public:

        using Duration = std::chrono::duration<float, std::chrono::seconds::period>;
        using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;

    public:

        enum class InterpolationMethod
        {
            LINEAR,
            STEP,
            CUBICSPLINE
        };

        struct Sampler
        {
            float m_Timestamp;
            InterpolationMethod m_Interpolation;
            glm::vec4 m_TRSoutputValuesToBeInterpolated;
        };

        struct KeyPosition
        {
            glm::vec3 m_Position;
            Sampler m_Sampler;
        };

        struct KeyRotation
        {
            glm::quat m_Rotation;
            Sampler m_Sampler;
        };

        struct KeyScale
        {
            glm::vec3 m_Scale;
            Sampler m_Sampler;
        };

        struct Keyframe
        {
            KeyPosition m_KeyPosition;
            KeyRotation m_KeyRotation;
            KeyScale m_KeyScale;
            Time m_TimeStamp;
        };

    public:

        SkeletalAnimation(std::string const& name);

        void Start();
        bool IsRunning() const;
        std::string GetName() const { return m_Name; }
        void SetRepeat(bool repeat) { m_Repeat = repeat; }
        glm::mat4 GetTransform();

        void PushKeyframe(KeyPosition const& position);
        void PushKeyframe(KeyRotation const& rotation);
        void PushKeyframe(KeyScale    const& scale);

    private:

        std::string m_Name; 
        bool m_Repeat;
        Duration m_Duration;
        Time m_StartTime;
        std::vector<Keyframe> m_Keyframes;

    };
}