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

#include <memory>
#include <string>

#include "engine.h"
#include "sprite/sprite.h"
#include "transform/matrix.h"
#include "renderer/texture.h"

namespace GfxRenderEngine
{
    // a 90-degree rotation cannot be achieved with UVs
    // since there are only two points and the picture only flips
    // a 90-degree rotation  must be done with four verticies

    Sprite::Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                   const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scale)
        : m_Pos1X(pos1X), m_Pos1Y(pos1Y), m_Pos2X(pos2X), m_Pos2Y(pos2Y), m_Width(width), m_Height(height),
          m_Texture(texture), m_ScaleX(scale), m_Name(name), m_ScaleY(scale), m_Rotated(false), m_IsValid(true)
    {
        SetTransform();
    }

    Sprite::Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                   const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scale,
                   const bool rotated)
        : m_Pos1X(pos1X), m_Pos1Y(pos1Y), m_Pos2X(pos2X), m_Pos2Y(pos2Y), m_Texture(texture), m_ScaleX(scale), m_Name(name),
          m_ScaleY(scale), m_Rotated(rotated), m_IsValid(true)
    {
        if (rotated)
        {
            m_Width = height;
            m_Height = width;
        }
        else
        {
            m_Width = width;
            m_Height = height;
        }
        SetTransform();
    }

    Sprite::Sprite(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                   const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scaleX,
                   const float scaleY, const bool rotated)
        : m_Pos1X(pos1X), m_Pos1Y(pos1Y), m_Pos2X(pos2X), m_Pos2Y(pos2Y), m_Texture(texture), m_Name(name), m_ScaleX(scaleX),
          m_ScaleY(scaleY), m_Rotated(rotated), m_IsValid(true)
    {
        if (rotated)
        {
            m_Width = height;
            m_Height = width;
        }
        else
        {
            m_Width = width;
            m_Height = height;
        }
        SetTransform();
    }

    Sprite::Sprite()
        : m_Pos1X(0.0f), m_Pos1Y(0.0f), m_Pos2X(0.0f), m_Pos2Y(0.0f), m_Texture(nullptr), m_Name(""), m_ScaleX(0.0f),
          m_ScaleY(0.0f), m_Rotated(false), m_IsValid(false), m_Transform(glm::mat4(1.0f))
    {
    }

    Sprite::~Sprite() {}

    const std::string& Sprite::GetName() const { return m_Name; }

    void Sprite::SetScale(const float scale)
    {
        m_ScaleX = m_ScaleY = scale;
        SetTransform();
    }

    void Sprite::SetScale(const float scaleX, const float scaleY)
    {
        if (m_Rotated)
        {
            m_ScaleX = scaleY;
            m_ScaleY = scaleX;
        }
        else
        {
            m_ScaleX = scaleX;
            m_ScaleY = scaleY;
        }
        SetTransform();
    }

    void Sprite::SetTransform()
    {
        glm::mat4 scale;
        glm::mat4 rotation;
        if (m_Rotated)
        {
            scale = Scale({m_ScaleX * m_Height / 2.0f, m_ScaleY * m_Width / 2.0f, 1.0f});
            rotation = Rotate(glm::half_pi<float>(), {0.0f, 0.0f, 1.0f});
        }
        else
        {
            rotation = glm::mat4(1.0f);
            scale = Scale({m_ScaleX * m_Width / 2.0f, m_ScaleY * m_Height / 2.0f, 1.0f});
        }
        m_Transform = rotation * scale;
    }

    void Sprite::Resize(uint width, uint height)
    {
        m_Width = width;
        m_Height = height;
        SetTransform();
    }

    float Sprite::GetAspectRatio() const { return static_cast<float>(m_Height) / static_cast<float>(m_Width); }

    // --------- Sprite2D ---------

    Sprite2D::Sprite2D(const Sprite& sprite) : Sprite()
    {
        m_Pos1X = sprite.m_Pos1X;
        m_Pos1Y = sprite.m_Pos1Y;
        m_Pos2X = sprite.m_Pos2X;
        m_Pos2Y = sprite.m_Pos2Y;
        m_Width = sprite.m_Width;
        m_Height = sprite.m_Height;
        m_Texture = sprite.m_Texture;
        m_Name = sprite.m_Name;
        m_ScaleX = sprite.m_ScaleX;
        m_ScaleY = sprite.m_ScaleY;
        m_Rotated = sprite.m_Rotated;
        m_IsValid = sprite.m_IsValid;
        m_Transform = sprite.m_Transform;
        FlipY();
    }

    Sprite2D::Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                       const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scale)
        : Sprite(pos1X, pos1Y, pos2X, pos2Y, width, height, texture, name, scale)
    {
        FlipY();
    }

    Sprite2D::Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                       const int height, const std::shared_ptr<Texture>& texture, const std::string& name, const float scale,
                       const bool rotated)
        : Sprite(pos1X, pos1Y, pos2X, pos2Y, width, height, texture, name, scale, rotated)
    {
        FlipY();
    }

    Sprite2D::Sprite2D(const float pos1X, const float pos1Y, const float pos2X, const float pos2Y, const int width,
                       const int height, const std::shared_ptr<Texture>& texture, const std::string& name,
                       const float scaleX, const float scaleY, const bool rotated)
        : Sprite(pos1X, pos1Y, pos2X, pos2Y, width, height, texture, name, scaleX, scaleY, rotated)
    {
        FlipY();
    }

    Sprite2D::Sprite2D() : Sprite() {}

    Sprite2D::~Sprite2D() {}

    void Sprite2D::FlipY()
    {
        if (m_Rotated)
        {
            std::swap(m_Pos1X, m_Pos2X);
        }
        else
        {
            std::swap(m_Pos1Y, m_Pos2Y);
        }
    }
} // namespace GfxRenderEngine
