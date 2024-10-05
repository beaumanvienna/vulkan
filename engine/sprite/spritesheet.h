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

#include <vector>

#include "engine.h"
#include "sprite/sprite.h"
#include "renderer/texture.h"
#include "resources/atlas/atlas.h"

namespace GfxRenderEngine
{
    struct AtlasImage
    {
        float u1, v1, u2, v2;
        int w, h;
        int rotation;
        char name[32];
    };

    struct Atlas
    {
        const AtlasImage* images = nullptr;
        const int num_images = 0;
    };

    class SpriteSheet
    {

    public:
        SpriteSheet();
        ~SpriteSheet();

        SpriteSheet(const Sprite&) = delete;
        SpriteSheet& operator=(const Sprite&) = delete;

        void AddSpritesheet();
        bool AddSpritesheet(const std::string& fileName);
        bool AddSpritesheet(const char* path /* GNU */, int resourceID /* MSVC */,
                            const std::string& resourceClass /* MSVC */);
        bool AddSpritesheetTile(const Sprite& originalSprite, const std::string& mapName, uint rows, uint columns,
                                uint spacing, const float scale = 1.0f);
        bool AddSpritesheetTile(const std::string& fileName, const std::string& mapName, uint rows, uint columns,
                                uint spacing, const float scale = 1.0f);
        bool AddSpritesheetTile(const char* path /* GNU */, int resourceID /* MSVC */,
                                const std::string& resourceClass /* MSVC */, const std::string& mapName, uint rows,
                                uint columns, uint spacing, const float scale = 1.0f);
        bool AddSpritesheetRow(const Sprite& originalSprite, uint frames, const float scaleX, const float scaleY);
        bool AddSpritesheetRow(const Sprite& originalSprite, uint frames, const float scale = 1.0f);
        bool AddSpritesheetRow(const std::string& fileName, uint frames, const float scaleX, const float scaleY);
        bool AddSpritesheetRow(const std::string& fileName, uint frames, const float scale = 1.0f);
        bool AddSpritesheetRow(const char* path /* GNU */, int resourceID /* MSVC */,
                               const std::string& resourceClass /* MSVC */, uint frames, const float scaleX,
                               const float scaleY);
        bool AddSpritesheetRow(const char* path /* GNU */, int resourceID /* MSVC */,
                               const std::string& resourceClass /* MSVC */, uint frames, const float scale = 1.0f);
        Sprite& GetSprite(uint index);
        void SetScale(const float scale);
        void ListSprites();
        std::shared_ptr<Texture> GetTexture() const { return m_Texture; }
        uint GetNumberOfSprites() const { return m_SpriteTable.size(); }
        uint GetRows() const { return m_Rows; }
        uint GetColumns() const { return m_Columns; }
        void BeginFrame() {}

    private:
        void AddSpritesheetTile(const std::string& mapName, uint rows, uint columns, uint spacing, const float scale);

    private:
        std::shared_ptr<Texture> m_Texture;
        std::vector<Sprite> m_SpriteTable;
        uint m_Rows, m_Columns;
        std::shared_ptr<Texture> m_TextureAtlas;
    };
} // namespace GfxRenderEngine
