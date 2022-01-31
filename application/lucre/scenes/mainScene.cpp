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

#include "core.h"
#include "events/event.h"
#include "events/mouseEvent.h"

#include "mainScene.h"

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
        m_Registry.emplace<TransformComponent>(m_Camera);

        KeyboardInputControllerSpec keyboardInputControllerSpec{};
        m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

        LoadModels();

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);

    }

    void MainScene::Stop()
    {
    }

    void MainScene::OnUpdate(const Timestep& timestep)
    {
        auto view = m_Registry.view<TransformComponent>();
        auto& cameraTransform = view.get<TransformComponent>(m_Camera);
        auto& groundTransform = view.get<TransformComponent>(m_Ground);
        auto& vase0Transform  = view.get<TransformComponent>(m_Vase0);
        auto& vase1Transform  = view.get<TransformComponent>(m_Vase1);

        m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
        m_CameraController->SetViewYXZ(cameraTransform.m_Translation, cameraTransform.m_Rotation);

        // draw new scene
        m_Renderer->BeginScene(m_CameraController->GetCamera());

        m_GamepadInputController->GetTransform(groundTransform);
        m_GamepadInputController->GetTransform(vase0Transform, true);
        m_GamepadInputController->GetTransform(vase1Transform, true);

        auto frameRotation = static_cast<const float>(timestep) * 0.0006f;
        vase0Transform.m_Rotation.y = glm::mod(vase0Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase0Transform.m_Rotation.z = glm::mod(vase0Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.y = glm::mod(vase1Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.z = glm::mod(vase1Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());

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

    void MainScene::LoadModels()
    {
        {
            Builder builder{};
            m_Ground = CreateEntity();

            builder.LoadModel("application/lucre/models/colored_cube.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"base cube", model};
            m_Registry.emplace<MeshComponent>(m_Ground, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, 0.7f, 2.5f};
            transform.m_Scale = glm::vec3{0.01f, 2.0f, 2.0f};
            transform.m_Rotation = glm::vec3{0.0f, 0.0f, glm::half_pi<float>()};
            m_Registry.emplace<TransformComponent>(m_Ground, transform);
        }
        {
            Builder builder{};
            m_Vase0 = CreateEntity();

            builder.LoadModel("application/lucre/models/flat_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"polygon vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase0, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{-0.8f, -0.2f, 2.5f};
            transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
            m_Registry.emplace<TransformComponent>(m_Vase0, transform);
        }
        {
            Builder builder{};
            m_Vase1 = CreateEntity();

            builder.LoadModel("application/lucre/models/smooth_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"smooth vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase1, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.8f, -0.2f, 2.5f};
            transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
            m_Registry.emplace<TransformComponent>(m_Vase1, transform);
        }
        {
            Builder builder{};
            m_Sphere = CreateEntity();

            builder.LoadModel("application/lucre/models/sphere.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"point light 0", model};
            m_Registry.emplace<MeshComponent>(m_Sphere, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, -0.2f, 2.5f};
            transform.m_Scale = glm::vec3{0.05f};
            m_Registry.emplace<TransformComponent>(m_Sphere, transform);
        }
    }
}
