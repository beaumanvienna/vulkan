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

#include <map>
#include <memory>

#include "engine.h"
#include "renderer/skeletalAnimation/skeletalAnimation.h"

namespace GfxRenderEngine
{

    class SkeletalAnimations
    {

    public:
        using pSkeletalAnimation = std::shared_ptr<SkeletalAnimation>;

        struct Iterator // used for range-based loops to traverse the array elements in m_AnimationsVector
        {
            Iterator(pSkeletalAnimation* pointer);          // iterator points to an array element of m_AnimationsVector
            Iterator& operator++();                         // pre increment operator (next element)
            bool operator!=(const Iterator& rightHandSide); // unequal operator
            SkeletalAnimation& operator*();                 // dereference operator

        private:
            pSkeletalAnimation* m_Pointer;
        };

    public:               // iterator functions to traverse Array
        Iterator begin(); // see Iterator struct above
        Iterator end();
        SkeletalAnimation& operator[](std::string const& animation);
        SkeletalAnimation& operator[](uint index);

    public:
        SkeletalAnimations();

        size_t Size() const { return m_Animations.size(); }
        void Push(std::shared_ptr<SkeletalAnimation> const& animation);

        void Start(std::string const& animation); // by name
        void Start(size_t index);                 // by index
        void Start() { Start(0); };               // start animation 0
        void Stop();
        void SetRepeat(bool repeat);
        void SetRepeatAll(bool repeat);
        bool IsRunning() const;
        bool WillExpire(const Timestep& timestep) const;
        float GetDuration(std::string const& animation);
        float GetCurrentTime();
        std::string GetName();
        void Update(const Timestep& timestep, Armature::Skeleton& skeleton, uint frameCounter);
        int GetIndex(std::string const& animation);

    private:
        std::map<std::string, std::shared_ptr<SkeletalAnimation>> m_Animations;
        std::vector<std::shared_ptr<SkeletalAnimation>> m_AnimationsVector;
        SkeletalAnimation* m_CurrentAnimation;
        uint m_FrameCounter;
        std::map<std::string, int> m_NameToIndex;
    };
} // namespace GfxRenderEngine
