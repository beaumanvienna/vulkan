/* Engine Copyright (c) 2023 Engine Development Team
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

#include "renderer/model.h"

namespace GfxRenderEngine
{
    class Builder
    {

    public:
        Builder() = default;

        void LoadParticle(const glm::vec4& color);
        void LoadSprite(Sprite const& sprite, float const amplification = 0.0f, int const unlit = 0,
                        glm::vec4 const& color = glm::vec4(1.0f));
        entt::entity LoadCubemap(const std::vector<std::string>& faces, entt::registry& registry);

    private:
        void CalculateTangents();
        void CalculateTangentsFromIndexBuffer(const std::vector<uint>& indices);

    public:
        std::vector<uint> m_Indices{};
        std::vector<Vertex> m_Vertices{};
        std::vector<Submesh> m_Submeshes{};

        // cubemap
        std::vector<std::shared_ptr<Cubemap>> m_Cubemaps;
        std::vector<Submesh> m_CubemapSubmeshes{};
    };
} // namespace GfxRenderEngine
