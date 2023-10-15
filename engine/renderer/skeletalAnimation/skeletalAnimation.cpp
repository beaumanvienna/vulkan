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

    glm::mat4 SkeletalAnimation::GetTransform()
    {
        glm::mat4 transform{1.0f};
        return transform;
    }

}
