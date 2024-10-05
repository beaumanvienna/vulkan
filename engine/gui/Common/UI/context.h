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

#include <memory>
#include <vector>

#include "core.h"
#include "gui/Common/Math/geom2d.h"
#include "gui/Render/textureAtlas.h"
#include "gui/Common/UI/view.h"

namespace GfxRenderEngine
{

    class SCREEN_ManagedTexture
    {
    public:
        SCREEN_ManagedTexture() { LOG_CORE_CRITICAL("not implemented: SCREEN_ManagedTexture"); }
        ~SCREEN_ManagedTexture() { LOG_CORE_CRITICAL("not implemented: ~SCREEN_ManagedTexture"); }
    };

    namespace SCREEN_Draw
    {
        class SCREEN_DrawContext;
        class SCREEN_Pipeline;
        class DepthStencilState;
        class SCREEN_Texture;
        class BlendState;
        class SCREEN_SamplerState;
        class RasterState;
    } // namespace SCREEN_Draw

    class SCREEN_Texture;
    class SCREEN_ManagedTexture;
    class SCREEN_DrawBuffer;
    class SCREEN_TextDrawer;

    namespace SCREEN_UI
    {
        struct Drawable;
        struct EventParams;
        struct Theme;
        struct FontStyle;
        class Event;
        class View;
    } // namespace SCREEN_UI

    class SCREEN_DrawBuffer;

    struct UITransform
    {
        glm::vec3 translate;
        glm::vec3 scale;
        float alpha;
    };

    class SCREEN_UIContext
    {
    public:
        SCREEN_UIContext();
        ~SCREEN_UIContext();

        void Flush();

        void PushScissor(const Bounds& bounds);
        void PopScissor();
        Bounds GetScissorBounds();

        void ActivateTopScissor();

        SCREEN_DrawBuffer* Draw() const { return uidrawbuffer_; }
        SCREEN_DrawBuffer* DrawTop() const { return uidrawbufferTop_; }
        SCREEN_UI::Theme ui_theme;
        const SCREEN_UI::Theme* theme;
        void UIThemeInit();

        SCREEN_TextDrawer* Text() const { return textDrawer_; }

        void SetFontStyle(const SCREEN_UI::FontStyle& style);
        const SCREEN_UI::FontStyle& GetFontStyle() { return *fontStyle_; }
        void SetFontScale(float scaleX, float scaleY);
        void MeasureTextCount(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str, int count,
                              float* x, float* y, int align = 0) const;
        void MeasureText(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str, float* x, float* y,
                         int align = 0) const;
        void MeasureTextRect(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str, int count,
                             const Bounds& bounds, float* x, float* y, int align = 0) const;
        void DrawText(const char* str, float x, float y, uint32_t color, int align = 0);
        void DrawTextShadow(const char* str, float x, float y, uint32_t color, int align = 0);
        void DrawTextRect(const char* str, const Bounds& bounds, uint32_t color, int align = 0);
        void FillRect(const SCREEN_UI::Drawable& drawable, const Bounds& bounds);

        void SetBounds(const Bounds& b) { bounds_ = b; }
        const Bounds& GetBounds() const { return bounds_; }
        Bounds GetLayoutBounds() const;
        SCREEN_Draw::SCREEN_DrawContext* GetSCREEN_DrawContext() { return draw_; }

        Bounds TransformBounds(const Bounds& bounds);

    private:
        SCREEN_Draw::SCREEN_DrawContext* draw_;
        Bounds bounds_;

        float fontScaleX_ = 1.0f;
        float fontScaleY_ = 1.0f;
        SCREEN_UI::FontStyle* fontStyle_{nullptr};
        SCREEN_TextDrawer* textDrawer_{nullptr};

        SCREEN_Draw::SCREEN_SamplerState* sampler_;
        SCREEN_Draw::SCREEN_Pipeline* ui_pipeline_{nullptr};
        SCREEN_Draw::SCREEN_Pipeline* ui_pipeline_notex_{nullptr};
        std::unique_ptr<SCREEN_ManagedTexture> uitexture_;

        SCREEN_DrawBuffer* uidrawbuffer_{nullptr};
        SCREEN_DrawBuffer* uidrawbufferTop_{nullptr};

        std::vector<Bounds> scissorStack_;
        std::vector<UITransform> transformStack_;

        FontID m_Font;
    };
} // namespace GfxRenderEngine
