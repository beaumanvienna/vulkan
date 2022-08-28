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

#include "scenes/settingsScene.h"
#include "transform/matrix.h"

namespace LucreApp
{

    void SettingsScene::Start()
    {
        float windowWidth       = static_cast<float>(Engine::m_Engine->GetWindowWidth());
        float windowHeight      = static_cast<float>(Engine::m_Engine->GetWindowHeight());
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_CameraController = std::make_shared<CameraController>(Camera::ORTHOGRAPHIC_PROJECTION);
        auto& camera = m_CameraController->GetCamera();
        auto position = glm::vec3(0.0f, 0.0f, 1.0f);
        auto direction = glm::vec3(0.0f, 0.0f, -1.0f);
        camera.SetViewDirection(position, direction);

        // barrels
        {
            Builder builder{};
        
            Sprite2D barrelSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_BARREL_LARGE));
            builder.LoadSprite(barrelSprite);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"barrel", model};
            mesh.m_Enabled = true;
        
            for (uint i = 0; i < NUM_BARRELS; i++)
            {
                m_Barrel[i] = CreateEntity();
                m_Registry.emplace<MeshComponent>(m_Barrel[i], mesh);
        
                TransformComponent transform = TransformComponent(barrelSprite.GetMat4());
                m_Registry.emplace<TransformComponent>(m_Barrel[i], transform);
        
                SpriteRendererComponent2D spriteRendererComponent2D{};
                m_Registry.emplace<SpriteRendererComponent2D>(m_Barrel[i], spriteRendererComponent2D);
            }
        }

        // background
        {
            Builder builder{};

            Sprite2D backGroundSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_SETTINGS_BG));
            builder.LoadSprite(backGroundSprite);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"logo", model};
            mesh.m_Enabled = true;

            m_BackGround = CreateEntity();
            m_Registry.emplace<MeshComponent>(m_BackGround, mesh);

            TransformComponent transform = TransformComponent(backGroundSprite.GetMat4());
            m_Registry.emplace<TransformComponent>(m_BackGround, transform);

            SpriteRendererComponent2D spriteRendererComponent2D{};
            m_Registry.emplace<SpriteRendererComponent2D>(m_BackGround, spriteRendererComponent2D);
        }
        Init();
    }

    void SettingsScene::Stop()
    {
    }

    void SettingsScene::Init()
    {
        Sprite2D backGroundSprite = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_SETTINGS_BG));
        Sprite2D barrelSprite     = Sprite2D(Lucre::m_Spritesheet->GetSprite(I_BARREL_LARGE));

        float windowWidth  = static_cast<float>(Engine::m_Engine->GetWindowWidth());
        float windowHeight = static_cast<float>(Engine::m_Engine->GetWindowHeight());

        // background
        {
            float scaleX = windowWidth / backGroundSprite.GetWidth();
            float scaleY = windowHeight / backGroundSprite.GetHeight();
            backGroundSprite.SetScale(scaleX, scaleY);

            auto& transform = m_Registry.get<TransformComponent>(m_BackGround);
            transform = TransformComponent(backGroundSprite.GetMat4());
            transform.SetTranslation(glm::vec3{windowWidth / 2.0f, windowHeight / 2.0f, 0.0f});
        }

        // barrels
        {
            float scale = windowWidth / (barrelSprite.GetWidth() * static_cast<float>(NUM_BARRELS * 16));
            barrelSprite.SetScale(scale, scale);

            for (uint i = 0; i < NUM_BARRELS; i++)
            {
                auto& transform = m_Registry.get<TransformComponent>(m_Barrel[i]);
                transform = TransformComponent(barrelSprite.GetMat4());
                transform.SetTranslation(glm::vec3{windowWidth / 2.0f, (windowHeight / static_cast<float>(NUM_BARRELS + 13)) * (static_cast<float>(i) + 10.5f), 0.0f});
            }
        }
    }

    void SettingsScene::OnUpdate(const Timestep& timestep)
    {
        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera(), m_Registry);
        // skip geometry and lighting passes
        m_Renderer->NextSubpass();
        m_Renderer->NextSubpass();
        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
        m_Renderer->Submit2D(&m_CameraController->GetCamera(), m_Registry);
    }

    void SettingsScene::OnEvent(Event& event)
    {
    }

    void SettingsScene::OnResize()
    {
        m_CameraController->SetProjection();
        Init();
    }
}
