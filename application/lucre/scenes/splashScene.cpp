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
#include "scenes/splashScene.h"
#include "resources/resources.h"
#include "transform/matrix.h"

#include "lucre.h"

namespace LucreApp
{

    void SplashScene::Start()
    {
        Lucre::m_Application->PlaySound(IDR_WAVES);

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

            SpriteRendererComponent2D spriteRendererComponent2D{};
            m_Registry.emplace<SpriteRendererComponent2D>(m_Guybrush[i], spriteRendererComponent2D);
        }

        // logo
        {
            Builder builder{};

            auto logoSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_LUCRE));
            builder.LoadSprite(logoSprite);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"logo", model};
            mesh.m_Enabled = true;

            m_Logo = m_Registry.Create();
            m_Registry.emplace<MeshComponent>(m_Logo, mesh);

            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(m_Logo, transform);

            SpriteRendererComponent2D spriteRendererComponent2D{};
            m_Registry.emplace<SpriteRendererComponent2D>(m_Logo, spriteRendererComponent2D);
        }
        Init();
    }

    void SplashScene::Init()
    {
        m_InitialPositionX = -static_cast<float>(Engine::m_Engine->GetWindowWidth()) * 0.1f;
        m_EndPositionX = static_cast<float>(Engine::m_Engine->GetWindowWidth()) * 1.1f;
        m_WindowWidth = static_cast<float>(Engine::m_Engine->GetWindowWidth());
        m_WindowHeight = static_cast<float>(Engine::m_Engine->GetWindowHeight());

        // walk
        float scaleHero = m_WindowHeight * 0.08f / m_SpritesheetWalk.GetSprite(0).GetHeight();

        m_GuybrushWalkDelta = m_WindowHeight * 0.16f;
        for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
        {
            auto sprite = Sprite2D(m_SpritesheetWalk.GetSprite(i));
            sprite.SetScale(scaleHero);
            float width = sprite.GetWidth();
            float height = sprite.GetHeight();

            auto& transform = m_Registry.get<TransformComponent>(m_Guybrush[i]);
            transform.SetScale({width, height, 0.0f});
            transform.SetTranslationY(m_WindowHeight * 0.65f);
        }

        // logo
        {
            float width = Lucre::m_Spritesheet->GetSprite(I_LUCRE).GetWidth();
            float height = Lucre::m_Spritesheet->GetSprite(I_LUCRE).GetHeight();
            float scale = m_WindowHeight / height * 0.2f;
            auto& transform = m_Registry.get<TransformComponent>(m_Logo);
            transform.SetScale({scale * width, scale * height, 0.0f});
            transform.SetTranslation(glm::vec3{m_WindowWidth / 2.0f, m_WindowHeight * 0.3f, 0.0f});
        }
    }

    void SplashScene::Stop() {}

    void SplashScene::OnUpdate(const Timestep& timestep)
    {
        {
            static float walkOffset = m_InitialPositionX;
            if (!m_WalkAnimation.IsRunning())
            {
                m_WalkAnimation.Start();
                walkOffset += m_GuybrushWalkDelta;
                if (walkOffset > m_EndPositionX)
                {
                    walkOffset = m_InitialPositionX;
                    m_IsRunning = false;
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
        m_Renderer->Submit2D(&m_CameraController->GetCamera(), m_Registry);
    }

    void SplashScene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(
            [this](KeyPressedEvent l_Event)
            {
                switch (l_Event.GetKeyCode())
                {
                    case ENGINE_KEY_ESCAPE:
                        m_IsRunning = false;
                        break;
                }
                return true;
            });

        dispatcher.Dispatch<ControllerButtonPressedEvent>(
            [this](ControllerButtonPressedEvent l_Event)
            {
                switch (l_Event.GetControllerButton())
                {
                    case Controller::BUTTON_GUIDE:
                        m_IsRunning = false;
                        break;
                }
                return false;
            });
    }

    void SplashScene::OnResize()
    {
        m_CameraController->SetProjection();
        Init();
    }
} // namespace LucreApp
