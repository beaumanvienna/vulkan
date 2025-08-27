/* Engine Copyright (c) 2025 Engine Development Team
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

#include "engine.h"
#include "renderer/texture.h"
namespace GfxRenderEngine
{

    // IBLBuilder: class to manage image-based lighting
    class IBLBuilder
    {
    public:
        enum IBLTexture
        {
            BRDFIntegrationMap = 0,
            environment,
            envPrefilteredDiffuse,
            envPrefilteredSpecularLevel0,
            envPrefilteredSpecularLevel1,
            envPrefilteredSpecularLevel2,
            envPrefilteredSpecularLevel3,
            envPrefilteredSpecularLevel4,
            envPrefilteredSpecularLevel5,
            NUM_IBL_IMAGES
        };
        using IBLTextureFilenames = std::array<std::string, IBLTexture::NUM_IBL_IMAGES>;

    public:
        IBLBuilder() = delete;
        IBLBuilder(IBLTextureFilenames const& filenames);
        bool IsInitialized() { return m_Initialized; }
        entt::entity LoadSkyboxHDRI(Registry& registry);

    private:
        // NUM_IBL_IMAGES: 9 images, but only BRDFint, env, prefilteredDiff, and prefilturedSpec (6 mip levels) as
        // textures
        static constexpr int NUM_OF_TEXTURES{4};
        static constexpr int NUM_MIP_LEVELS_SPECULAR{IBLTexture::NUM_IBL_IMAGES - IBLTexture::envPrefilteredSpecularLevel0};
        std::array<std::shared_ptr<Texture>, NUM_OF_TEXTURES> m_IBLTextures;
        bool m_Initialized;

    public:
        // HDRI skybox
        std::vector<Vertex> m_Vertices;
        std::vector<Submesh> m_Submeshes;
    };
} // namespace GfxRenderEngine
