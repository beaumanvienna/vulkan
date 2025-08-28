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

#include <future>

#include "core.h"
#include "scene/skyboxHDRIMaterial.h"
#include "renderer/builder/IBLBuilder.h"
#include "renderer/hiResImage.h"

namespace GfxRenderEngine
{
    IBLBuilder::IBLBuilder(IBLTextureFilenames const& filenames) : m_Initialized{false}
    {
        ThreadPool& threadPool = Engine::m_Engine->m_PoolSecondary;
        std::vector<std::future<bool>> loadFuturesOneMip;

        { // textures with one mip level
            auto iblTextures = std::to_array({BRDFIntegrationMap, environment, envPrefilteredDiffuse});
            loadFuturesOneMip.resize(iblTextures.size());

            for (uint index = 0; auto ibltexture : iblTextures)
            {
                auto& texture = m_IBLTextures[ibltexture];
                auto& filename = filenames[ibltexture];
                auto loadHiResImageAndCreateTexture = [&]()
                {
                    // vector with size == 1 to satisfy the interface of Texture
                    std::vector<HiResImage> hiResImages(1 /* size = 1*/);
                    auto& hiResImage = hiResImages[0];
                    hiResImage.Init(filename);
                    if (!hiResImage.IsInitialized())
                    {
                        return false;
                    }

                    texture = Texture::Create();
                    bool textureOk = texture->Init(hiResImages);
                    if (!textureOk)
                    {
                        return false;
                    }
                    LOG_APP_INFO("loaded {0}", hiResImage.GetFilename());
                    return true;
                };
                loadFuturesOneMip[index] = threadPool.SubmitTask(loadHiResImageAndCreateTexture);
                ++index;
            }
        }

        { // envPrefilteredSpecular with NUM_MIP_LEVELS_SPECULAR mip levels
            std::vector<HiResImage> hiResImages(NUM_MIP_LEVELS_SPECULAR);
            std::vector<std::future<bool>> loadFuturesSpecularImages(NUM_MIP_LEVELS_SPECULAR);
            for (uint index = 0; auto& hiResImage : hiResImages)
            {
                auto& filename = filenames[IBLTexture::envPrefilteredSpecularLevel0 + index];
                auto loadHiResImage = [&]()
                {
                    hiResImage.Init(filename);
                    if (!hiResImage.IsInitialized())
                    {
                        return false;
                    }
                    LOG_APP_INFO("loaded {0}", hiResImage.GetFilename());
                    return true;
                };
                loadFuturesSpecularImages[index] = threadPool.SubmitTask(loadHiResImage);
                ++index;
            }

            // wait for all PrefilteredSpecular images to be loaded from disk
            // before creating the texture with mip levels
            for (auto& future : loadFuturesSpecularImages)
            {
                if (!future.get())
                {
                    return;
                }
            }

            auto& texture = m_IBLTextures[IBLTexture::envPrefilteredSpecularLevel0];
            texture = Texture::Create();
            bool textureOk = texture->Init(hiResImages);
            if (!textureOk)
            {
                return;
            }
        }

        for (auto& future : loadFuturesOneMip)
        {
            if (!future.get())
            {
                return;
            }
        }

        { // create resource descriptor
            std::vector<std::shared_ptr<Texture>> textures = {m_IBLTextures[IBLTexture::envPrefilteredDiffuse],
                                                              m_IBLTextures[IBLTexture::envPrefilteredSpecularLevel0],
                                                              m_IBLTextures[IBLTexture::BRDFIntegrationMap]};
            m_ResourceDescriptor = ResourceDescriptor::Create(ResourceDescriptor::ResourceType::RtIBL, textures);
        }

        m_Initialized = true;
    }

    entt::entity IBLBuilder::LoadSkyboxHDRI(Registry& registry)
    {
        if (!m_Initialized)
        {
            LOG_CORE_CRITICAL("IBLBuilder::LoadSkyboxHDRI() not initialized!");
            return entt::null;
        }
        ZoneScopedN("Builder::LoadSkyboxHDRI()");
        entt::entity entity{entt::null};

        m_Vertices.clear();
        // Cube vertices for skybox (NDC directions)

        auto skyboxVertices = std::to_array<glm::vec3>({// positions
                                                        {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
                                                        {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},

                                                        {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
                                                        {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},

                                                        {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
                                                        {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},

                                                        {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
                                                        {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},

                                                        {-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},
                                                        {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f},

                                                        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},
                                                        {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f}});

        // create vertices
        {
            m_Vertices.reserve(skyboxVertices.size());
            for (auto& skyboxVertex : skyboxVertices)
            {
                m_Vertices.emplace_back(
                    /*pos*/ std::move(skyboxVertex), // emplace_back needs std::move
                    /*col*/ glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
                    /*norm*/ glm::vec3{0.0f, 0.0f, 0.0f},
                    /*uv*/ glm::vec2{0.0f, 0.0f});
            }
        }

        {
            Submesh submesh{};
            submesh.m_FirstVertex = 0;
            submesh.m_VertexCount = skyboxVertices.size();
            auto material = std::make_shared<SkyboxHDRIMaterial>();
            submesh.m_Material = material;
            { // create material descriptor
                auto materialDescriptor =
                    MaterialDescriptor::Create(Material::MtSkyboxHDRI, m_IBLTextures[IBLTexture::environment]);
                material->m_MaterialDescriptor = materialDescriptor;
            }

            m_Submeshes.push_back(submesh);
        }

        // create game object
        {
            auto model = Engine::m_Engine->LoadModel(*this);
            entity = registry.Create();
            MeshComponent mesh{"skyboxHDRI", model};
            registry.emplace<MeshComponent>(entity, mesh);
            TransformComponent transform{};
            registry.emplace<TransformComponent>(entity, transform);
            SkyboxHDRIComponent skyboxHDRIComponent{};
            registry.emplace<SkyboxHDRIComponent>(entity, skyboxHDRIComponent);
        }

        return entity;
    }

} // namespace GfxRenderEngine
