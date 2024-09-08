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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/controllerEvent.h"
#include "scenes/cutScene.h"
#include "resources/resources.h"
#include "transform/matrix.h"

#include "lucre.h"

namespace LucreApp
{

    void CutScene::Start()
    {
        m_Renderer = Engine::m_Engine->GetRenderer();

        // create orthogonal camera

        OrthographicCameraComponent orthographicCameraComponent(1.0f /*m_XMag*/, 1.0f /*m_YMag*/, 2.0f /*m_ZNear*/,
                                                                -2.0f /*ZFar*/);
        m_CameraController = std::make_shared<CameraController>(orthographicCameraComponent);
        auto& camera = m_CameraController->GetCamera();
        auto position = glm::vec3(0.0f, 0.0f, 1.0f);
        auto direction = glm::vec3(0.0f, 0.0f, -1.0f);
        camera.SetViewDirection(position, direction);

        // walk
        m_SpritesheetWalk.AddSpritesheetRow(Lucre::m_Spritesheet->GetSprite(I_WALK), WALK_ANIMATION_SPRITES /* frames */
        );
        m_WalkAnimation.Create(150ms /* per frame */, &m_SpritesheetWalk);
        m_WalkAnimation.Start();
        for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
        {
            Builder builder{};

            Sprite2D sprite = Sprite2D(m_SpritesheetWalk.GetSprite(i));
            builder.LoadSprite(sprite);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"walk animation", model};
            mesh.m_Enabled = false;

            m_Guybrush[i] = m_Registry.Create();
            m_Registry.emplace<MeshComponent>(m_Guybrush[i], mesh);

            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(m_Guybrush[i], transform);
        }

        // beach
        {
            m_Beach = m_Registry.Create();

            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(m_Beach, transform);
        }

        // cloud
        {
            for (uint i = 0; i < 2; i++)
            {
                m_Clouds[i] = m_Registry.Create();

                TransformComponent transform{};
                m_Registry.emplace<TransformComponent>(m_Clouds[i], transform);
            }
        }
        Init();
    }

    void CutScene::ResetTimer() { m_StartTime = Engine::m_Engine->GetTime(); }

    // used at the beginning and to resize
    void CutScene::Init()
    {
        m_InitialPositionX = -static_cast<float>(Engine::m_Engine->GetWindowWidth()) * 0.1f;
        m_EndPositionX = static_cast<float>(Engine::m_Engine->GetWindowWidth()) * 1.1f;
        float windowWidth = static_cast<float>(Engine::m_Engine->GetWindowWidth());
        float windowHeight = static_cast<float>(Engine::m_Engine->GetWindowHeight());

        // walk
        float scaleHero = windowHeight * 0.08f / m_SpritesheetWalk.GetSprite(0).GetHeight();

        m_GuybrushWalkDelta = windowHeight * 0.16f;
        for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
        {
            auto sprite = Sprite2D(m_SpritesheetWalk.GetSprite(i));
            sprite.SetScale(scaleHero);
            float width = sprite.GetWidth();
            float height = sprite.GetHeight();

            auto& transform = m_Registry.get<TransformComponent>(m_Guybrush[i]);
            transform.SetScale({width, height, 0.0f});
            transform.SetTranslationY(windowHeight * 0.8f);
        }

        // set up scale for beach and clouds
        {
            // calc scale based on original height
            auto beachSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_BEACH));
            float spriteHeight = beachSprite.GetHeight();
            m_Scale = windowHeight / spriteHeight;

            m_BeachSprite = beachSprite;
            m_CloudSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_CLOUDS));

            // scale global sprites
            m_BeachSprite.SetScale(m_Scale);
            m_CloudSprite.SetScale(m_Scale);

            float spriteWidthClouds = m_CloudSprite.GetWidth();
            m_TranslationX0 = spriteWidthClouds / 2.0f;
            m_TranslationX1 = -spriteWidthClouds / 2.0f;
        }
        // beach
        {
            float spriteHeight = m_BeachSprite.GetHeight();

            auto& transform = m_Registry.get<TransformComponent>(m_Beach);
            transform = TransformComponent(m_BeachSprite.GetMat4());
            transform.SetTranslation(glm::vec3{windowWidth / 2.0f, windowHeight - spriteHeight / 2, 0.0f});
        }

        // clouds
        {
            for (uint i = 0; i < 2; i++)
            {
                auto& transform = m_Registry.get<TransformComponent>(m_Clouds[i]);
                transform = TransformComponent(m_CloudSprite.GetMat4());
            }
        }
    }

    void CutScene::MoveClouds(const Timestep& timestep)
    {
        float spriteWidth = m_CloudSprite.GetWidth();
        float spriteHeight = m_CloudSprite.GetHeight();

        float speed = 20.0f;

        m_TranslationX0 += timestep * speed;
        m_TranslationX1 += timestep * speed;

        if (m_TranslationX0 > spriteWidth * 1.5f)
        {
            m_TranslationX0 = -spriteWidth / 2.0f;
        }
        if (m_TranslationX1 > spriteWidth * 1.5f)
        {
            m_TranslationX1 = -spriteWidth / 2.0f;
        }

        // need to gloss over some rounding effects
        glm::mat4 glossOver = Scale(glm::vec3(1.01f)) * m_CloudSprite.GetMat4();
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Clouds[0]);
            transform = TransformComponent(glossOver);
            transform.SetTranslation(glm::vec3{m_TranslationX0, spriteHeight / 2.0f, 0.0f});
        }

        {
            auto& transform = m_Registry.get<TransformComponent>(m_Clouds[1]);
            transform = TransformComponent(glossOver);
            transform.SetTranslation(glm::vec3{m_TranslationX1, spriteHeight / 2.0f, 0.0f});
        }
    }

    void CutScene::Stop() {}

    void CutScene::OnUpdate(const Timestep& timestep)
    {
        if ((Engine::m_Engine->GetTime() - m_StartTime) > MIN_TIME_IN_CUTSCENE)
        {
            m_IsRunning = false;
        }

        // walk animation
        {
            static float walkOffset = m_InitialPositionX;
            if (!m_WalkAnimation.IsRunning())
            {
                m_WalkAnimation.Start();
                walkOffset += m_GuybrushWalkDelta;
                if (walkOffset > m_EndPositionX)
                {
                    walkOffset = m_InitialPositionX;
                }
            }

            static uint previousFrame = 0;
            if (m_WalkAnimation.IsNewFrame())
            {
                auto& previousMesh = m_Registry.get<MeshComponent>(m_Guybrush[previousFrame]);
                previousMesh.m_Enabled = false;
                uint currentFrame = m_WalkAnimation.GetCurrentFrame();
                auto& currentMesh = m_Registry.get<MeshComponent>(m_Guybrush[currentFrame]);
                currentMesh.m_Enabled = true;
            }
            else
            {
                previousFrame = m_WalkAnimation.GetCurrentFrame();
            }
            float frameTranslationX =
                0.1f / static_cast<float>(m_WalkAnimation.GetFrames()) * m_WalkAnimation.GetCurrentFrame();

            for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
            {
                auto& transform = m_Registry.get<TransformComponent>(m_Guybrush[i]);
                transform.SetTranslationX(frameTranslationX + walkOffset);
            }
        }
        MoveClouds(timestep);
        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera());
        m_Renderer->Renderpass3D(m_Registry);

        // skip geometry and lighting passes
        m_Renderer->NextSubpass();
        m_Renderer->NextSubpass();

        // post processing
        m_Renderer->PostProcessingRenderpass();

        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
        Draw();
    }

    void CutScene::Draw()
    {
        // cloud 0
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Clouds[0]);
            m_Renderer->DrawWithTransform(m_CloudSprite, transform.GetMat4Local());
        }

        // cloud 1
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Clouds[1]);
            m_Renderer->DrawWithTransform(m_CloudSprite, transform.GetMat4Local());
        }

        // beach
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Beach);
            m_Renderer->DrawWithTransform(m_BeachSprite, transform.GetMat4Local());
        }

        // hero
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Guybrush[0]);
            m_Renderer->DrawWithTransform(Sprite2D(m_WalkAnimation.GetSprite()), transform.GetMat4Local());
        }
    }

    void CutScene::OnEvent(Event& event) {}

    void CutScene::OnResize()
    {
        m_CameraController->SetProjection();
        Init();
    }
} // namespace LucreApp
