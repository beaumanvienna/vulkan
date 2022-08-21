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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <memory>
#include <string>

#include "engine.h"
#include "sprite/sprite.h"
#include "transform/matrix.h"
#include "renderer/texture.h"

namespace GfxRenderEngine
{

    Sprite::Sprite(
        const float pos1X, const float pos1Y, 
        const float pos2X, const float pos2Y,
        const int  width, const int  height,
        const std::shared_ptr<Texture> texture,
        const std::string& name,
        const float scale) :
                m_Pos1X(pos1X), m_Pos1Y(pos1Y), 
                m_Pos2X(pos2X), m_Pos2Y(pos2Y),
                m_Width(width), m_Height(height),
                m_Texture(texture), m_ScaleX(scale),
                m_Name(name), m_ScaleY(scale), 
                m_Rotated(false)
    {
        SetScaleMatrix();
    }

    Sprite::Sprite(
        const float pos1X, const float pos1Y, 
        const float pos2X, const float pos2Y,
        const int  width, const int  height,
        const std::shared_ptr<Texture> texture,
        const std::string& name,
        const float scale,
        const bool rotated) :
                m_Pos1X(pos1X), m_Pos1Y(pos1Y), 
                m_Pos2X(pos2X), m_Pos2Y(pos2Y),
                m_Texture(texture), m_ScaleX(scale),
                m_Name(name), m_ScaleY(scale), 
                m_Rotated(rotated)
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
        
        SetScaleMatrix();
    }

    Sprite::Sprite(
        const float pos1X, const float pos1Y, 
        const float pos2X, const float pos2Y,
        const int  width, const int  height,
        const std::shared_ptr<Texture> texture,
        const std::string& name,
        const float scaleX, const float scaleY,
        const bool rotated) :
                m_Pos1X(pos1X), m_Pos1Y(pos1Y), 
                m_Pos2X(pos2X), m_Pos2Y(pos2Y),
                m_Texture(texture), m_Name(name),
                m_ScaleX(scaleX), m_ScaleY(scaleY),
                m_Rotated(rotated)
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
        SetScaleMatrix();
    }

    std::string& Sprite::GetName()
    {
        return m_Name;
    }

    void Sprite::SetScale(const float scale)
    {
        m_ScaleX = m_ScaleY = scale;
        SetScaleMatrix();
    }

    void Sprite::SetScale(const float scaleX, const float scaleY)
    {
        m_ScaleX = scaleX;
        m_ScaleY = scaleY;
        SetScaleMatrix();
    }

    void Sprite::SetScaleMatrix()
    {
        m_Transform.SetScale({m_ScaleX, m_ScaleY * GetAspectRatio(), 1.0f});

        if (m_Rotated)
        {
            m_Transform.SetRotationZ(glm::half_pi<float>());
        }
    }

    TransformComponent& Sprite::GetTransform()
    {
        m_Transform.GetMat4();
        return m_Transform;
    }

    float Sprite::GetWidthGUI() const
    {
        return static_cast<float>(m_Width) * m_ScaleX;
    }

    float Sprite::GetHeightGUI() const
    {
        return static_cast<float>(m_Height) * m_ScaleY;
    }

    void Sprite::Resize(uint width, uint height)
    {
        m_Width = width;
        m_Height = height;
        SetScaleMatrix();
    }

    float Sprite::GetAspectRatio() const
    {
        return static_cast<float>(m_Height) / static_cast<float>(m_Width);
    }
}
