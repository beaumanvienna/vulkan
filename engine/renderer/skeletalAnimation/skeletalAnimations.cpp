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

#include "auxiliary/timestep.h"
#include "renderer/skeletalAnimation/skeletalAnimations.h"

namespace GfxRenderEngine
{

    SkeletalAnimations::SkeletalAnimations() : m_CurrentAnimation{nullptr}, m_FrameCounter{1} {}

    // by name
    SkeletalAnimation& SkeletalAnimations::operator[](std::string const& animation) { return *m_Animations[animation]; }

    // by index
    SkeletalAnimation& SkeletalAnimations::operator[](uint index) { return *m_AnimationsVector[index]; }

    void SkeletalAnimations::Push(std::shared_ptr<SkeletalAnimation> const& animation)
    {
        if (animation)
        {
            m_Animations[animation->GetName()] = animation;
            m_AnimationsVector.push_back(animation);
            m_NameToIndex[animation->GetName()] = static_cast<int>(m_AnimationsVector.size() - 1);
        }
        else
        {
            LOG_CORE_ERROR("SkeletalAnimations::Push: animation is empty!");
        }
    }

    void SkeletalAnimations::Start(std::string const& animation)
    {
        SkeletalAnimation* currentAnimation = m_Animations[animation].get();
        if (currentAnimation)
        {
            m_CurrentAnimation = currentAnimation;
            m_CurrentAnimation->Start();
        }
    }
    float SkeletalAnimations::GetCurrentTime()
    {
        if (m_CurrentAnimation)
        {
            return m_CurrentAnimation->GetCurrentTime();
        }
        else
        {
            return 0.0f;
        }
    }

    std::string SkeletalAnimations::GetName()
    {
        if (m_CurrentAnimation)
        {
            return m_CurrentAnimation->GetName();
        }
        else
        {
            return std::string("");
        }
    }

    float SkeletalAnimations::GetDuration(std::string const& animation) { return m_Animations[animation]->GetDuration(); }

    void SkeletalAnimations::Start(size_t index)
    {
        if (!(index < m_AnimationsVector.size()))
        {
            LOG_CORE_ERROR("SkeletalAnimations::Start(uint index) out of bounds");
            return;
        }
        SkeletalAnimation* currentAnimation = m_AnimationsVector[index].get();
        if (currentAnimation)
        {
            m_CurrentAnimation = currentAnimation;
            m_CurrentAnimation->Start();
        }
    }

    void SkeletalAnimations::Stop()
    {
        if (m_CurrentAnimation)
        {
            m_CurrentAnimation->Stop();
        }
    }

    void SkeletalAnimations::SetRepeat(bool repeat)
    {
        if (m_CurrentAnimation)
        {
            m_CurrentAnimation->SetRepeat(repeat);
        }
    }

    void SkeletalAnimations::SetRepeatAll(bool repeat)
    {
        for (auto& animation : m_AnimationsVector)
        {
            animation->SetRepeat(repeat);
        }
    }

    bool SkeletalAnimations::IsRunning() const
    {
        if (m_CurrentAnimation)
        {
            return m_CurrentAnimation->IsRunning();
        }
        else
        {
            return false;
        }
    }

    bool SkeletalAnimations::WillExpire(const Timestep& timestep) const
    {
        if (m_CurrentAnimation)
        {
            return m_CurrentAnimation->WillExpire(timestep);
        }
        else
        {
            return false;
        }
    }

    void SkeletalAnimations::Update(const Timestep& timestep, Armature::Skeleton& skeleton, uint frameCounter)
    {
        if (m_FrameCounter != frameCounter)
        {
            m_FrameCounter = frameCounter;

            if (m_CurrentAnimation)
            {
                m_CurrentAnimation->Update(timestep, skeleton);
            }
        }
    }

    // range-based for loop auxiliary functions
    SkeletalAnimations::Iterator SkeletalAnimations::begin() { return Iterator(&(*m_AnimationsVector.begin())); }
    SkeletalAnimations::Iterator SkeletalAnimations::end() { return Iterator(&(*m_AnimationsVector.end())); }

    // iterator functions
    SkeletalAnimations::Iterator::Iterator(pSkeletalAnimation* pointer) // constructor
    {
        m_Pointer = pointer;
    }
    SkeletalAnimations::Iterator& SkeletalAnimations::Iterator::operator++() // pre increment
    {
        ++m_Pointer;
        return *this;
    }
    bool SkeletalAnimations::Iterator::operator!=(const Iterator& rightHandSide) // compare
    {
        return m_Pointer != rightHandSide.m_Pointer;
    }
    SkeletalAnimation& SkeletalAnimations::Iterator::operator*() // dereference
    {
        return *(*m_Pointer /*shared_ptr*/);
    }

    int SkeletalAnimations::GetIndex(std::string const& animation)
    {
        bool found = false;
        for (auto& element : m_AnimationsVector)
        {
            if (element->GetName() == animation)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            return m_NameToIndex[animation];
        }
        else
        {
            return -1;
        }
    }
} // namespace GfxRenderEngine
