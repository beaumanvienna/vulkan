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
        : m_GamepadInput{}
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
        transform.m_Rotation = {-0.11, 0.0f, 0.0f};
        m_Registry.emplace<TransformComponent>(m_Camera, transform);

        KeyboardInputControllerSpec keyboardInputControllerSpec{};
        m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);

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

        InitPhysics();
        LoadModels();

    }

    void MainScene::Stop()
    {
    }

    void MainScene::OnUpdate(const Timestep& timestep)
    {
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

        auto frameRotation = static_cast<const float>(timestep) * 0.0006f;
        vase0Transform.m_Rotation.y = glm::mod(vase0Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase0Transform.m_Rotation.z = glm::mod(vase0Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.y = glm::mod(vase1Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.z = glm::mod(vase1Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());

        RotateLights(timestep);
        AnimateVulcan(timestep);

        SimulatePhysics(timestep);
        RotateBananas(timestep);

        m_Renderer->Submit(m_Registry);
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
    }

    void MainScene::OnResize()
    {
        m_CameraController->SetProjection();
    }

    void MainScene::InitPhysics()
    {

        srand(time(nullptr));
        m_World = std::make_unique<b2World>(GRAVITY);

        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0.0f, -1.0f);

        b2Body* groundBody = m_World->CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(50.0f, 0.4f);
        groundBody->CreateFixture(&groundBox, 0.0f);

    }
}
