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

#pragma once

#include <memory>

#include "engine.h"
#include "sprite/sprite.h"

namespace GfxRenderEngine
{

    struct Vertex
    {
        glm::vec3 m_Position;
        glm::vec3 m_Color;
        glm::vec3 m_Normal;
        glm::vec2 m_UV;
        int m_TextureSlot;

        bool operator==(const Vertex& other) const;

    };

    struct Builder
    {
        std::vector<Vertex> m_Vertices{};
        std::vector<uint> m_Indices{};

        void LoadModel(const std::string& filepath);
        void LoadSprite(Sprite* sprite, const glm::mat4& position, const glm::vec4& color = glm::vec4(1.0f));
        void LoadParticle(const glm::vec4& color);
    };

    class Model
    {

    public:

        Model() {}
        virtual ~Model() {}

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        virtual void CreateVertexBuffers(const std::vector<Vertex>& vertices) = 0;
        virtual void CreateIndexBuffers(const std::vector<uint>& indices) = 0;

    };
}
