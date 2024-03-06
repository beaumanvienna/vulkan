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

#include "gui/Render/textureAtlas.h"
#include "resources/atlas/fontAtlas.h"
#include "resources/atlas/fontAtlas.cpp"

namespace GfxRenderEngine
{

    const SCREEN_AtlasFont* SCREEN_Atlas::getFont(FontID id) const
    {
        if (id.isInvalid())
        {
            return nullptr;
        }

        for (int i = 0; i < num_fonts; i++)
        {
            if (!strcmp(id.id, fonts[i]->name))
            {
                return fonts[i];
            }
        }
        return nullptr;
    }

    const AtlasChar* SCREEN_AtlasFont::getChar(int utf32) const
    {
        for (int i = 0; i < numRanges; i++)
        {
            if (utf32 >= ranges[i].start && utf32 < ranges[i].end)
            {
                const AtlasChar* c = &charData[ranges[i].result_index + utf32 - ranges[i].start];
                if (c->ex == 0 && c->ey == 0)
                {
                    return nullptr;
                }
                else
                {
                    return c;
                }
            }
        }
        return nullptr;
    }
} // namespace GfxRenderEngine
