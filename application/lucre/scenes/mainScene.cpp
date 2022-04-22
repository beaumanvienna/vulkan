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
#include "gui/Common/UI/screen.h"
#include "scene/sceneLoader.h"
#include "auxiliary/math.h"

#include "mainScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"

namespace LucreApp
{

    MainScene::MainScene(const std::string& filepath)
        : Scene(filepath), m_GamepadInput{}, m_Fire{false},
          m_LaunchVolcanoTimer(1500)
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
        m_Registry.emplace<TransformComponent>(m_Camera, transform);
        ResetScene();

        KeyboardInputControllerSpec keyboardInputControllerSpec{};
        m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);

        GamepadInputControllerSpec gamepadInputControllerSpec{};
        m_GamepadInputController = std::make_unique<GamepadInputController>(gamepadInputControllerSpec);

        InitPhysics();

        // --- sprites ---

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

        Load();
        LoadModels();
        LoadScripts();
        StartScripts();
        TreeNode::Traverse(m_SceneHierarchy);
        m_Dictionary.List();

        m_LaunchVolcanoTimer.SetEventCallback
        (
            [](uint in, void* data)
            {
                std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
                Engine::m_Engine->QueueEvent(event);
                return 0u;
            }
        );
        m_LaunchVolcanoTimer.Start();

        // volcano smoke animation
        int poolSize = 50;
        float zaxis = -39.0f;
        m_SpritesheetSmoke.AddSpritesheetTile
        (
            Lucre::m_Spritesheet->GetSprite(I_VOLCANO_SMOKE), "volcano smoke sprite sheet",
            8, 8, /* rows, columns */
            0, /* margin */
            0.02f /* scale) */
        );
        m_VolcanoSmoke = std::make_shared<ParticleSystem>(poolSize, zaxis, &m_SpritesheetSmoke, 5.0f /*amplification*/, 1/*unlit*/);

    }

    void MainScene::Load()
    {
        SceneLoader loader(*this);
        loader.Deserialize();
    }

    void MainScene::LoadScripts()
    {
        auto duck = m_Dictionary.Retrieve("application/lucre/models/duck/duck.gltf::SceneWithDuck::duck");
        if (duck != entt::null)
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
        m_CameraController->SetViewYXZ(cameraTransform.GetTranslation(), cameraTransform.GetRotation());

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera(), m_Registry);

        auto frameRotation = static_cast<const float>(timestep) * 0.6f;

        ApplyDebugSettings();

        RotateLights(timestep);
        AnimateVulcan(timestep);

        SimulatePhysics(timestep);
        UpdateBananas(timestep);

        EmitVolcanoSmoke();
        m_VolcanoSmoke->OnUpdate(timestep);

        m_Renderer->Submit(m_Registry, m_SceneHierarchy);
        m_Renderer->Submit(m_VolcanoSmoke);
        m_Renderer->SubmitGUI(Lucre::m_Application->GetUI()->m_Registry);
        m_Renderer->SubmitGUI(SCREEN_ScreenManager::m_Registry);
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
                        FireVolcano();
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
        transform.SetTranslation({0.0f, 1.08f, 3.6f});
        transform.SetRotation({-0.04f, 0.0f, 0.0f});
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
                if (ImGUI::m_UseRoughness || ImGUI::m_UseMetallic || ImGUI::m_UseNormalMapIntensity || ImGUI::m_UsePointLightIntensity)
        {
            { // PbrNoMapComponent
                auto view = m_Registry.view<PbrNoMapComponent>();
                for (auto entity : view)
                {
                    auto& pbrNoMapComponent = view.get<PbrNoMapComponent>(entity);
                    if (ImGUI::m_UseRoughness)
                    {
                        pbrNoMapComponent.m_Roughness = ImGUI::m_Roughness;
                    }

                    if (ImGUI::m_UseMetallic)
                    {
                        pbrNoMapComponent.m_Metallic = ImGUI::m_Metallic;
                    }
                }
            }
            { // PbrDiffuseComponent
                auto view = m_Registry.view<PbrDiffuseComponent>();
                for (auto entity : view)
                {
                    auto& pbrDiffuseComponent = view.get<PbrDiffuseComponent>(entity);
                    if (ImGUI::m_UseRoughness)
                    {
                        pbrDiffuseComponent.m_Roughness = ImGUI::m_Roughness;
                    }

                    if (ImGUI::m_UseMetallic)
                    {
                        pbrDiffuseComponent.m_Metallic = ImGUI::m_Metallic;
                    }
                }
            }
            { // PbrDiffuseNormalComponent
                auto view = m_Registry.view<PbrDiffuseNormalComponent>();
                for (auto entity : view)
                {
                    auto& pbrDiffuseNormalComponent = view.get<PbrDiffuseNormalComponent>(entity);
                    if (ImGUI::m_UseRoughness)
                    {
                        pbrDiffuseNormalComponent.m_Roughness = ImGUI::m_Roughness;
                    }

                    if (ImGUI::m_UseMetallic)
                    {
                        pbrDiffuseNormalComponent.m_Metallic = ImGUI::m_Metallic;
                    }
                    if (ImGUI::m_UseNormalMapIntensity)
                    {
                        pbrDiffuseNormalComponent.m_NormalMapIntensity = ImGUI::m_NormalMapIntensity;
                    }
                }
            }
            { // PbrDiffuseNormalRoughnessMetallicComponent
                auto view = m_Registry.view<PbrDiffuseNormalRoughnessMetallicComponent>();
                for (auto entity : view)
                {
                    auto& pbrDiffuseNormalRoughnessMetallicComponent = view.get<PbrDiffuseNormalRoughnessMetallicComponent>(entity);

                    if (ImGUI::m_UseNormalMapIntensity)
                    {
                        pbrDiffuseNormalRoughnessMetallicComponent.m_NormalMapIntensity = ImGUI::m_NormalMapIntensity;
                    }
                }
            }

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
    }
}
