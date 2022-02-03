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
#include "resources/resources.h"

#include "mainScene.h"

namespace GfxRenderEngine
{
    extern std::shared_ptr<Texture> gTexture;
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

        // --- vulcano sprite ---
        #warning "this code should be a single line"
        //size_t fileSize;
        //const uchar* data = (const uchar*) ResourceSystem::GetDataPointer(fileSize, "/images/images/I_Vulkan.png", IDB_VULKAN, "PNG");
        //m_Texture = Texture::Create();
        //m_Texture->Init(data, fileSize);

        m_VulcanoSprite = std::make_shared<Sprite>
        (
            0.0f, 0.0f, 1.0f, 1.0f,
            gTexture->GetWidth(), gTexture->GetHeight(),
            gTexture, "vulcano"
        );

        LoadModels();

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
        auto& spriteTransform = view.get<TransformComponent>(m_Sprite);

        m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
        m_CameraController->SetViewYXZ(cameraTransform.m_Translation, cameraTransform.m_Rotation);

        // draw new scene
        m_Renderer->BeginScene(m_CameraController->GetCamera().get(), m_Registry);

        m_GamepadInputController->GetTransform(groundTransform);
        m_GamepadInputController->GetTransform(spriteTransform);
        m_GamepadInputController->GetTransform(vase0Transform, true);
        m_GamepadInputController->GetTransform(vase1Transform, true);

        auto frameRotation = static_cast<const float>(timestep) * 0.0006f;
        vase0Transform.m_Rotation.y = glm::mod(vase0Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase0Transform.m_Rotation.z = glm::mod(vase0Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.y = glm::mod(vase1Transform.m_Rotation.y + frameRotation, glm::two_pi<float>());
        vase1Transform.m_Rotation.z = glm::mod(vase1Transform.m_Rotation.z + frameRotation, glm::two_pi<float>());

        RotateLights(timestep);

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

    void MainScene::RotateLights(const Timestep& timestep)
    {
        //LOG_APP_CRITICAL("timestep: {0}", (float) timestep);
        float time = 0.1f * glm::pi<float>() * timestep / 1000;
        auto rotateLight = glm::rotate(glm::mat4(1.f), time, {0.f, -1.f, 0.f});

        int lightIndex = 0;
        auto view = m_Registry.view<PointLightComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& transform  = view.get<TransformComponent>(entity);

            // update light position
            transform.m_Translation = glm::vec3(rotateLight * glm::vec4(transform.m_Translation, 1.f));

            lightIndex++;
        }
    }

    void MainScene::LoadModels()
    {
        {
            Builder builder{};
            m_Sprite = CreateEntity();

            glm::mat4 position = m_VulcanoSprite->GetScaleMatrix();
            builder.LoadSprite(m_VulcanoSprite.get(), position);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"sprite", model};
            m_Registry.emplace<MeshComponent>(m_Sprite, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, -0.2f, 2.0f};
            transform.m_Scale = glm::vec3{0.017f, 0.014f, 0.017f};
            m_Registry.emplace<TransformComponent>(m_Sprite, transform);
        }
        {
            Builder builder{};
            m_Ground = CreateEntity();

            builder.LoadModel("application/lucre/models/colored_cube.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"base cube", model};
            m_Registry.emplace<MeshComponent>(m_Ground, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, 0.7f, 0.0f};
            transform.m_Scale = glm::vec3{0.01f, 6.0f, 2.0f};
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
            transform.m_Translation = glm::vec3{-0.8f, -0.2f, 0.0f};
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
            transform.m_Translation = glm::vec3{0.8f, -0.2f, 0.0f};
            transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
            m_Registry.emplace<TransformComponent>(m_Vase1, transform);
        }
        {

            std::vector<glm::vec3> lightColors
            {
                {1.f, .1f, .1f},
                {.1f, .1f, 1.f},
                {.1f, 1.f, .1f},
                {1.f, 1.f, .1f},
                {.1f, 1.f, 1.f},
                {1.f, 1.f, 1.f}
            };

            for (int i = 0; i < lightColors.size(); i++)
            {
                m_PointLight[i] = CreatePointLight(0.2f, 0.1f, lightColors[i]);
                auto rotateLight = glm::rotate
                (
                    glm::mat4(1.0f),
                    (i * glm::two_pi<float>()) / lightColors.size(),
                    {0.f, -1.f, 0.f}
                );
                TransformComponent transform{};
                transform.m_Translation = glm::vec3(rotateLight * glm::vec4(-1.0f, +0.3f, -1.0f, 0.0f));
                m_Registry.emplace<TransformComponent>(m_PointLight[i], transform);
            }
        }
    }
}
