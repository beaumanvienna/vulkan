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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "scenes/splashScene.h"
#include "resources/resources.h"
#include "transform/matrix.h"

#include "lucre.h"

namespace LucreApp
{

    void SplashScene::Start()
    {
        m_IsRunning = true;
        Lucre::m_Application->PlaySound(IDR_WAVES);

        m_Renderer = Engine::m_Engine->GetRenderer();

        // create orthogonal camera 
        m_CameraController = std::make_shared<CameraController>(Camera::ORTHOGRAPHIC_PROJECTION);
        auto& camera = m_CameraController->GetCamera();
        auto position = glm::vec3(0.0f, 0.0f, 1.0f);
        auto direction = glm::vec3(0.0f, 0.0f, -1.0f);
        camera.SetViewDirection(position, direction);

        // --- sprites ---
        float scaleHero = 0.005f;
        // horn
        m_SpritesheetWalk.AddSpritesheetRow
        (
            Lucre::m_Spritesheet->GetSprite(I_WALK),
            WALK_ANIMATION_SPRITES /* frames */, 
            scaleHero /* scale) */
        );
        m_WalkAnimation.Create(150ms /* per frame */, &m_SpritesheetWalk);
        m_WalkAnimation.Start();

        for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
        {
            Builder builder{};

            auto sprite = m_SpritesheetWalk.GetSprite(i);
            glm::mat4 position = sprite->GetScaleMatrix();
            builder.LoadSprite(sprite, position, 10.0f /*amplification*/);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"walk animation", model};
            mesh.m_Enabled = false;

            m_Guybrush[i] = CreateEntity();
            m_Registry.emplace<MeshComponent>(m_Guybrush[i], mesh);

            TransformComponent transform{};
            transform.SetTranslation(glm::vec3{0.0f, -0.4f, 0.0f});
            m_Registry.emplace<TransformComponent>(m_Guybrush[i], transform);

            SpriteRendererComponent spriteRendererComponent{};
            m_Registry.emplace<SpriteRendererComponent>(m_Guybrush[i], spriteRendererComponent);
        }
        {
            Builder builder{};

            m_LogoSprite = Lucre::m_Spritesheet->GetSprite(I_LUCRE);
            m_LogoSprite->SetScale(0.001f);
            glm::mat4 position = m_LogoSprite->GetScaleMatrix(true);
            builder.LoadSprite(m_LogoSprite, position, 10.0f /*amplification*/);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"logo", model};
            mesh.m_Enabled = true;

            auto entity = CreateEntity();
            m_Registry.emplace<MeshComponent>(entity, mesh);

            TransformComponent transform{};
            transform.SetTranslation(glm::vec3{0.0f, 0.4f, 0.0f});
            m_Registry.emplace<TransformComponent>(entity, transform);

            SpriteRendererComponent spriteRendererComponent{};
            m_Registry.emplace<SpriteRendererComponent>(entity, spriteRendererComponent);
        }
    }

    void SplashScene::Stop()
    {
    }

    void SplashScene::OnUpdate(const Timestep& timestep)
    {
        {
            constexpr float initialPositionX = -1.0f;
            static float walkOffset = initialPositionX;
            if (!m_WalkAnimation.IsRunning())
            {
                m_WalkAnimation.Start();
                walkOffset += 0.42f;
                if (walkOffset > 1.4f)
                {
                    walkOffset = initialPositionX;
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
            float frameTranslationX = 0.1f / static_cast<float>(m_WalkAnimation.GetFrames()) * m_WalkAnimation.GetCurrentFrame();

            for (uint i = 0; i < WALK_ANIMATION_SPRITES; i++)
            {
                auto& transform = m_Registry.get<TransformComponent>(m_Guybrush[i]);
                transform.SetTranslationX(frameTranslationX+walkOffset);
            }
        }

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera(), m_Registry);

        // skip geometry and lighting passes
        m_Renderer->NextSubpass();
        m_Renderer->NextSubpass();

        m_Renderer->SubmitGUI(m_Registry);
        m_Renderer->EndScene();
    }

    void SplashScene::OnEvent(Event& event)
    {
    }

    void SplashScene::OnResize()
    {
    }
}
