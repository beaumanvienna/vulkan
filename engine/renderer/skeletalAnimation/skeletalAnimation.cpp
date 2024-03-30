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

    SkeletalAnimation::SkeletalAnimation(std::string const& name) : m_Name{name}, m_Repeat{false} {}

    void SkeletalAnimation::Start() { m_CurrentKeyFrameTime = m_FirstKeyFrameTime; }

    void SkeletalAnimation::Stop() { m_CurrentKeyFrameTime = m_LastKeyFrameTime + 1.0f; }

    bool SkeletalAnimation::IsRunning() const { return (m_Repeat || (m_CurrentKeyFrameTime <= m_LastKeyFrameTime)); }

    bool SkeletalAnimation::WillExpire(const Timestep& timestep) const
    {
        return (!m_Repeat && ((m_CurrentKeyFrameTime + timestep) > m_LastKeyFrameTime));
    }

    void SkeletalAnimation::Update(const Timestep& timestep, Armature::Skeleton& skeleton)
    {
        if (!IsRunning())
        {
            LOG_CORE_WARN("Animation '{0}' expired", m_Name);
            return;
        }
        m_CurrentKeyFrameTime += timestep;

        if (m_Repeat && (m_CurrentKeyFrameTime > m_LastKeyFrameTime))
        {
            m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
        }
        for (auto& channel : m_Channels)
        {
            auto& sampler = m_Samplers[channel.m_SamplerIndex];
            int jointIndex = skeleton.m_GlobalNodeToJointIndex[channel.m_Node];
            auto& joint = skeleton.m_Joints[jointIndex]; // the joint to be animated

            for (size_t i = 0; i < sampler.m_Timestamps.size() - 1; i++)
            {
                if ((m_CurrentKeyFrameTime >= sampler.m_Timestamps[i]) &&
                    (m_CurrentKeyFrameTime <= sampler.m_Timestamps[i + 1]))
                {
                    switch (sampler.m_Interpolation)
                    {
                        case InterpolationMethod::LINEAR:
                        {
                            float a = (m_CurrentKeyFrameTime - sampler.m_Timestamps[i]) /
                                      (sampler.m_Timestamps[i + 1] - sampler.m_Timestamps[i]);
                            switch (channel.m_Path)
                            {
                                case Path::TRANSLATION:
                                {
                                    joint.m_DeformedNodeTranslation =
                                        glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
                                                 sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
                                    break;
                                }
                                case Path::ROTATION:
                                {
                                    glm::quat quaternion1;
                                    quaternion1.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
                                    quaternion1.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
                                    quaternion1.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
                                    quaternion1.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;

                                    glm::quat quaternion2;
                                    quaternion2.x = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].x;
                                    quaternion2.y = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].y;
                                    quaternion2.z = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].z;
                                    quaternion2.w = sampler.m_TRSoutputValuesToBeInterpolated[i + 1].w;

                                    joint.m_DeformedNodeRotation = glm::normalize(glm::slerp(quaternion1, quaternion2, a));
                                    break;
                                }
                                case Path::SCALE:
                                {
                                    joint.m_DeformedNodeScale =
                                        glm::mix(sampler.m_TRSoutputValuesToBeInterpolated[i],
                                                 sampler.m_TRSoutputValuesToBeInterpolated[i + 1], a);
                                    break;
                                }
                                default:
                                    LOG_CORE_CRITICAL("path not found");
                            }
                            break;
                        }
                        case InterpolationMethod::STEP:
                        {
                            switch (channel.m_Path)
                            {
                                case Path::TRANSLATION:
                                {
                                    joint.m_DeformedNodeTranslation =
                                        glm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
                                    break;
                                }
                                case Path::ROTATION:
                                {
                                    joint.m_DeformedNodeRotation.x = sampler.m_TRSoutputValuesToBeInterpolated[i].x;
                                    joint.m_DeformedNodeRotation.y = sampler.m_TRSoutputValuesToBeInterpolated[i].y;
                                    joint.m_DeformedNodeRotation.z = sampler.m_TRSoutputValuesToBeInterpolated[i].z;
                                    joint.m_DeformedNodeRotation.w = sampler.m_TRSoutputValuesToBeInterpolated[i].w;
                                    break;
                                }
                                case Path::SCALE:
                                {
                                    joint.m_DeformedNodeScale = glm::vec3(sampler.m_TRSoutputValuesToBeInterpolated[i]);
                                    break;
                                }
                                default:
                                    LOG_CORE_CRITICAL("path not found");
                            }
                            break;
                        }
                        case InterpolationMethod::CUBICSPLINE:
                        {
                            LOG_CORE_WARN("SkeletalAnimation::Update(...): interploation method CUBICSPLINE not supported");
                            break;
                        }
                        default:
                            LOG_CORE_WARN("SkeletalAnimation::Update(...): interploation method not supported");
                            break;
                    }
                }
            }
        }
    }
} // namespace GfxRenderEngine
