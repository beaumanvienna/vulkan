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

#include <stdlib.h>
#include <time.h>

#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "resources/resources.h"

#include "mainScene.h"

namespace GfxRenderEngine
{
    extern std::shared_ptr<Texture> gTextureBloodIsland;
    extern std::shared_ptr<Texture> gTextureWalkway;
}

namespace LucreApp
{

    MainScene::MainScene()
        : m_GamepadInput{}, m_Fire{false},
          m_LaunchVulcanoTimer(1500)
    {
    }

    void MainScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();

        m_CameraController = std::make_shared<CameraController>();
        m_CameraController->SetTranslationSpeed(400.0f);
        m_CameraController->SetRotationSpeed(0.5f);

        m_Camera = CreateEntity();
        TransformComponent transform{};
        transform.m_Translation = {0.0f, -1.0f, -4.6f};
        transform.m_Rotation = {-0.0257f, 0.0f, 0.0f};
        m_Registry.emplace<TransformComponent>(m_Camera, transform);

        KeyboardInputControllerSpec keyboardInputControllerSpec{};
        m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);

        InitPhysics();

        // --- sprites ---
        m_VulcanoSprite = std::make_shared<Sprite>
        (
            0.0f, 0.0f, 1.0f, 1.0f,
            gTextureBloodIsland->GetWidth(), gTextureBloodIsland->GetHeight(),
            gTextureBloodIsland, "vulcano"
        );

        m_WalkwaySprite = std::make_shared<Sprite>
        (
            0.0f, 0.0f, 1.0f, 1.0f,
            gTextureWalkway->GetWidth(), gTextureWalkway->GetHeight(),
            gTextureWalkway, "walkway"
        );

        float scaleHero = 1.5f;
        // horn
        m_SpritesheetHorn.AddSpritesheetRow
        (
            Lucre::m_Spritesheet->GetSprite(I_HORN),
            HORN_ANIMATION_SPRITES /* frames */, 
            scaleHero /* scale) */
        );
        m_HornAnimation.Create(500ms /* per frame */, &m_SpritesheetHorn);
        m_HornAnimation.Start();

        LoadModels();

        m_LaunchVulcanoTimer.SetEventCallback
        (
            [](uint in, void* data)
            {
                std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
                Engine::m_Engine->QueueEvent(event);
                return 0u;
            }
        );
        m_LaunchVulcanoTimer.Start();

        // volcano smoke animation
        int poolSize = 50;
        float zaxis = 39.0f;
        m_SpritesheetSmoke.AddSpritesheetTile
        (
            Lucre::m_Spritesheet->GetSprite(I_VOLCANO_SMOKE), "volcano smoke sprite sheet",
            8, 8, /* rows, columns */
            0, /* margin */
            0.02f /* scale) */
        );
        m_VulcanoSmoke = std::make_shared<ParticleSystem>(poolSize, zaxis, &m_SpritesheetSmoke);

    }

    void MainScene::Stop()
    {
    }

    void MainScene::OnUpdate(const Timestep& timestep)
    {
        {
            static uint previousFrame = 0;
            if (!m_HornAnimation.IsRunning()) m_HornAnimation.Start();
            if (m_HornAnimation.IsNewFrame())
            {
                auto& previousMesh = m_Registry.get<MeshComponent>(m_Guybrush[previousFrame]);
                previousMesh.m_Enabled = false;
                uint currentFrame = m_HornAnimation.GetCurrentFrame();
                auto& currentMesh = m_Registry.get<MeshComponent>(m_Guybrush[currentFrame]);
                currentMesh.m_Enabled = true;
            }
            else
            {
                previousFrame = m_HornAnimation.GetCurrentFrame();
            }
        }

        auto view = m_Registry.view<TransformComponent>();
        auto& cameraTransform  = view.get<TransformComponent>(m_Camera);
        auto& vase0Transform   = view.get<TransformComponent>(m_Vase0);
        auto& vase1Transform   = view.get<TransformComponent>(m_Vase1);

        m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
        m_CameraController->SetViewYXZ(cameraTransform.m_Translation, cameraTransform.m_Rotation);

        // draw new scene
        m_Renderer->BeginScene(m_CameraController->GetCamera().get(), m_Registry);

        m_GamepadInputController->GetTransform(vase0Transform, true);
        m_GamepadInputController->GetTransform(vase1Transform, true);

        auto frameRotation = static_cast<const float>(timestep) * 0.6f;
        vase0Transform.m_Rotation.y = glm::mod(vase0Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase0Transform.m_Rotation.z = glm::mod(vase0Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.y = glm::mod(vase1Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.z = glm::mod(vase1Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());

        RotateLights(timestep);
        AnimateVulcan(timestep);

        SimulatePhysics(timestep);
        UpdateBananas(timestep);

        EmitVulcanoSmoke();
        m_VulcanoSmoke->OnUpdate(timestep);

        m_Renderer->Submit(m_Registry);
        m_Renderer->Submit(m_VulcanoSmoke);
        m_Renderer->EndScene();
    }

    void MainScene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent event)
            {
                auto zoomFactor = m_CameraController->GetZoomFactor();
                zoomFactor -= event.GetY()*0.1f;
                m_CameraController->SetZoomFactor(zoomFactor);
                return true;
            }
        );

        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent event)
            {
                switch(event.GetKeyCode())
                {
                    case ENGINE_KEY_R:
                        ResetScene();
                        ResetBananas();
                        break;
                    case ENGINE_KEY_G:
                        FireVulcano();
                        break;
                }
                return false;
            }
        );
    }

    void MainScene::OnResize()
    {
        m_CameraController->SetProjection();
    }

    void MainScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& transform = m_Registry.get<TransformComponent>(m_Camera);
        transform.m_Translation = {0.0f, -1.0f, -4.6f};
        transform.m_Rotation = {-0.0257f, 0.0f, 0.0f};
    }

    void MainScene::InitPhysics()
    {
        srand(time(nullptr));
        m_World = std::make_unique<b2World>(GRAVITY);

        {
            b2BodyDef groundBodyDef;
            groundBodyDef.position.Set(0.0f, -1.0f);

            m_GroundBody = m_World->CreateBody(&groundBodyDef);
            b2PolygonShape groundBox;
            groundBox.SetAsBox(50.0f, 0.4f);
            m_GroundBody->CreateFixture(&groundBox, 0.0f);
        }

        {
            b2BodyDef localGroundBodyDef;
            localGroundBodyDef.position.Set(0.0f, -10.0f);

            b2Body* localGroundBody = m_World->CreateBody(&localGroundBodyDef);
            b2PolygonShape localGroundBox;
            localGroundBox.SetAsBox(50.0f, 0.1f);
            localGroundBody->CreateFixture(&localGroundBox, 0.0f);
        }
    }

    void MainScene::FireVulcano()
    {
        m_Fire = true;
        m_GroundBody->SetTransform(b2Vec2(0.0f, -10.0f), 0.0f);

        auto view = m_Registry.view<BananaComponent, RigidbodyComponent>();
        for (auto banana : view)
        {
            auto& rigidbody = view.get<RigidbodyComponent>(banana);
            auto body = static_cast<b2Body*>(rigidbody.m_Body);
            body->SetTransform(b2Vec2(0.0f, -8.f), 0.0f);
        }
    }
}
