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

#include "gui/common.h"
#include "transform/matrix.h"
#include "gui/Common/UI/screen.h"
#include "gui/Common/UI/context.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Render/textureAtlas.h"

namespace GfxRenderEngine
{

    inline SCREEN_UI::Style MakeStyle(uint32_t fg, uint32_t bg)
    {
        SCREEN_UI::Style s;
        s.background = SCREEN_UI::Drawable(bg);
        s.fgColor = fg;

        return s;
    }

    SCREEN_UIContext::SCREEN_UIContext()
    {
        fontStyle_ = new SCREEN_UI::FontStyle();
        uidrawbuffer_ = new SCREEN_DrawBuffer();

        UIThemeInit();

        ui_theme.checkOn = SCREEN_ScreenManager::m_SpritesheetUI->GetSprite(I_CHECKEDBOX);
        ui_theme.checkOff = SCREEN_ScreenManager::m_SpritesheetUI->GetSprite(I_SQUARE);
        ui_theme.whiteImage = SCREEN_ScreenManager::m_SpritesheetUI->GetSprite(I_WHITE);
        ui_theme.sliderKnob = SCREEN_ScreenManager::m_SpritesheetUI->GetSprite(I_CIRCLE);
        ui_theme.dropShadow4Grid = SCREEN_ScreenManager::m_SpritesheetUI->GetSprite(I_DROP_SHADOW);

        theme = &ui_theme;
    }

    SCREEN_UIContext::~SCREEN_UIContext()
    {
        delete fontStyle_;
        delete uidrawbuffer_;
    }

    void SCREEN_UIContext::UIThemeInit()
    {
        bounds_ = Bounds(0, 0, Engine::m_Engine->GetWindowWidth(), Engine::m_Engine->GetWindowHeight());
        if (CoreSettings::m_UITheme == THEME_RETRO)
        {
            ui_theme.uiFont = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 22);
            ui_theme.uiFontSmall = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 18);
            ui_theme.uiFontSmaller = SCREEN_UI::FontStyle(FontID("RETRO24"), "", 8);

            ui_theme.itemStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x80000000);
            ui_theme.itemFocusedStyle = MakeStyle(0xFFFFFFFF, 0xA0000000); // active icons
            ui_theme.itemDownStyle = MakeStyle(0xFFFFFFFF, 0xB0000000);
            ui_theme.itemDisabledStyle = MakeStyle(0xffEEEEEE, 0x55E0D4AF);
            ui_theme.itemHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55ffffff); //

            ui_theme.buttonStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x70000000); // inactive button
            ui_theme.buttonFocusedStyle =
                MakeStyle(RETRO_COLOR_FONT_FOREGROUND, RETRO_COLOR_FONT_BACKGROUND); // active button
            ui_theme.buttonDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
            ui_theme.buttonDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
            ui_theme.buttonHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

            ui_theme.headerStyle.fgColor = RETRO_COLOR_FONT_FOREGROUND;
            ui_theme.infoStyle = MakeStyle(RETRO_COLOR_FONT_FOREGROUND, 0x00000000U);

            ui_theme.popupTitle.fgColor = RETRO_COLOR_FONT_FOREGROUND;
            ui_theme.popupStyle = MakeStyle(0xFFFFFFFF, 0xFF303030);

            m_Font = FontID{"RETRO24"};
        }
        else
        {
            ui_theme.uiFont = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 26);
            ui_theme.uiFontSmall = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 24);
            ui_theme.uiFontSmaller = SCREEN_UI::FontStyle(FontID("UBUNTU24"), "", 24);

            ui_theme.itemStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
            ui_theme.itemFocusedStyle = MakeStyle(0xFFFFFFFF, 0xA0000000);
            ui_theme.itemDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
            ui_theme.itemDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
            ui_theme.itemHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

            ui_theme.buttonStyle = MakeStyle(0xFFFFFFFF, 0x55000000);
            ui_theme.buttonFocusedStyle = MakeStyle(0xFFFFFFFF, 0xB0000000);
            ui_theme.buttonDownStyle = MakeStyle(0xFFFFFFFF, 0xFFBD9939);
            ui_theme.buttonDisabledStyle = MakeStyle(0x80EEEEEE, 0x55E0D4AF);
            ui_theme.buttonHighlightedStyle = MakeStyle(0xFFFFFFFF, 0x55BDBB39);

            ui_theme.headerStyle.fgColor = 0xFFFFFFFF;
            ui_theme.infoStyle = MakeStyle(0xFFFFFFFF, 0x00000000U);

            ui_theme.popupTitle.fgColor = 0xFFE3BE59;
            ui_theme.popupStyle = MakeStyle(0xFFFFFFFF, 0xFF303030);

            m_Font = FontID{"UBUNTU24"};
        }
    }

    // void SCREEN_UIContext::Init(SCREEN_Draw::SCREEN_DrawContext *thin3d, SCREEN_Draw::SCREEN_Pipeline *uipipe,
    //                             SCREEN_Draw::SCREEN_Pipeline *uipipenotex, SCREEN_DrawBuffer *uidrawbuffer,
    //                             SCREEN_DrawBuffer *uidrawbufferTop)
    //{
    //     using namespace SCREEN_Draw;
    //     draw_ = thin3d;
    //     sampler_ = draw_->CreateSamplerState({ SCREEN_TextureFilter::LINEAR, SCREEN_TextureFilter::LINEAR,
    //     SCREEN_TextureFilter::LINEAR }); ui_pipeline_ = uipipe; ui_pipeline_notex_ = uipipenotex; uidrawbuffer_ =
    //     uidrawbuffer; uidrawbufferTop_ = uidrawbufferTop; textDrawer_ = SCREEN_TextDrawer::Create(thin3d);  // May return
    //     nullptr if no implementation is available for this platform.
    // }
    //
    // void SCREEN_UIContext::BeginFrame()
    //{
    //     if (!uitexture_) {
    //         uitexture_ = CreateTextureFromFile(draw_, "ui_atlas.zim", ImageFileType::ZIM, false);
    //         _dbg_assert_msg_(uitexture_, "Failed to load ui_atlas.zim.\n\nPlace it in the directory \"assets\" under your
    //         PPSSPP directory.");
    //     }
    //     uidrawbufferTop_->SetCurZ(0.0f);
    //     uidrawbuffer_->SetCurZ(0.0f);
    //     ActivateTopScissor();
    // }
    //
    // void SCREEN_UIContext::Begin()
    //{
    //     BeginPipeline(ui_pipeline_, sampler_);
    // }
    //
    // void SCREEN_UIContext::BeginNoTex()
    //{
    //     draw_->BindSamplerStates(0, 1, &sampler_);
    //     UIBegin(ui_pipeline_notex_);
    // }
    //
    // void SCREEN_UIContext::BeginPipeline(SCREEN_Draw::SCREEN_Pipeline *pipeline, SCREEN_Draw::SCREEN_SamplerState
    // *samplerState)
    //{
    //     draw_->BindSamplerStates(0, 1, &samplerState);
    //     RebindTexture();
    //     UIBegin(pipeline);
    // }
    //
    // void SCREEN_UIContext::RebindTexture() const
    //{
    //     if (uitexture_)
    //         draw_->BindTexture(0, uitexture_->GetTexture());
    // }

    void SCREEN_UIContext::Flush()
    {
        /// RenderCommand::Flush();
    }

    // void SCREEN_UIContext::SetCurZ(float curZ)
    //{
    //     SCREEN_ui_draw2d.SetCurZ(curZ);
    //     SCREEN_ui_draw2d_front.SetCurZ(curZ);
    // }
    //
    void SCREEN_UIContext::PushScissor(const Bounds& bounds)
    {
        Flush();
        Bounds clipped = TransformBounds(bounds);
        if (scissorStack_.size())
        {
            clipped.Clip(scissorStack_.back());
        }
        else
        {
            clipped.Clip(bounds_);
        }
        scissorStack_.push_back(clipped);
        ActivateTopScissor();
    }

    void SCREEN_UIContext::PopScissor()
    {
        Flush();
        scissorStack_.pop_back();
        ActivateTopScissor();
    }

    Bounds SCREEN_UIContext::GetScissorBounds()
    {
        if (!scissorStack_.empty())
        {
            return scissorStack_.back();
        }
        else
        {
            return bounds_;
        }
    }

    Bounds SCREEN_UIContext::GetLayoutBounds() const
    {
        Bounds bounds = GetBounds();

        return bounds;
    }

    void SCREEN_UIContext::ActivateTopScissor()
    {
        return; // scissors deactivated
        Bounds bounds;
        if (scissorStack_.size())
        {
            // float scale_x = 1.0f;
            // float scale_y = 1.0f;
            bounds = scissorStack_.back();
            // int x = floorf(scale_x * bounds.x);
            // int y = Engine::m_Engine->GetWindowHeight() - floorf(scale_y * (bounds.y + bounds.h));
            // int w = std::max(0.0f, ceilf(scale_x * bounds.w));
            // int h = std::max(0.0f, ceilf(scale_y * bounds.h));
            // RenderCommand::SetScissor(x, y, w, h);
        }
        else
        {
            // RenderCommand::SetScissor(0, 0, Engine::m_Engine->GetWindowWidth(), Engine::m_Engine->GetWindowHeight());
        }
    }

    void SCREEN_UIContext::SetFontScale(float scaleX, float scaleY)
    {
        fontScaleX_ = scaleX * Engine::m_Engine->GetWindowHeight() / 1080.0f;
        fontScaleY_ = scaleY * Engine::m_Engine->GetWindowHeight() / 1080.0f;
    }

    void SCREEN_UIContext::SetFontStyle(const SCREEN_UI::FontStyle& fontStyle) { *fontStyle_ = fontStyle; }

    void SCREEN_UIContext::MeasureText(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str,
                                       float* x, float* y, int align) const
    {
        MeasureTextCount(style, scaleX, scaleY, str, (int)strlen(str), x, y, align);
    }

    void SCREEN_UIContext::MeasureTextCount(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str,
                                            int count, float* x, float* y, int align) const
    {
        float sizeFactor = (float)style.sizePts / 24.0f;
        Draw()->SetFontScale(scaleX * sizeFactor, scaleY * sizeFactor);
        Draw()->MeasureTextCount(style.atlasFont, str, count, x, y);
    }

    void SCREEN_UIContext::MeasureTextRect(const SCREEN_UI::FontStyle& style, float scaleX, float scaleY, const char* str,
                                           int count, const Bounds& bounds, float* x, float* y, int align) const
    {
        float sizeFactor = (float)style.sizePts / 24.0f;
        Draw()->SetFontScale(scaleX * sizeFactor, scaleY * sizeFactor);
        Draw()->MeasureTextRect(m_Font, str, count, bounds, x, y, align);
    }

    void SCREEN_UIContext::DrawText(const char* str, float x, float y, uint32_t color, int align)
    {
        float sizeFactor = (float)fontStyle_->sizePts / 24.0f;
        Draw()->SetFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
        Draw()->DrawText(fontStyle_->atlasFont, str, x, y, color, align);
    }

    // void SCREEN_UIContext::DrawTextShadow(const char *str, float x, float y, uint32_t color, int align)
    //{
    //     uint32_t alpha = (color >> 1) & 0xFF000000;
    //     DrawText(str, x + 2, y + 2, alpha, align);
    //     DrawText(str, x, y, color, align);
    // }
    //
    void SCREEN_UIContext::DrawTextRect(const char* str, const Bounds& bounds, uint32_t color, int align)
    {
        float sizeFactor = (float)fontStyle_->sizePts / 24.0f;
        Draw()->SetFontScale(fontScaleX_ * sizeFactor, fontScaleY_ * sizeFactor);
        Draw()->DrawTextRect(m_Font, str, bounds.x, bounds.y, bounds.w, bounds.h, color, align);
    }
    namespace MarleyApp
    {
        extern Sprite* whiteImage;
    }

    void SCREEN_UIContext::FillRect(const SCREEN_UI::Drawable& drawable, const Bounds& bounds)
    {
        if ((drawable.color & 0xFF000000) == 0)
        {
            return;
        }

        switch (drawable.type)
        {
            case SCREEN_UI::DRAW_SOLID_COLOR:
                uidrawbuffer_->DrawImageStretch(theme->whiteImage, bounds.x, bounds.y, bounds.x2(), bounds.y2(),
                                                drawable.color);
                break;
            case SCREEN_UI::DRAW_4GRID:
                // uidrawbuffer_->DrawImage4Grid(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(),
                // drawable.color);
                LOG_CORE_ERROR("not supported: case SCREEN_UI::DRAW_4GRID");
                break;
            case SCREEN_UI::DRAW_STRETCH_IMAGE:
                // uidrawbuffer_->DrawImageStretch(drawable.image, bounds.x, bounds.y, bounds.x2(), bounds.y2(),
                // drawable.color);
                LOG_CORE_ERROR("not supported: case SCREEN_UI::DRAW_STRETCH_IMAGE");
                break;
            case SCREEN_UI::DRAW_NOTHING:
            {
                break;
            }
        }
    }

    // void SCREEN_UIContext::PushTransform(const UITransform &transform)
    //{
    //     Flush();
    //
    //     using namespace SCREEN_Lin;
    //
    //     SCREEN_Matrix4x4 m = Draw()->GetDrawMatrix();
    //     const SCREEN_Vec3 &t = transform.translate;
    //     SCREEN_Vec3 scaledTranslate = SCREEN_Vec3(
    //         t.x * m.xx + t.y * m.xy + t.z * m.xz + m.xw,
    //         t.x * m.yx + t.y * m.yy + t.z * m.yz + m.yw,
    //         t.x * m.zx + t.y * m.zy + t.z * m.zz + m.zw);
    //
    //     m.translateAndScale(scaledTranslate, transform.scale);
    //     Draw()->PushDrawMatrix(m);
    //     Draw()->PushAlpha(transform.alpha);
    //
    //     transformStack_.push_back(transform);
    // }
    //
    // void SCREEN_UIContext::PopTransform()
    //{
    //     Flush();
    //
    //     transformStack_.pop_back();
    //
    //     Draw()->PopDrawMatrix();
    //     Draw()->PopAlpha();
    // }

    Bounds SCREEN_UIContext::TransformBounds(const Bounds& bounds)
    {
        if (!transformStack_.empty())
        {
            const UITransform t = transformStack_.back();
            Bounds translated = bounds.Offset(t.translate.x, t.translate.y);

            float scaledX = translated.x * t.scale.x;
            float scaledY = translated.y * t.scale.y;

            return Bounds(scaledX, scaledY, translated.w * t.scale.x, translated.h * t.scale.y);
        }

        return bounds;
    }
} // namespace GfxRenderEngine
