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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <vector>

#include "engine.h"
#include "sprite/spritesheet.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    class SpriteAnimation
    {

    public:
        using Duration = std::chrono::duration<float, std::chrono::seconds::period>;

    public:
        SpriteAnimation() {}
        SpriteAnimation(uint frames, Duration durationPerFrame, SpriteSheet* spritesheet);
        void Create(uint frames, Duration durationPerFrame, SpriteSheet* spritesheet);
        void Create(Duration durationPerFrame, SpriteSheet* spritesheet);
        uint GetFrames() const { return m_Frames; }
        uint GetCurrentFrame() const;
        bool IsNewFrame();
        void Start();
        bool IsRunning() const;
        Sprite GetSprite();

    private:
        uint m_Frames;
        Duration m_Duration;
        float m_TimeFactor;
        SpriteSheet* m_Spritesheet;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
        uint m_PreviousFrame;
    };
} // namespace GfxRenderEngine
