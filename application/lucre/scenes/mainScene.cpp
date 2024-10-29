/* Engine Copyright (c) 2023 Engine Development Team
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

#include <stdlib.h>
#include <time.h>

#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "resources/resources.h"
#include "gui/Common/UI/screen.h"
#include "auxiliary/math.h"

#include "mainScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"

namespace LucreApp
{

    MainScene::MainScene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_Fire{false}, m_StartTimer{true}, m_LaunchVolcanoTimer(1000),
          m_SceneLoaderJSON{*this}
    {
    }

    void MainScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        ImGUI::m_AmbientLightIntensity = 0.12;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        { // set up camera
            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 500.0f;

            PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, zfar, znear);
            m_CameraController = std::make_shared<CameraController>(perspectiveCameraComponent);

            m_Camera = m_Registry.Create();
            TransformComponent cameraTransform{};
            m_Registry.emplace<TransformComponent>(m_Camera, cameraTransform);
            m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, m_Camera, "defaultCamera", m_Dictionary);
            ResetScene();

            KeyboardInputControllerSpec keyboardInputControllerSpec{};
            m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

            GamepadInputControllerSpec gamepadInputControllerSpec{};
            m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);
        }

        // --- sprites ---
        m_HornAnimation.Create(500ms /* per frame */, &m_SpritesheetHorn);
        m_HornAnimation.Start();

        StartScripts();
        m_SceneGraph.TraverseLog(SceneGraph::ROOT_NODE);
        m_Dictionary.List();

        m_LaunchVolcanoTimer.SetEventCallback(
            [](uint in, void* data)
            {
                std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
                Engine::m_Engine->QueueEvent(event);
                return 0u;
            });

        {
            std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
            Engine::m_Engine->QueueEvent(event);
        }

        m_Barrel = m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/barrel/barrel.gltf::0::root");
        m_Helmet =
            m_Dictionary.Retrieve("SL::application/lucre/models/assets/DamagedHelmet/glTF/DamagedHelmet.gltf::0::root");
        m_ToyCar = m_Dictionary.Retrieve("SL::application/lucre/models/assets/ToyCar/glTF/ToyCar.gltf::0::root");
        m_Sponza = m_Dictionary.Retrieve("SL::application/lucre/models/assets/Sponza/glTF/Sponza.gltf::0::root");
        if (m_Sponza != entt::null)
        {
            // place sponze scene
            auto& transform = m_Registry.get<TransformComponent>(m_Sponza);
            transform.SetTranslationX(0.229f);

            // place static lights for sponza scene
            float lightRadius = 0.1f;
            float height1 = 0.2f;
            float height2 = 1.3f;
            float height3 = 2.4f;
            float height4 = 3.5f;
            std::vector<glm::vec3> lightPositions = {
                {-0.285, height1, -1.542}, {-3.2, height1, -1.5420}, {-6.1, height1, -1.5420}, {2.7, height1, -1.5420},
                {5.6, height1, -1.5420},   {-0.285, height1, 1.2},   {-3.2, height1, 1.2},     {-6.1, height1, 1.2},
                {2.7, height1, 1.2},       {5.6, height1, 1.2},

                {-0.285, height2, -1.542}, {-3.2, height2, -1.5420}, {-6.1, height2, -1.5420}, {2.7, height2, -1.5420},
                {5.6, height2, -1.5420},   {-0.285, height2, 1.2},   {-3.2, height2, 1.2},     {-6.1, height2, 1.2},
                {2.7, height2, 1.2},       {5.6, height2, 1.2},

                {-0.285, height3, -1.542}, {-3.2, height3, -1.5420}, {-6.1, height3, -1.5420}, {2.7, height3, -1.5420},
                {5.6, height3, -1.5420},   {-0.285, height3, 1.2},   {-3.2, height3, 1.2},     {-6.1, height3, 1.2},
                {2.7, height3, 1.2},       {5.6, height3, 1.2},

                {-0.285, height4, -1.542}, {-3.2, height4, -1.5420}, {-6.1, height4, -1.5420}, {2.7, height4, -1.5420},
                {5.6, height4, -1.5420},   {-0.285, height4, 1.2},   {-3.2, height4, 1.2},     {-6.1, height4, 1.2},
                {2.7, height4, 1.2},       {5.6, height4, 1.2}};

            for (size_t index = 0; index < lightPositions.size(); ++index)
            {
                auto entity = CreatePointLight(POINT_LIGHT_INTENSITY, lightRadius);
                auto& lightTransform = m_Registry.get<TransformComponent>(entity);
                lightTransform.SetTranslation(lightPositions[index]);
                m_Registry.emplace<Group2>(entity, true);
            }
        }
    }

    void MainScene::Load()
    {

        float scaleHero = 1.5f;
        // horn
        m_SpritesheetHorn.AddSpritesheetRow(Lucre::m_Spritesheet->GetSprite(I_HORN), HORN_ANIMATION_SPRITES /* frames */,
                                            scaleHero /* scale) */
        );

        InitPhysics();

        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);

        LoadModels();
        LoadScripts();
    }

    void MainScene::LoadScripts()
    {
        auto duck =
            m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/duck/duck.gltf::0::SceneWithDuck::duck");
        if ((duck != entt::null) && m_Registry.all_of<ScriptComponent>(duck))
        {
            auto& duckScriptComponent = m_Registry.get<ScriptComponent>(duck);

            duckScriptComponent.m_Script = std::make_shared<DuckScript>(duck, this);
            LOG_APP_INFO("scripts loaded");
        }
    }

    void MainScene::StartScripts()
    {
        auto view = m_Registry.view<ScriptComponent>();
        for (auto& entity : view)
        {
            auto& scriptComponent = m_Registry.get<ScriptComponent>(entity);
            if (scriptComponent.m_Script)
            {
                LOG_APP_INFO("starting script {0}", scriptComponent.m_Filepath);
                scriptComponent.m_Script->Start();
            }
        }
    }

    void MainScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void MainScene::OnUpdate(const Timestep& timestep)
    {
        {
            static uint previousFrame = 0;
            if (!m_HornAnimation.IsRunning())
                m_HornAnimation.Start();
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

        if (m_StartTimer)
        {
            m_StartTimer = false;
            m_LaunchVolcanoTimer.Start();
        }

        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetView(cameraTransform.GetMat4Global());
        }

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera());
        m_Renderer->SubmitShadows(m_Registry);
        m_Renderer->Renderpass3D(m_Registry);

        ApplyDebugSettings();

        RotateLights(timestep);

        SimulatePhysics(timestep);
        UpdateBananas(timestep);

        // opaque objects
        m_Renderer->Submit(*this);

        // light opaque objects
        m_Renderer->NextSubpass();
        m_Renderer->LightingPass();

        // transparent objects
        m_Renderer->NextSubpass();
        m_Renderer->TransparencyPass(m_Registry);

        // post processing
        m_Renderer->PostProcessingRenderpass();

        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
    }

    void MainScene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<MouseScrolledEvent>(
            [this](MouseScrolledEvent mouseEvent)
            {
                auto zoomFactor = m_CameraController->GetZoomFactor();
                zoomFactor -= mouseEvent.GetY() * 0.1f;
                m_CameraController->SetZoomFactor(zoomFactor);
                return true;
            });

        dispatcher.Dispatch<KeyPressedEvent>(
            [this](KeyPressedEvent keyboardEvent)
            {
                switch (keyboardEvent.GetKeyCode())
                {
                    case ENGINE_KEY_R:
                        ResetScene();
                        ResetBananas();
                        break;
                    case ENGINE_KEY_G:
                        FireVolcano();
                        break;
                }
                return false;
            });
    }

    void MainScene::OnResize() { m_CameraController->SetProjection(); }

    void MainScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);

        cameraTransform.SetTranslation({3.1f, 1.08f, -1.6f});
        cameraTransform.SetRotation({-0.04f, 1.9f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraController->SetView(cameraTransform.GetMat4Local());
    }

    void MainScene::InitPhysics()
    {
        srand(time(nullptr));
        m_World = std::make_unique<b2World>(GRAVITY);

        {
            b2BodyDef groundBodyDef;
            groundBodyDef.position.Set(0.0f, 0.0f);

            m_GroundBody = m_World->CreateBody(&groundBodyDef);
            b2PolygonShape groundBox;
            groundBox.SetAsBox(50.0f, 0.04f);
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

    void MainScene::FireVolcano()
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

    void MainScene::ApplyDebugSettings()
    {
        if (ImGUI::m_UseNormalMapIntensity)
        {
            Model::m_NormalMapIntensity = ImGUI::m_NormalMapIntensity;
        }
        else
        {
            Model::m_NormalMapIntensity = 1.0f;
        }

        if (ImGUI::m_UseRoughness || ImGUI::m_UseMetallic || ImGUI::m_UseNormalMapIntensity ||
            ImGUI::m_UsePointLightIntensity)
        {
            if (ImGUI::m_UsePointLightIntensity)
            {
                auto view = m_Registry.view<PointLightComponent>();
                for (auto entity : view)
                {
                    auto& pointLight = view.get<PointLightComponent>(entity);
                    pointLight.m_LightIntensity = ImGUI::m_PointLightIntensity;
                }
            }
        }

        if (ImGUI::m_UseAmbientLightIntensity)
        {
            m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);
        }
    }
} // namespace LucreApp
