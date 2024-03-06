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

#include "engine.h"
#include "core.h"
#include "sprite/spriteAnimation.h"

namespace GfxRenderEngine
{

    SpriteAnimation::SpriteAnimation(uint frames, Duration durationPerFrame, SpriteSheet* spritesheet)
        : m_Spritesheet(spritesheet), m_Frames(frames)
    {
        m_Duration = frames * durationPerFrame;
        m_TimeFactor = m_Frames / m_Duration.count();
    }

    void SpriteAnimation::Create(uint frames, Duration durationPerFrame, SpriteSheet* spritesheet)
    {
        m_Duration = frames * durationPerFrame;
        m_Spritesheet = spritesheet;
        m_Frames = frames;
        m_TimeFactor = m_Frames / m_Duration.count();
    }

    void SpriteAnimation::Create(Duration durationPerFrame, SpriteSheet* spritesheet)
    {
        Create(spritesheet->GetNumberOfSprites(), durationPerFrame, spritesheet);
    }

    Sprite SpriteAnimation::GetSprite()
    {
        Sprite sprite;
        if (IsRunning())
        {
            sprite = m_Spritesheet->GetSprite(GetCurrentFrame());
        }
        else
        {
            sprite = m_Spritesheet->GetSprite(0);
        }
        return sprite;
    }

    void SpriteAnimation::Start()
    {
        m_PreviousFrame = -1;
        m_StartTime = Engine::m_Engine->GetTime();
    }

    bool SpriteAnimation::IsRunning() const { return (Engine::m_Engine->GetTime() - m_StartTime) < m_Duration; }

    uint SpriteAnimation::GetCurrentFrame() const
    {
        Duration timeElapsed = Engine::m_Engine->GetTime() - m_StartTime;
        uint index = static_cast<uint>(timeElapsed.count() * m_TimeFactor);
        return std::min(index, m_Frames - 1);
    }

    bool SpriteAnimation::IsNewFrame()
    {
        uint currentFrame = GetCurrentFrame();
        bool isNewFrame = (currentFrame != m_PreviousFrame);
        m_PreviousFrame = currentFrame;
        return isNewFrame;
    }
} // namespace GfxRenderEngine
