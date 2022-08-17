/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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
#include "layer/layer.h"
#include "renderer/renderer.h"
#include "sprite/spritesheet.h"
#include "gui/Common/UI/screen.h"
#include "UI/mainScreen.h"

namespace LucreApp
{

    class UI : public Layer
    {

    public:

        UI(const std::string& name = "U")
            : Layer(name) {}

        void OnAttach() override;
        void OnDetach() override;
        void OnEvent(Event& event) override;
        void OnUpdate(const Timestep& timestep) override;

        void OnResize();
        void Health(const float health);
        SCREEN_DrawBuffer* Draw() const { return m_ScreenManager->getUIContext()->Draw(); }

        static std::unique_ptr<SCREEN_ScreenManager> m_ScreenManager;
        static std::shared_ptr<Texture> m_FontAtlasTexture;
        static std::shared_ptr<Texture> m_SpritesheetTexture;

    private:

        bool Touch(int flags, float x, float y, int deviceID);
        void Key(int keyFlag, int keyCode, int deviceID);
        void Axis();

    private:

        MainScreen* m_MainScreen{nullptr};

        SpriteSheet* m_Spritesheet;

    };
}
