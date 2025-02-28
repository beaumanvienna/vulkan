/* Engine Copyright (c) 2025 Engine Development Team
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

#include "auxiliary/math.h"
#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "gui/Common/UI/screen.h"
#include "resources/resources.h"

#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"
#include "reserved0Scene.h"
#include "renderer/builder/grassBuilder.h"

namespace LucreApp
{

    Reserved0Scene::Reserved0Scene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}, m_CandleParticleSystem{*this, "candles.json"},
          m_LaunchVolcanoTimer(1000), m_ReflectionCamera(Camera::PERSPECTIVE_PROJECTION)
    {
    }

    Reserved0Scene::~Reserved0Scene() {}

    void Reserved0Scene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        {
            // set up camera
            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 1500.0f;

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

        {
            // place static lights for beach scene
            float intensity = 5.0f;
            float lightRadius = 0.1f;
            float height1 = 5.4f;
            std::vector<glm::vec3> lightPositions = {{5.6, height1, 0.7}};

            for (size_t i = 0; i < lightPositions.size(); ++i)
            {
                auto entity = CreatePointLight(intensity, lightRadius);
                auto& transform = m_Registry.get<TransformComponent>(entity);
                transform.SetTranslation(lightPositions[i]);
                m_Registry.emplace<Group2>(entity, true);
            }
        }
        {
            float intensity = 5.0f;
            glm::vec3 color{1.0f, 1.0f, 1.0f};
            m_DirectionalLight0 = CreateDirectionalLight(intensity, color);
            m_DirectionalLight1 = CreateDirectionalLight(intensity, color);
            auto& directionalLightComponent0 = m_Registry.get<DirectionalLightComponent>(m_DirectionalLight0);
            auto& directionalLightComponent1 = m_Registry.get<DirectionalLightComponent>(m_DirectionalLight1);
            m_DirectionalLights.push_back(&directionalLightComponent0);
            m_DirectionalLights.push_back(&directionalLightComponent1);
        }

        {
            // TLMM = terrain loader multi material
            m_Terrain1 = m_Dictionary.Retrieve("TLMM::application/lucre/models/terrain/terrain1.glb::0::root");
            if (m_Terrain1 != entt::null)
            {
                Water1Component water1Component{.m_Scale = {25.0f, 1.0f, 50.0f}, .m_Translation = {0.0f, 1.5f, 0.0f}};
                m_Registry.emplace<Water1Component>(m_Terrain1, water1Component);
            }

            auto terrain = m_Dictionary.Retrieve("TLMM::application/lucre/models/terrain/terrain1.glb::0::Scene::terrain");
            if (terrain != entt::null)
            {
                m_Registry.remove<PbrMaterialTag>(terrain);
                PbrMultiMaterialTag pbrMultiMaterialTag{};
                m_Registry.emplace<PbrMultiMaterialTag>(terrain, pbrMultiMaterialTag);
            }

            auto gaea =
                m_Dictionary.Retrieve("TLMM::application/lucre/models/terrain/terrainGaea.glb::0::Scene::TerrainGaea");
            if (gaea != entt::null)
            {
                m_Registry.remove<PbrMaterialTag>(gaea);
                PbrMultiMaterialTag pbrMultiMaterialTag{};
                m_Registry.emplace<PbrMultiMaterialTag>(gaea, pbrMultiMaterialTag);
            }
        }

        {
            Scene& scene = *this;
            Grass::GrassSpec grassSpec{.m_FilepathGrassModel = "application/lucre/models/assets/grass/grass1.glb",
                                       .m_FilepathGrassMask =
                                           "application/lucre/models/mario/mario section 01 - grass mask.glb",
                                       .m_Rotation = glm::vec3{-3.14159f, 0.0f, -3.14159f},
                                       .m_Translation = glm::vec3{7.717f, 3.491f, 45.133f},
                                       .m_Scale = glm::vec3{2.352f, 2.352f, 2.352f},
                                       .m_ScaleXZ = 0.1f,
                                       .m_ScaleY = 0.05f};
            GrassBuilder builder(grassSpec, scene);
            builder.Build();
        }
    }

    void Reserved0Scene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);
        InitPhysics();
        LoadModels();
        LoadTerrain();
        LoadScripts();
    }

    void Reserved0Scene::LoadTerrain() {}

    void Reserved0Scene::LoadModels()
    {
        {
            std::vector<std::string> faces = {
                "application/lucre/models/assets/Skybox/right.png", "application/lucre/models/assets/Skybox/left.png",
                "application/lucre/models/assets/Skybox/top.png",   "application/lucre/models/assets/Skybox/bottom.png",
                "application/lucre/models/assets/Skybox/front.png", "application/lucre/models/assets/Skybox/back.png"};

            Builder builder;
            m_Skybox = builder.LoadCubemap(faces, m_Registry);
            auto view = m_Registry.view<TransformComponent>();
            auto& skyboxTransform = view.get<TransformComponent>(m_Skybox);
            skyboxTransform.SetScale(500.0f);
        }
        { // directional lights
            {
                m_Lightbulb0 = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/lightBulb/lightBulb.gltf::0::root");
                if (m_Lightbulb0 == entt::null)
                {
                    LOG_APP_INFO("m_Lightbulb0 not found");
                    m_Lightbulb0 = m_Registry.Create();
                    TransformComponent transform{};

                    transform.SetScale({0.01, 0.01, 0.01});
                    transform.SetRotation({-0.888632, -0.571253, -0.166816});
                    transform.SetTranslation({1.5555, 4, -4.13539});

                    m_Registry.emplace<TransformComponent>(m_Lightbulb0, transform);
                }

                m_LightView0 = std::make_shared<Camera>(Camera::ProjectionType::ORTHOGRAPHIC_PROJECTION);
                SetLightView(m_Lightbulb0, m_LightView0);
            }

            {
                m_Lightbulb1 = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/lightBulb/lightBulb2.gltf::0::root");
                if (m_Lightbulb1 == entt::null)
                {
                    LOG_APP_INFO("m_Lightbulb1 not found");
                    m_Lightbulb1 = m_Registry.Create();
                    TransformComponent transform{};

                    transform.SetScale({0.00999934, 0.00999997, 0.00999993});
                    transform.SetRotation({-1.11028, -0.546991, 0.165967});
                    transform.SetTranslation({6, 6.26463, -14.1572});

                    m_Registry.emplace<TransformComponent>(m_Lightbulb1, transform);
                }
                m_LightView1 = std::make_shared<Camera>(Camera::ProjectionType::ORTHOGRAPHIC_PROJECTION);
                float left = -20.0f;
                float right = 20.0f;
                float bottom = -14.0f;
                float top = 14.0f;
                float near = 0.1f;
                float far = 40.0f;
                m_LightView1->SetOrthographicProjection3D(left, right, bottom, top, near, far);
                SetLightView(m_Lightbulb1, m_LightView1);
            }
        }

        {
            m_Penguin =
                m_Dictionary.Retrieve("SL::application/lucre/models/ice/penguin.glb::0::Scene::Linux Penguin (Left Leg)");
            if (m_Penguin != entt::null)
            {
                if (m_Registry.all_of<SkeletalAnimationTag>(m_Penguin))
                {
                    auto& mesh = m_Registry.get<MeshComponent>(m_Penguin);
                    SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
                    animations.SetRepeatAll(true);
                    animations.Start();
                }
                else
                {
                    LOG_APP_CRITICAL("entity {0} must have skeletal animation tag", static_cast<int>(m_Penguin));
                }
            }
        }

        {
            m_Mario = m_Dictionary.Retrieve("SL::application/lucre/models/mario/mario animated.glb::0::Scene::mario mesh");
            if (m_Mario != entt::null)
            {
                if (m_Registry.all_of<SkeletalAnimationTag>(m_Mario))
                {
                    auto& mesh = m_Registry.get<MeshComponent>(m_Mario);
                    SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
                    animations.SetRepeatAll(true);
                    animations.Start();
                }
                else
                {
                    LOG_APP_CRITICAL("entity {0} must have skeletal animation tag", static_cast<int>(m_Mario));
                }
            }
        }

        {
            FastgltfBuilder builder("application/lucre/models/mario/banana_minion_rush.glb", *this);
            builder.SetDictionaryPrefix("mainScene");
            builder.Load(MAX_B /*instance(s)*/);

            for (uint i = 0; i < MAX_B; ++i)
            {
                m_Banana[i] = m_Dictionary.Retrieve(
                    "mainScene::application/lucre/models/mario/banana_minion_rush.glb::" + std::to_string(i) + "::root");

                TransformComponent transform{};
                if (i < 12)
                {
                    transform.SetTranslation(glm::vec3{5.0f + 0.5 * i, 3.5f, 45.1736});
                }
                else
                {
                    transform.SetTranslation(glm::vec3{5.0f + 0.5 * (i - 12), 3.5f, 44.1736});
                }
                m_Registry.emplace<BananaComponent>(m_Banana[i], true);

                b2BodyDef bodyDef;
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(0.0f, -1.0f);
                auto body = m_World->CreateBody(&bodyDef);

                b2CircleShape circle;
                circle.m_radius = 0.001f;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circle;
                fixtureDef.density = 1.0f;
                fixtureDef.friction = 0.2f;
                fixtureDef.restitution = 0.4f;
                body->CreateFixture(&fixtureDef);
                m_Registry.emplace<RigidbodyComponent>(m_Banana[i], RigidbodyComponent::DYNAMIC, body);
            }
        }
    }

    void Reserved0Scene::LoadScripts() {}

    void Reserved0Scene::StartScripts() {}

    void Reserved0Scene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void Reserved0Scene::OnUpdate(const Timestep& timestep)
    {
        ZoneScopedNC("Reserved0Scene", 0x0000ff);
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetView(cameraTransform.GetMat4Global());
        }

        SimulatePhysics(timestep);
        UpdateBananas(timestep);

        if (m_StartTimer)
        {
            m_StartTimer = false;
            m_LaunchVolcanoTimer.Start();
        }

        if (m_CharacterAnimation)
        {
            m_CharacterAnimation->OnUpdate(timestep);
        }

        { // update particle systems
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);
            m_CandleParticleSystem.OnUpdate(timestep, cameraTransform);
        }

        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(m_Lightbulb0);
            float scaleX = lightbulbTransform.GetScale().x;
            float left = -400.0f * scaleX;
            float right = 400.0f * scaleX;
            float bottom = -400.0f * scaleX;
            float top = 400.0f * scaleX;
            float near = 10.0f * scaleX;
            float far = 1000.0f * scaleX;
            m_LightView0->SetOrthographicProjection3D(left, right, bottom, top, near, far);
        }
        SetLightView(m_Lightbulb0, m_LightView0);
        SetLightView(m_Lightbulb1, m_LightView1);
        SetDirectionalLight(m_DirectionalLight0, m_Lightbulb0, m_LightView0, 0 /*shadow renderpass*/);
        SetDirectionalLight(m_DirectionalLight1, m_Lightbulb1, m_LightView1, 1 /*shadow renderpass*/);

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera());
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        m_Renderer->UpdateAnimations(m_Registry, timestep);
        m_Renderer->ShowDebugShadowMap(ImGUI::m_ShowDebugShadowMap);
        m_Renderer->SubmitShadows(m_Registry, m_DirectionalLights);

        if (m_Terrain1 != entt::null)
        { // water
            auto& water1Component = m_Registry.get<Water1Component>(m_Terrain1);
            float heightWater = water1Component.m_Translation.y;

            m_ReflectionCamera = m_CameraController->GetCamera();
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera);
            glm::vec3 position = cameraTransform.GetTranslation();
            glm::vec3 rotation = cameraTransform.GetRotation();

            position.y = position.y - 2 * (position.y - heightWater);

            m_ReflectionCamera.SetViewYXZ(position, rotation);

            static constexpr bool refraction = false;
            static constexpr bool reflection = true;
            std::array<bool, Renderer::WaterPasses::NUMBER_OF_WATER_PASSES> passes = {refraction, reflection};

            for (bool pass : passes)
            {
                float sign = (pass == reflection) ? 1.0f : -1.0f;
                glm::vec4 waterPlane{0.0f, sign, 0.0f, (-sign) * heightWater};
                auto& camera = (pass == reflection) ? m_ReflectionCamera : m_CameraController->GetCamera();
                m_Renderer->RenderpassWater(m_Registry, camera, pass, waterPlane);
                // opaque objects
                m_Renderer->SubmitWater(*this, pass);

                // light opaque objects
                m_Renderer->NextSubpass();
                m_Renderer->LightingPassWater(pass);

                // transparent objects
                m_Renderer->NextSubpass();
                m_Renderer->TransparencyPassWater(m_Registry, pass);

                m_Renderer->EndRenderpassWater();
            }
        }

        { // 3D
            m_Renderer->Renderpass3D(m_Registry);

            ApplyDebugSettings();

            // opaque objects
            m_Renderer->Submit(*this);

            // light opaque objects
            m_Renderer->NextSubpass();
            m_Renderer->LightingPass();

            // transparent objects
            m_Renderer->NextSubpass();
            m_Renderer->TransparencyPass(m_Registry);
        }

        // post processing
        m_Renderer->PostProcessingRenderpass();

        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
    }

    void Reserved0Scene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<MouseScrolledEvent>(
            [this](MouseScrolledEvent l_Event)
            {
                auto zoomFactor = m_CameraController->GetZoomFactor();
                zoomFactor -= l_Event.GetY() * 0.1f;
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

    void Reserved0Scene::OnResize() { m_CameraController->SetProjection(); }

    void Reserved0Scene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);

        cameraTransform.SetTranslation({-3.485f, 3.625f, -25});
        cameraTransform.SetRotation({-0.074769905f, 3.11448769f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraController->SetView(cameraTransform.GetMat4Local());
    }

    void Reserved0Scene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);

            glm::vec3 position = lightbulbTransform.GetTranslation();
            glm::vec3 rotation = lightbulbTransform.GetRotation();
            lightView->SetViewYXZ(position, rotation);
        }
    }

    void Reserved0Scene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                             const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightbulbTransform.GetRotation();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void Reserved0Scene::ApplyDebugSettings()
    {
        if (ImGUI::m_UseAmbientLightIntensity)
        {
            m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);
        }
    }

    void Reserved0Scene::InitPhysics()
    {
        // box2D
        std::srand(time(nullptr));
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
        // Jolt
        m_Physics = Physics::Create(*this);
        m_Physics->CreateGroundPlane();
        m_Physics->CreateSphere();
    }

    void Reserved0Scene::FireVolcano()
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

    void Reserved0Scene::ResetBananas()
    {
        m_GroundBody->SetTransform(b2Vec2(0.0f, 0.0f), 0.0f);
        auto view = m_Registry.view<BananaComponent, TransformComponent, RigidbodyComponent>();

        uint i = 0;
        for (auto banana : view)
        {
            auto& transform = view.get<TransformComponent>(banana);
            auto& rigidbody = view.get<RigidbodyComponent>(banana);
            auto body = static_cast<b2Body*>(rigidbody.m_Body);
            body->SetLinearVelocity(b2Vec2(0.0f, 0.01f));
            body->SetAngularVelocity(0.0f);
            if (i < 12)
            {
                body->SetTransform(b2Vec2(7.0f + 0.5 * i, 6.0f + i * 1.0f), 0.0f);
                transform.SetTranslationZ(47.1f);
            }
            else
            {
                body->SetTransform(b2Vec2(7.0f + 0.5 * (i - 12), 6.0f + i * 1.0f), 0.0f);
                transform.SetTranslationZ(43.0f);
            }
            i++;
        }
    }

    void Reserved0Scene::SimulatePhysics(const Timestep& timestep)
    {
        // box2D
        float step = timestep;

        int velocityIterations = 6;
        int positionIterations = 2;
        m_World->Step(step, velocityIterations, positionIterations);
        // Jolt
        m_Physics->OnUpdate(timestep);
    }

    void Reserved0Scene::UpdateBananas(const Timestep& timestep)
    {
        auto view = m_Registry.view<BananaComponent, TransformComponent, RigidbodyComponent>();

        static constexpr float ROTATIONAL_SPEED = 3.0f;
        auto rotationDelta = ROTATIONAL_SPEED * timestep;
        for (auto banana : view)
        {
            auto& transform = view.get<TransformComponent>(banana);
            auto& rigidbody = view.get<RigidbodyComponent>(banana);
            auto body = static_cast<b2Body*>(rigidbody.m_Body);
            b2Vec2 position = body->GetPosition();
            transform.SetTranslationX(position.x - 2.5f);
            transform.SetTranslationY(position.y + 3.5f);
            transform.SetRotationY(transform.GetRotation().y + rotationDelta);
        }

        static uint index = 0;
        if (m_Fire) // from volcano
        {
            static auto start = Engine::m_Engine->GetTime();
            if ((Engine::m_Engine->GetTime() - start) > 100ms)
            {
                if (index < MAX_B)
                {
                    // random values in [-1.0f, 1.0f]
                    float rVal = 2 * (static_cast<float>(rand()) / RAND_MAX) - 1.0f;
                    // get new start time
                    start = Engine::m_Engine->GetTime();

                    // move to backgound on z-axis
                    auto& transform = m_Registry.get<TransformComponent>(m_Banana[index]);
                    transform.SetTranslationZ(5.0f);

                    auto& rigidbody = m_Registry.get<RigidbodyComponent>(m_Banana[index]);
                    auto body = static_cast<b2Body*>(rigidbody.m_Body);
                    body->SetLinearVelocity(b2Vec2(0.1f + rVal * 4, 5.0f));
                    body->SetTransform(b2Vec2(0.0f, 3.2f), 0.0f);

                    index++;
                }
                else if ((Engine::m_Engine->GetTime() - start) > 1500ms)
                {
                    ResetBananas();
                    m_Fire = false;
                }
            }
        }
        else
        {
            index = 0;
        }
    }
} // namespace LucreApp
