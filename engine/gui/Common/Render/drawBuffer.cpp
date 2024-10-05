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

#include <algorithm>

#include "core.h"
#include "gui/common.h"
#include "gui/Common/UI/screen.h"
#include "gui/Render/textureAtlas.h"
#include "gui/Common/Render/drawBuffer.h"
#include "gui/Common/Data/Text/wrapText.h"
#include "gui/Common/Data/Text/utf8.h"
#include "gui/Common/stringUtils.h"

namespace GfxRenderEngine
{
    bool SCREEN_DrawBuffer::MeasureImage(const Sprite& sprite, float& w, float& h)
    {
        w = sprite.GetWidth();
        h = sprite.GetHeight();
        return true;
    }

    void SCREEN_DrawBuffer::DrawImage(const Sprite& sprite, float x, float y, float scale, Color color, int align)
    {
        if (!sprite.IsValid())
        {
            return;
        }
        float w = sprite.GetWidth() * scale;
        float h = sprite.GetHeight() * scale;
        if (align & ALIGN_HCENTER)
            x -= w / 2;
        if (align & ALIGN_RIGHT)
            x -= w;
        if (align & ALIGN_VCENTER)
            y -= h / 2;
        if (align & ALIGN_BOTTOM)
            y -= h;
        DrawImageStretch(sprite, x, y, x + w, y + h, color);
    }

    glm::vec4 SCREEN_DrawBuffer::ConvertColor(Color color)
    {
        int red, green, blue, aplha;
        aplha = (color & 0xFF000000) >> 24;
        blue = (color & 0x00FF0000) >> 16;
        green = (color & 0x0000FF00) >> 8;
        red = (color & 0x000000FF) >> 0;
        glm::vec4 colorVec{static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f,
                           static_cast<float>(blue) / 255.0f, static_cast<float>(aplha) / 255.0f};
        return colorVec;
    }

    void SCREEN_DrawBuffer::DrawImageStretch(const Sprite& sprite, float x1, float y1, float x2, float y2, Color color)
    {
        glm::vec4 colorVec = ConvertColor(color);
        glm::mat4 position;

        position[0][0] = x1;
        position[1][0] = y1;
        position[0][1] = x2;
        position[1][1] = y1;
        position[0][2] = x2;
        position[1][2] = y2;
        position[0][3] = x1;
        position[1][3] = y2;

        m_Renderer->Draw(sprite, position, colorVec);
    }

    void SCREEN_DrawBuffer::DrawWithTransform(const Sprite& sprite, const glm::mat4& transform)
    {
        m_Renderer->DrawWithTransform(sprite, transform);
    }

    void SCREEN_DrawBuffer::DrawTexRect(std::shared_ptr<Texture> texture, float x1, float y1, float x2, float y2, float u1,
                                        float v1, float u2, float v2, Color color)
    {

        glm::vec4 colorVec = ConvertColor(color);
        glm::mat4 position;

        position[0][0] = x1;
        position[1][0] = y1;
        position[0][1] = x2;
        position[1][1] = y1;
        position[0][2] = x2;
        position[1][2] = y2;
        position[0][3] = x1;
        position[1][3] = y2;

        Sprite sprite = Sprite(u1, v1, u2, v2,
                               0,       // width not needed
                               0,       // height not needed
                               nullptr, // texture is default atlas at the moment
                               "");

        m_Renderer->Draw(sprite, position, colorVec);
    }

    void SCREEN_DrawBuffer::DrawImage4Grid(const Sprite& sprite, float x1, float y1, float x2, float y2, Color color,
                                           float corner_scale)
    {
        if (!sprite.IsValid())
        {
            return;
        }
        float u1 = sprite.m_Pos1X;
        float v1 = sprite.m_Pos1Y;
        float u2 = sprite.m_Pos2X;
        float v2 = sprite.m_Pos2Y;
        float um = (u2 + u1) * 0.5f;
        float vm = (v2 + v1) * 0.5f;
        float iw2 = (sprite.GetWidth() * 0.5f) * corner_scale;
        float ih2 = (sprite.GetHeight() * 0.5f) * corner_scale;
        float xa = x1 + iw2;
        float xb = x2 - iw2;
        float ya = y1 + ih2;
        float yb = y2 - ih2;
        // Top row
        DrawTexRect(sprite.m_Texture, x1, y1, xa, ya, u1, v1, um, vm, color);
        DrawTexRect(sprite.m_Texture, xa, y1, xb, ya, um, v1, um, vm, color);
        DrawTexRect(sprite.m_Texture, xb, y1, x2, ya, um, v1, u2, vm, color);
        // Middle row
        DrawTexRect(sprite.m_Texture, x1, ya, xa, yb, u1, vm, um, vm, color);
        DrawTexRect(sprite.m_Texture, xa, ya, xb, yb, um, vm, um, vm, color);
        DrawTexRect(sprite.m_Texture, xb, ya, x2, yb, um, vm, u2, vm, color);
        // Bottom row
        DrawTexRect(sprite.m_Texture, x1, yb, xa, y2, u1, vm, um, v2, color);
        DrawTexRect(sprite.m_Texture, xa, yb, xb, y2, um, vm, um, v2, color);
        DrawTexRect(sprite.m_Texture, xb, yb, x2, y2, um, vm, u2, v2, color);
    }

    class SCREEN_AtlasWordWrapper : public SCREEN_WordWrapper
    {
    public:
        SCREEN_AtlasWordWrapper(const SCREEN_AtlasFont& atlasfont, float scale, const char* str, float maxW, int flags)
            : SCREEN_WordWrapper(str, maxW, flags), atlasfont_(atlasfont), scale_(scale)
        {
        }

    protected:
        float MeasureWidth(const char* str, size_t bytes) override;

        const SCREEN_AtlasFont& atlasfont_;
        const float scale_;
    };

    float SCREEN_AtlasWordWrapper::MeasureWidth(const char* str, size_t bytes)
    {
        float w = 0.0f;
        for (SCREEN_UTF8 utf(str); utf.byteIndex() < (int)bytes;)
        {
            uint32_t c = utf.next();
            if (c == '&')
            {
                c = utf.next();
            }
            const AtlasChar* ch = atlasfont_.getChar(c);
            if (!ch)
                ch = atlasfont_.getChar('?');

            w += ch->wx * scale_;
        }
        return w;
    }

    void SCREEN_DrawBuffer::MeasureTextCount(FontID font, const char* text, int count, float* w, float* h)
    {
        const SCREEN_AtlasFont* atlasfont = ui_atlas.getFont(font);
        if (!atlasfont)
        {
            *w = 0.0f;
            *h = 0.0f;
            return;
        }

        unsigned int cval;
        float wacc = 0;
        float maxX = 0.0f;
        int lines = 1;
        SCREEN_UTF8 utf(text);
        while (true)
        {
            if (utf.end())
            {
                break;
            }
            if (utf.byteIndex() >= count)
            {
                break;
            }
            cval = utf.next();

            if (cval == 0xA0)
            {
                cval = ' ';
            }
            else if (cval == '\n')
            {
                maxX = std::max(maxX, wacc);
                wacc = 0;
                lines++;
                continue;
            }
            else if (cval == '\t')
            {
                cval = ' ';
            }
            else if (cval == '&' && utf.peek() != '&')
            {
                continue;
            }
            const AtlasChar* c = atlasfont->getChar(cval);
            if (c)
            {
                wacc += c->wx * fontscalex;
            }
        }
        if (w)
            *w = std::max(wacc, maxX);
        if (h)
            *h = atlasfont->height * fontscaley * lines;
    }

    void SCREEN_DrawBuffer::MeasureTextRect(FontID font_id, const char* text, int count, const Bounds& bounds, float* w,
                                            float* h, int align)
    {
        if (!text || font_id.isInvalid())
        {
            *w = 0.0f;
            *h = 0.0f;
            return;
        }

        std::string toMeasure = std::string(text, count);
        int wrap = align & (FLAG_WRAP_TEXT | FLAG_ELLIPSIZE_TEXT);
        if (wrap)
        {
            const SCREEN_AtlasFont* font = ui_atlas.getFont(font_id);
            if (!font)
            {
                *w = 0.0f;
                *h = 0.0f;
                return;
            }
            SCREEN_AtlasWordWrapper wrapper(*font, fontscalex, toMeasure.c_str(), bounds.w, wrap);
            toMeasure = wrapper.Wrapped();
        }
        MeasureTextCount(font_id, toMeasure.c_str(), (int)toMeasure.length(), w, h);
    }

    void SCREEN_DrawBuffer::MeasureText(FontID font, const char* text, float* w, float* h)
    {
        return MeasureTextCount(font, text, (int)strlen(text), w, h);
    }

    void SCREEN_DrawBuffer::DoAlign(int flags, float* x, float* y, float* w, float* h)
    {
        if (flags & ALIGN_HCENTER)
            *x -= *w / 2;
        if (flags & ALIGN_RIGHT)
            *x -= *w;
        if (flags & ALIGN_VCENTER)
            *y -= *h / 2;
        if (flags & ALIGN_BOTTOM)
            *y -= *h;
        if (flags & (ROTATE_90DEG_LEFT | ROTATE_90DEG_RIGHT))
        {
            std::swap(*w, *h);
            std::swap(*x, *y);
        }
    }

    void SCREEN_DrawBuffer::DrawTextRect(FontID font, const char* text, float x, float y, float w, float h, Color color,
                                         int align)
    {
        if (align & ALIGN_HCENTER)
        {
            x += w / 2;
        }
        else if (align & ALIGN_RIGHT)
        {
            x += w;
        }

        if (align & ALIGN_VCENTER)
        {
            y += h / 2;
        }
        else if (align & ALIGN_BOTTOM)
        {
            y += h;
        }

        std::string toDraw = text;
        int wrap = align & (FLAG_WRAP_TEXT | FLAG_ELLIPSIZE_TEXT);
        const SCREEN_AtlasFont* atlasfont = ui_atlas.getFont(font);
        if (wrap && atlasfont)
        {
            SCREEN_AtlasWordWrapper wrapper(*atlasfont, fontscalex, toDraw.c_str(), w, wrap);
            toDraw = wrapper.Wrapped();
        }

        float totalWidth, totalHeight;
        MeasureTextRect(font, toDraw.c_str(), (int)toDraw.size(), Bounds(x, y, w, h), &totalWidth, &totalHeight, align);

        std::vector<std::string> lines;
        SCREEN_PSplitString(toDraw, '\n', lines);

        float baseY = y;
        if (align & ALIGN_VCENTER)
        {
            baseY -= totalHeight / 2;
            align = align & ~ALIGN_VCENTER;
        }
        else if (align & ALIGN_BOTTOM)
        {
            baseY -= totalHeight;
            align = align & ~ALIGN_BOTTOM;
        }

        for (const std::string& line : lines)
        {
            DrawText(font, line.c_str(), x, baseY, color, align);

            float tw, th;
            MeasureText(font, line.c_str(), &tw, &th);
            baseY += th;
        }
    }

    void SCREEN_DrawBuffer::DrawText(FontID font, const char* text, float x, float y, Color color, int align)
    {
        size_t textLen = strlen(text);

        if (!textLen)
        {
            return;
        }

        const SCREEN_AtlasFont* atlasfont = ui_atlas.getFont(font);
        if (!atlasfont)
        {
            return;
        }
        unsigned int cval;
        float w, h;
        MeasureText(font, text, &w, &h);

        if (align)
        {
            DoAlign(align, &x, &y, &w, &h);
        }

        if (align & ROTATE_90DEG_LEFT)
        {
            x -= atlasfont->ascend * fontscaley;
            // y += h;
        }
        else
        {
            y += atlasfont->ascend * fontscaley;
        }

        float sx = x;
        SCREEN_UTF8 utf(text);

        for (size_t i = 0; i < textLen; i++)
        {
            if (utf.end())
            {
                break;
            }
            cval = utf.next();

            if (cval == 0xA0)
            {
                cval = ' ';
            }
            else if (cval == '\n')
            {
                y += atlasfont->height * fontscaley;
                x = sx;
                continue;
            }
            else if (cval == '\t')
            {
                cval = ' ';
            }
            else if (cval == '&' && utf.peek() != '&')
            {
                continue;
            }
            const AtlasChar* ch = atlasfont->getChar(cval);
            if (!ch)
            {
                ch = atlasfont->getChar('?');
            }
            else
            {
                const AtlasChar& c = *ch;
                float cx1, cy1, cx2, cy2;
                if (align & ROTATE_90DEG_LEFT)
                {
                    cy1 = y - c.ox * fontscalex;
                    cx1 = x + c.oy * fontscaley;
                    cy2 = y - (c.ox + c.pw) * fontscalex;
                    cx2 = x + (c.oy + c.ph) * fontscaley;
                }
                else
                {
                    cx1 = x + c.ox * fontscalex;
                    cy1 = y + c.oy * fontscaley;
                    cx2 = x + (c.ox + c.pw) * fontscalex;
                    cy2 = y + (c.oy + c.ph) * fontscaley;
                }

                float textureID = 2.0f;
                glm::mat4 position;

                position[0][0] = cx1;
                position[1][0] = cy1;
                position[0][1] = cx2;
                position[1][1] = cy1;
                position[0][2] = cx2;
                position[1][2] = cy2;
                position[0][3] = cx1;
                position[1][3] = cy2;

                glm::vec4 textureCoordinates{c.sx, 1.0f - c.sy, c.ex, 1.0f - c.ey};
                glm::vec4 colorVec = ConvertColor(color);

                Sprite sprite = Sprite(c.sx, 1.0f - c.sy, c.ex, 1.0f - c.ey,
                                       0,       // width not needed
                                       0,       // height not needed
                                       nullptr, // texture is default atlas at the moment
                                       "");

                m_Renderer->Draw(sprite, position, colorVec, textureID);

                if (align & ROTATE_90DEG_LEFT)
                {
                    y -= c.wx * fontscalex;
                }
                else
                {
                    x += c.wx * fontscalex;
                }
            }
        }
    }

    void SCREEN_DrawBuffer::DrawImageStretch(const Sprite& sprite, const Bounds& bounds, Color color)
    {
        DrawImageStretch(sprite, bounds.x, bounds.y, bounds.x2(), bounds.y2(), color);
    }

    void SCREEN_DrawBuffer::SetFontScale(float xs, float ys)
    {
        fontscalex = xs;
        fontscaley = ys;
    }
} // namespace GfxRenderEngine
