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

#include "core.h"
#include "sprite/spritesheet.h"
#include "gui/Render/textureAtlas.h"
#include "gui/Common/Math/geom2d.h"
#include "renderer/renderer.h"

namespace GfxRenderEngine
{
#define COLOR(i) (((i & 0xFF) << 16) | (i & 0xFF00) | ((i & 0xFF0000) >> 16) | 0xFF000000)
    typedef unsigned int Color;

    struct SCREEN_Atlas;

    enum
    {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT = 16,
        ALIGN_TOP = 0,
        ALIGN_BOTTOM = 1,
        ALIGN_HCENTER = 4,
        ALIGN_VCENTER = 8,
        ALIGN_VBASELINE = 32,

        ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER,
        ALIGN_TOPLEFT = ALIGN_TOP | ALIGN_LEFT,
        ALIGN_TOPRIGHT = ALIGN_TOP | ALIGN_RIGHT,
        ALIGN_BOTTOMLEFT = ALIGN_BOTTOM | ALIGN_LEFT,
        ALIGN_BOTTOMRIGHT = ALIGN_BOTTOM | ALIGN_RIGHT,

        ROTATE_90DEG_LEFT = 256,
        ROTATE_90DEG_RIGHT = 512,
        ROTATE_180DEG = 1024,

        FLAG_DYNAMIC_ASCII = 2048,
        FLAG_NO_PREFIX = 4096,
        FLAG_WRAP_TEXT = 8192,
        FLAG_ELLIPSIZE_TEXT = 16384,
    };

    namespace SCREEN_Draw
    {
        class SCREEN_Pipeline;
    }

    struct GradientStop
    {
        float t;
        uint32_t color;
    };

    class SCREEN_TextDrawer;

    class SCREEN_DrawBuffer
    {
    public:
        SCREEN_DrawBuffer()
        {
            m_Renderer = Engine::m_Engine->GetRenderer();
            fontscalex = 1.0f;
            fontscaley = 1.0f;
        }
        ~SCREEN_DrawBuffer() {}

        bool MeasureImage(const Sprite& sprite, float& w, float& h);
        void DrawImage(const Sprite& sprite, float x, float y, float scale, Color color = COLOR(0xFFFFFF),
                       int align = ALIGN_TOPLEFT);
        void DrawImageStretch(const Sprite& sprite, float x1, float y1, float x2, float y2, Color color = COLOR(0xFFFFFF));
        void DrawImageStretch(const Sprite& sprite, const Bounds& bounds, Color color = COLOR(0xFFFFFF));
        void DrawWithTransform(const Sprite& sprite, const glm::mat4& transform);
        void DrawTexRect(std::shared_ptr<Texture> texture, float x1, float y1, float x2, float y2, float u1, float v1,
                         float u2, float v2, Color color);
        void DrawImage4Grid(const Sprite& sprite, float x1, float y1, float x2, float y2, Color color = COLOR(0xFFFFFF),
                            float corner_scale = 1.0);
        void MeasureText(FontID font, const char* text, float* w, float* h);
        void MeasureTextCount(FontID font, const char* text, int count, float* w, float* h);
        void MeasureTextRect(FontID font, const char* text, int count, const Bounds& bounds, float* w, float* h,
                             int align = 0);
        void DrawTextRect(FontID font, const char* text, float x, float y, float w, float h, Color color = 0xFFFFFFFF,
                          int align = 0);
        void DrawText(FontID font, const char* text, float x, float y, Color color = 0xFFFFFFFF, int align = 0);

        void SetFontScale(float xs, float ys);

        static void DoAlign(int flags, float* x, float* y, float* w, float* h);

        float fontscalex;
        float fontscaley;
        Renderer* m_Renderer;

    private:
        glm::vec4 ConvertColor(Color color);
    };
} // namespace GfxRenderEngine
