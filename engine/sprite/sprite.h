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

#include <memory>

#include "engine.h"
#include "renderer/texture.h"
#include "scene/components.h"

namespace GfxRenderEngine
{
    class SpriteSheet;
    class Sprite2D;
    class Sprite
    {

    public:
        Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width, const int height,
               const std::shared_ptr<Texture>& texture, const std::string& name, const float scale = 1.0f);

        Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width, const int height,
               const std::shared_ptr<Texture>& texture, const std::string& name, const float scale, const bool rotated);

        Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width, const int height,
               const std::shared_ptr<Texture>& texture, const std::string& name, const float scaleX, const float scaleY,
               const bool rotated = false);

        Sprite();
        virtual ~Sprite();
        Sprite(const Sprite&) = default;
        Sprite& operator=(const Sprite&) = default;

        const std::string& GetName() const;
        const glm::mat4& GetMat4() const { return m_Transform; }
        void SetScale(const float scale);
        void SetScale(const float scaleX, const float scaleY);
        float GetWidth() const { return static_cast<float>(m_Width) * m_ScaleX; }
        float GetHeight() const { return static_cast<float>(m_Height) * m_ScaleY; }
        void Resize(uint width, uint height);
        float GetAspectRatio() const;
        bool IsValid() const { return m_IsValid; }

        float m_Pos1X, m_Pos1Y, m_Pos2X, m_Pos2Y;

        std::shared_ptr<Texture> m_Texture;

    private:
        void SetTransform();
        bool IsRotated() const { return m_Rotated; }

    protected:
        int m_Width, m_Height;
        std::string m_Name;
        float m_ScaleX;
        float m_ScaleY;
        bool m_Rotated, m_IsValid;
        glm::mat4 m_Transform;

    private:
        friend class SpriteSheet;
        friend class Sprite2D;
    };

    // --------- Sprite2D ---------
    // a wrapper for sprite with
    // pos1Y and pos2Y flipped
    // to support a camera in the
    // opposite up direction

    class Sprite2D : public Sprite
    {

    public:
        Sprite2D(const Sprite& sprite);
        virtual ~Sprite2D();

        Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                 const int height, const std::shared_ptr<Texture>& texture, const std::string& name,
                 const float scale = 1.0f);

        Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                 const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scale,
                 const bool rotated);

        Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                 const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scaleX,
                 const float scaleY, const bool rotated = false);

        Sprite2D();

    private:
        void FlipY();
    };
} // namespace GfxRenderEngine
