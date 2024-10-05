/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
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

#include <stack>
#include <mutex>

#include "renderer/renderer.h"
#include "renderer/cameraController.h"
#include "sprite/spritesheet.h"
#include "gui/Common/Input/inputState.h"

namespace GfxRenderEngine
{

    namespace SCREEN_UI
    {
        class View;
    }

    enum DialogResult
    {
        DR_OK,
        DR_CANCEL,
        DR_YES,
        DR_NO,
        DR_BACK,
    };

    class SCREEN_ScreenManager;
    class SCREEN_UIContext;

    namespace SCREEN_Draw
    {
        class SCREEN_DrawContext;
    }

    class SCREEN_Screen
    {
    public:
        SCREEN_Screen() : screenManager_(nullptr) {}

        virtual ~SCREEN_Screen() { screenManager_ = nullptr; }

        virtual void onFinish(DialogResult reason) {}
        virtual void update() {}
        virtual void preRender() {}
        virtual void render() {}
        virtual void postRender() {}
        virtual void resized() {}
        virtual void dialogFinished(const SCREEN_Screen* dialog, DialogResult result) {}
        virtual bool touch(const SCREEN_TouchInput& touch) { return false; }
        virtual bool key(const SCREEN_KeyInput& key) { return false; }
        virtual bool axis(const SCREEN_AxisInput& touch) { return false; }
        virtual void sendMessage(const char* msg, const char* value) {}
        virtual void deviceLost() {}
        virtual void deviceRestored() {}

        virtual void RecreateViews() {}

        SCREEN_ScreenManager* screenManager() { return screenManager_; }
        void setSCREEN_ScreenManager(SCREEN_ScreenManager* sm) { screenManager_ = sm; }

        virtual void* dialogData() { return 0; }

        virtual std::string tag() const { return std::string(""); }

        virtual bool isTransparent() const { return false; }
        virtual bool isTopLevel() const { return false; }

        virtual SCREEN_TouchInput transformTouch(const SCREEN_TouchInput& touch) { return touch; }

    private:
        SCREEN_ScreenManager* screenManager_;
    };

    class SCREEN_Transition
    {
    public:
        SCREEN_Transition() {}
    };

    enum
    {
        LAYER_SIDEMENU = 1,
        LAYER_TRANSPARENT = 2,
    };

    typedef void (*PostRenderCallback)(SCREEN_UIContext* ui, void* userdata);

    class SCREEN_ScreenManager
    {
    public:
        SCREEN_ScreenManager(Renderer* renderer, SpriteSheet* spritesheetUI);
        virtual ~SCREEN_ScreenManager();

        void switchScreen(SCREEN_Screen* screen);
        void update();

        void setUIContext(SCREEN_UIContext* context) { uiContext_ = context; }
        SCREEN_UIContext* getUIContext() { return uiContext_; }

        void setSCREEN_DrawContext(SCREEN_Draw::SCREEN_DrawContext* context) { thin3DContext_ = context; }
        SCREEN_Draw::SCREEN_DrawContext* getSCREEN_DrawContext() { return thin3DContext_; }

        void setPostRenderCallback(PostRenderCallback cb, void* userdata)
        {
            postRenderCb_ = cb;
            postRenderUserdata_ = userdata;
        }

        void render();
        void resized();
        void shutdown();

        void deviceLost();
        void deviceRestored();

        void push(SCREEN_Screen* screen, int layerFlags = 0);

        void RecreateAllViews();

        void finishDialog(SCREEN_Screen* dialog, DialogResult result = DR_OK);
        SCREEN_Screen* dialogParent(const SCREEN_Screen* dialog) const;

        bool touch(const SCREEN_TouchInput& touch);
        bool key(const SCREEN_KeyInput& key);
        bool axis(const SCREEN_AxisInput& touch);

        void sendMessage(const char* msg, const char* value);

        SCREEN_Screen* topScreen() const;

    public:
        std::recursive_mutex inputLock_;
        static SpriteSheet* m_SpritesheetUI;
        static SCREEN_ScreenManager* m_ScreenManager;
        static std::shared_ptr<CameraController> m_CameraController;

    private:
        void pop();
        void switchToNext();
        void processFinishDialog();

    private:
        SCREEN_UIContext* uiContext_;
        SCREEN_Draw::SCREEN_DrawContext* thin3DContext_;

        PostRenderCallback postRenderCb_ = nullptr;
        void* postRenderUserdata_ = nullptr;

        const SCREEN_Screen* dialogFinished_;
        DialogResult dialogResult_;

        struct Layer
        {
            SCREEN_Screen* screen;
            int flags;
            SCREEN_UI::View* focusedView;
        };

        std::vector<Layer> stack_;
        std::vector<Layer> nextStack_;
        std::stack<SCREEN_UI::View*> lastFocusView;

        Renderer* m_Renderer;
    };
} // namespace GfxRenderEngine
