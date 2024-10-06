/* Engine Copyright (c) 2024 Engine Development Team
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

#include "engine.h"
#include "core.h"
#include "layer/layer.h"
#include "renderer/renderer.h"
#include "sprite/spritesheet.h"
#include "transform/transformation.h"

namespace LucreApp
{

    class UIStarIcon : public Layer
    {
        enum State
        {
            IDLE,
            MOVE_IN,
            ROTATE,
            MOVE_OUT
        };

    public:
        UIStarIcon(std::string const& name = "UIStarIcon");
        ~UIStarIcon() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;
        void OnUpdate(Timestep const& timestep) override;
        void OnResize();

        void Start() { m_Start = true; }
        void Stop() { m_Stop = true; }

    private:
        void StartSequence();
        void StopSequence();
        void Rotate();
        void ChangeState(State state);
        void Init();

    private:
        Renderer* m_Renderer{nullptr};

        SpriteSheet* m_Spritesheet{nullptr};
        Sprite2D m_StarSprite;

        Animation m_StarMoveIn1;
        Animation m_StarMoveIn2;
        Animation m_StarMoveIn3;
        Animation m_StarRotate1;
        Animation m_StarRotate2;
        Animation m_StarRotate3;
        Animation m_StarMoveOut1;
        Animation m_StarMoveOut2;
        Animation m_StarMoveOut3;

        bool m_Start;
        bool m_Stop;

        State m_State;
    };
} // namespace LucreApp
