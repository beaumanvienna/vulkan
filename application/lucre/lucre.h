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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <memory>

#include "core.h"
#include "engine.h"
#include "scene/entity.h"
#include "platform/window.h"
#include "renderer/cursor.h"
#include "renderer/cameraController.h"
#include "sprite/spritesheet.h"

#include "application.h"
#include "appSettings.h"
#include "gameState.h"
#include "appEvent.h"
#include "UI/UIControllerIcon.h"
#include "UI/UI.h"

namespace LucreApp
{
    class Lucre : public Application
    {

    public:

        Lucre();
        ~Lucre() {}

        bool Start() override;
        void Shutdown() override;
        void OnUpdate(const Timestep& timestep) override;
        void OnEvent(Event& event) override;
        void OnAppEvent(AppEvent& event);
        void OnResize();

        void PlaySound(int resourceID);
        UIControllerIcon* GetUI() const { return m_UIControllerIcon; }
        Scene& GetScene() override { return m_GameState.GetScene(); }
        bool KeyboardInputIsReleased() const { return !m_GUIisRunning; }

    public:

        static std::shared_ptr<Lucre> m_Application;
        static SpriteSheet* m_Spritesheet;

    private:

        void InitSettings();
        void InitCursor();
        void ShowCursor();
        void HideCursor();
        void Cancel();

    private:

        UIControllerIcon* m_UIControllerIcon = nullptr;
        UI*               m_UI = nullptr;

        AppSettings m_AppSettings{&Engine::m_SettingsManager};
        GameState m_GameState;
        Scene* m_CurrentScene;

        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Cursor> m_Cursor;
        std::shared_ptr<Renderer> m_Renderer;
        std::shared_ptr<Cursor> m_EmptyCursor;
        std::shared_ptr<CameraController> m_CameraController;

        SpriteSheet m_Atlas;
        bool m_GUIisRunning;

    };
}
