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

#include "engine.h"
#include "core.h"
#include "layer/layer.h"
#include "renderer/renderer.h"
#include "sprite/spritesheet.h"
#include "transform/transformation.h"

namespace LucreApp
{

    class UIControllerIcon : public Layer
    {

    public:
        UIControllerIcon(bool indent, const std::string& name = "UIControllerIcon");
        virtual ~UIControllerIcon();

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;
        void OnUpdate(const Timestep& timestep) override;
        void Indent(bool indent);
        bool IsMovingIn();
        void Init();

    public:
        Registry m_Registry;

    private:
        void LoadModels();

    private:
        Sprite2D m_ControllerSprite;
        entt::entity m_ID1, m_ID2;
        bool m_Indent;

        Animation m_Controller1MoveIn;
        Animation m_Controller1MoveOut;
        bool m_Controller1Detected;

        Animation m_Controller2MoveIn;
        Animation m_Controller2MoveOut;
        bool m_Controller2Detected;
    };
} // namespace LucreApp
