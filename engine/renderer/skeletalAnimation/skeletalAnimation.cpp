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

#include "core.h"

#include "renderer/skeletalAnimation/skeletalAnimation.h"

namespace GfxRenderEngine
{

    SkeletalAnimation::SkeletalAnimation(std::string const& name)
        : m_Name{name}, m_Repeat{false}
    {}

    void SkeletalAnimation::Start()
    {
        m_StartTime = Engine::m_Engine->GetTime();
        Time endTime = m_Keyframes.back().m_TimeStamp; // last keyframe
        m_Duration = endTime - m_StartTime;
    }

    bool SkeletalAnimation::IsRunning() const
    {
        return (m_Repeat || ((Engine::m_Engine->GetTime() - m_StartTime) < m_Duration));
    }

    void SkeletalAnimation::PushKeyframe(KeyPosition const& position)
    {
    }

    void SkeletalAnimation::PushKeyframe(KeyRotation const& rotation)
    {
    }

    void SkeletalAnimation::PushKeyframe(KeyScale const& scale)
    {
    }

    void SkeletalAnimation::Update(Armature::Skeleton& skeleton)
    {
        static float currentTime = 0.041f;
        currentTime += 0.01666;
        if (currentTime > 2.0f)
        {
            currentTime = 0.041f;
        }
        for (auto& channel : m_Channels)
        {
            auto& sampler = m_Samplers[channel.m_SamplerIndex];
            int jointIndex = skeleton.m_GlobalGltfNodeToJointIndex[channel.m_Node];
            auto& joint = skeleton.m_Joints[jointIndex]; // the joint to be animated

            for (size_t i = 0; i < sampler.m_Timestamps.size() - 1; i++)
            {
                if ((currentTime >= sampler.m_Timestamps[i]) && (currentTime <= sampler.m_Timestamps[i + 1]))
                {
                    float a = (currentTime - sampler.m_Timestamps[i]) / (sampler.m_Timestamps[i + 1] - sampler.m_Timestamps[i]);
                    switch (channel.m_Path)
                    {
                        case Path::TRANSLATION:
                        {
                            joint.m_DeformedNodeTranslation = glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i], sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
                            break;
                        }
                        case Path::ROTATION:
                        {
                            glm::quat q1;
                            q1.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
                            q1.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
                            q1.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
                            q1.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;

                            glm::quat q2;
                            q2.x = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].x;
                            q2.y = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].y;
                            q2.z = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].z;
                            q2.w = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].w;

                            joint.m_DeformedNodeRotation = glm::normalize(glm::slerp(q1, q2, a));
                            break;
                        }
                        case Path::SCALE:
                        {
                            joint.m_DeformedNodeScale = glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i], sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
                            break;
                        }
                        default:
                            LOG_CORE_CRITICAL("path not found");
                    }
                }
            }
        }
    }
}
