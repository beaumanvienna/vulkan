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
#include "auxiliary/debug.h"
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
          m_LaunchVolcanoTimer(1000)
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
            m_CameraControllers[CameraTypes::DefaultCamera] = std::make_shared<CameraController>(perspectiveCameraComponent);
            m_CameraControllers[CameraTypes::DefaultCamera]->GetCamera().SetName("default camera");

            m_Camera[CameraTypes::DefaultCamera] = m_Registry.Create();
            TransformComponent cameraTransform{};
            m_Registry.emplace<TransformComponent>(m_Camera[CameraTypes::DefaultCamera], cameraTransform);
            m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, m_Camera[CameraTypes::DefaultCamera], "defaultCamera",
                                    m_Dictionary);
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
                Water1Component water1Component{.m_Scale = {500.0f, 1.0f, 500.0f}, .m_Translation = {0.0f, 3.0f, 0.0f}};
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

        { // physics
            m_Car = m_Dictionary.Retrieve("SL::application/lucre/models/mario/car10.glb::0::root");
            m_Wheels[0] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::0::root");
            m_Wheels[1] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::1::root");
            m_Wheels[2] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::2::root");
            m_Wheels[3] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::3::root");
            if ((m_Car != entt::null) && (m_Wheels[0] != entt::null) && (m_Wheels[1] != entt::null) &&
                (m_Wheels[2] != entt::null) && (m_Wheels[3] != entt::null))
            {
                // set up 2nd camera
                m_Camera[CameraTypes::AttachedToCar1] =
                    m_Dictionary.Retrieve("SL::application/lucre/models/mario/car10.glb::0::Scene::CarCamera1");

                if (m_Camera[CameraTypes::AttachedToCar1] != entt::null)
                {
                    auto& cameraComponent =
                        m_Registry.get<PerspectiveCameraComponent>(m_Camera[CameraTypes::AttachedToCar1]);
                    m_CameraControllers[CameraTypes::AttachedToCar1] = std::make_shared<CameraController>(cameraComponent);
                    m_CameraControllers[CameraTypes::AttachedToCar1]->GetCamera().SetName("camera attached to car");

                    auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[CameraTypes::AttachedToCar1]);
                    cameraTransform.SetRotation(glm::vec3(0.0f, 3.141592654f, 0.0f));
                }
                // set up 3rd camera
                m_Camera[CameraTypes::AttachedToCar2] =
                    m_Dictionary.Retrieve("SL::application/lucre/models/mario/car10.glb::0::Scene::CarCamera2");

                if (m_Camera[CameraTypes::AttachedToCar2] != entt::null)
                {
                    auto& cameraComponent =
                        m_Registry.get<PerspectiveCameraComponent>(m_Camera[CameraTypes::AttachedToCar2]);
                    m_CameraControllers[CameraTypes::AttachedToCar2] = std::make_shared<CameraController>(cameraComponent);
                    m_CameraControllers[CameraTypes::AttachedToCar2]->GetCamera().SetName("camera attached to car");

                    auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[CameraTypes::AttachedToCar2]);
                    cameraTransform.SetRotation(glm::vec3(0.0f, 3.141592654f, 0.0f));
                }

                m_Physics->SetGameObject(Physics::GAME_OBJECT_CAR, m_Car);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_WHEEL_FRONT_LEFT, m_Wheels[0]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_WHEEL_FRONT_RIGHT, m_Wheels[1]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_WHEEL_REAR_LEFT, m_Wheels[2]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_WHEEL_REAR_RIGHT, m_Wheels[3]);

                auto createWheelTranslation = [](glm::vec3 const& translation)
                {
                    glm::mat4 translationTransform =
                        glm::translate(glm::mat4(1.0f), glm::vec3{translation.x, translation.y, translation.z});
                    return translationTransform;
                };

                auto createWheelScale = [](glm::vec3 const& scale)
                {
                    glm::mat4 scaleTransform = glm::scale(glm::vec3{scale.x, scale.y, scale.z});
                    return scaleTransform;
                };

                {
                    float wheelScale = 1.0f;
                    float liftWheels = 0.11f - 0.2f;
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{-0.418f, liftWheels, -0.414f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_FRONT_LEFT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_FRONT_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.418f, liftWheels, -0.414f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_FRONT_RIGHT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_FRONT_RIGHT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{-0.35f, liftWheels, 0.596f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_REAR_LEFT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_REAR_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.35f, liftWheels, 0.596f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_REAR_RIGHT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_REAR_RIGHT, wheelScaleTransform);
                    }
                }
                m_Physics->SetCarHeightOffset(0.2f);
            }

            auto racingLoop = m_Dictionary.Retrieve("SL::application/lucre/models/mario/racing loop.glb::0::root");
            if (racingLoop != entt::null)
            {
                float friction = 2.0f;
                m_Physics->CreateMeshTerrain(racingLoop, "application/lucre/models/mario/racing loop surface.glb", friction);
            }
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
                m_Lightbulb0 =
                    m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/lightBulb/lightBulb.glb::0::root");
                if (m_Lightbulb0 == entt::null)
                {
                    LOG_APP_INFO("m_Lightbulb0 not found");
                    m_Lightbulb0 = m_Registry.Create();
                    TransformComponent lightbulbTransform{};

                    lightbulbTransform.SetScale({1.0f, 1.0f, 1.0f});
                    lightbulbTransform.SetRotation({-0.888632, -0.571253, -0.166816});
                    lightbulbTransform.SetTranslation({1.5555, 4, -4.13539});

                    m_Registry.emplace<TransformComponent>(m_Lightbulb0, lightbulbTransform);
                }

                m_LightView0 = std::make_shared<Camera>(Camera::ProjectionType::ORTHOGRAPHIC_PROJECTION);
                SetLightView(m_Lightbulb0, m_LightView0);
            }

            {
                m_Lightbulb1 = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/lightBulb/lightBulb2.glb::0::root");
                if (m_Lightbulb1 == entt::null)
                {
                    LOG_APP_INFO("m_Lightbulb1 not found");
                    m_Lightbulb1 = m_Registry.Create();
                    TransformComponent transform{};

                    transform.SetScale({1.0f, 1.0f, 1.0f});
                    transform.SetRotation({0.0f, 0.0f, 0.785398f});
                    transform.SetTranslation({0.0f, -30.0f, 0.0f});

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
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraControllers.GetActiveCameraController()->SetView(cameraTransform.GetMat4Global());
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
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);
            m_CandleParticleSystem.OnUpdate(timestep, cameraTransform);
        }

        { // directional light / shadow maps
            enum ShadowRenderPass
            {
                HIGH_RESOLUTION = 0,
                LOW_RESOLUTION
            };
            struct Parameters
            {
                float m_Width;
                float m_LightBulbDistanceInCameraPlane;
                float m_LightBulbHeightOffset;
            };
            { // set rotation of low res shadow frustum to the one from high res
                auto& lightbulbTransform0 = m_Registry.get<TransformComponent>(m_Lightbulb0);
                auto& lightbulbTransform1 = m_Registry.get<TransformComponent>(m_Lightbulb1);
                auto& rotation0 = lightbulbTransform0.GetRotation();
                lightbulbTransform1.SetRotation(rotation0);
            }
            auto lightBulbUpdate = [&](const entt::entity directionalLightID, const entt::entity lightBulbID,
                                       const std::shared_ptr<Camera>& lightView, uint renderpass,
                                       Parameters const& parameters)
            {
                auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightBulbID);
                float scaleX = lightbulbTransform.GetScale().x;
                const float& width = parameters.m_Width;
                float left = -width / 2.0f * scaleX;
                float right = width / 2.0f * scaleX;
                float bottom = -width / 2.0f * scaleX;
                float top = width / 2.0f * scaleX;
                float near = 0.1f * scaleX;
                float far = 200.0f * scaleX;
                lightView->SetOrthographicProjection3D(left, right, -bottom, -top, near, far);
                { // put the directional light in front of the currently active camera
                    // retrieve camera position and camera look at direction
                    int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
                    auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);
                    auto& cameraPosition = cameraTransform.GetTranslation();
                    Camera& activeCamera = GetCamera();
                    glm::vec3 activeCameraDirection = activeCamera.GetDirection();
                    const float& lightBulbDistanceInCameraPlane = parameters.m_LightBulbDistanceInCameraPlane;
                    const float& lightBulbHeightOffset = parameters.m_LightBulbHeightOffset;

                    // point in front of camera for the light to look at
                    glm::vec3 vectorToPoint = activeCameraDirection * lightBulbDistanceInCameraPlane;
                    glm::vec3 inFrontOfCamera = cameraPosition + vectorToPoint;

                    // calculate vector to light
                    glm::vec3 directionToLight = -lightView->GetDirection();
                    glm::vec3 vectorToLight = directionToLight * lightBulbHeightOffset;

                    // acount for rotation of light
                    glm::vec3 cross = glm::cross(directionToLight, activeCameraDirection);
                    glm::vec3 lightRotationAdjustmentNorm{-cross.z, -cross.y, -cross.x};
                    glm::vec3 lightRotationAdjustment =
                        lightRotationAdjustmentNorm * lightBulbDistanceInCameraPlane / 8.0f; // fudge factor

                    glm::vec3 lightbulbPosition = inFrontOfCamera + vectorToLight + lightRotationAdjustment;
                    lightbulbTransform.SetTranslation(lightbulbPosition);
                }
                SetLightView(lightBulbID, lightView);
                SetDirectionalLight(directionalLightID, lightBulbID, lightView, renderpass /*shadow renderpass*/);
            };

            { // hi-res shadow map (1st cascade)
                Parameters parameters{
                    .m_Width = 80.0f, .m_LightBulbDistanceInCameraPlane = 40.0f, .m_LightBulbHeightOffset = 40.0f};
                lightBulbUpdate(m_DirectionalLight0, m_Lightbulb0, m_LightView0, ShadowRenderPass::HIGH_RESOLUTION,
                                parameters);
            }
            { // low-res shadow map (2nd cascade)
                Parameters parameters{
                    .m_Width = 250.0f, .m_LightBulbDistanceInCameraPlane = 125.0f, .m_LightBulbHeightOffset = 80.0f};
                lightBulbUpdate(m_DirectionalLight1, m_Lightbulb1, m_LightView1, ShadowRenderPass::LOW_RESOLUTION,
                                parameters);
            }
        }

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraControllers.GetActiveCameraController()->GetCamera());
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        m_Renderer->UpdateAnimations(m_Registry, timestep);
        m_Renderer->ShowDebugShadowMap(ImGUI::m_ShowDebugShadowMap);
        m_Renderer->SubmitShadows(m_Registry, m_DirectionalLights);

        if (m_Terrain1 != entt::null)
        { // water
            auto& water1Component = m_Registry.get<Water1Component>(m_Terrain1);
            float heightWater = water1Component.m_Translation.y;

            Camera reflectionCamera = m_CameraControllers.GetActiveCameraController()->GetCamera();
            auto view = m_Registry.view<TransformComponent>();
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera[activeCameraIndex]);

            glm::vec3 position = cameraTransform.GetTranslation();
            glm::vec3 rotation = cameraTransform.GetRotation();

            position.y = position.y - 2 * (position.y - heightWater);

            reflectionCamera.SetViewYXZ(position, rotation);

            static constexpr bool refraction = false;
            static constexpr bool reflection = true;
            std::array<bool, Renderer::WaterPasses::NUMBER_OF_WATER_PASSES> passes = {refraction, reflection};

            for (bool pass : passes)
            {
                float sign = (pass == reflection) ? 1.0f : -1.0f;
                glm::vec4 waterPlane{0.0f, sign, 0.0f, (-sign) * heightWater};
                auto& camera =
                    (pass == reflection) ? reflectionCamera : m_CameraControllers.GetActiveCameraController()->GetCamera();
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

        // physics debug visualization
        if (m_DrawDebugMesh)
        {
            m_Physics->Draw(m_CameraControllers.GetActiveCameraController()->GetCamera());
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
                auto zoomFactor = m_CameraControllers.GetActiveCameraController()->GetZoomFactor();
                zoomFactor -= l_Event.GetY() * 0.1f;
                m_CameraControllers.GetActiveCameraController()->SetZoomFactor(zoomFactor);
                return true;
            });

        dispatcher.Dispatch<KeyPressedEvent>(
            [this](KeyPressedEvent keyboardEvent)
            {
                switch (keyboardEvent.GetKeyCode())
                {
                    case ENGINE_KEY_N:
                    {
                        ++m_CameraControllers;
                        break;
                    }
                    case ENGINE_KEY_B:
                    {
                        m_DrawDebugMesh = !m_DrawDebugMesh;
                        break;
                    }
                    case ENGINE_KEY_R:
                    {
                        ResetScene();
                        ResetBananas();
                        break;
                    }
                    case ENGINE_KEY_G:
                    {
                        FireVolcano();
                        break;
                    }
                }
                return false;
            });
    }

    void Reserved0Scene::OnResize() { m_CameraControllers.GetActiveCameraController()->SetProjection(); }

    void Reserved0Scene::ResetScene()
    {
        m_CameraControllers.SetActiveCameraController(CameraTypes::DefaultCamera);
        m_CameraControllers[CameraTypes::DefaultCamera]->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[CameraTypes::DefaultCamera]);

        cameraTransform.SetTranslation({-3.0f, 6.0f, -25});
        cameraTransform.SetRotation({0.0f, TransformComponent::DEGREES_180, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraControllers[CameraTypes::DefaultCamera]->SetView(cameraTransform.GetMat4Local());
    }

    void Reserved0Scene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);

        glm::vec3 position = lightbulbTransform.GetTranslation();
        glm::vec3 rotation = lightbulbTransform.GetRotation();
        lightView->SetViewYXZ(position, rotation);
    }

    void Reserved0Scene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                             const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightView->GetDirection();
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
        {
            glm::vec3 scaleGroundPlane{5.0f, 0.4f, 50.0f};
            float heigtWaterSurface{5.0f};
            float zFightingOffset{0.00f};
            glm::vec3 translationGroundPlane{0.0f, zFightingOffset + heigtWaterSurface - scaleGroundPlane.y, 0.0f};
            m_Physics->CreateGroundPlane(scaleGroundPlane, translationGroundPlane); // 5x50 plane, with a small thickness
        }
        {
            glm::vec3 scaleGroundPlane{500.0f, 0.4f, 500.0f};
            float heigtWaterSurface{3.0f};
            float zFightingOffset{-0.050f};
            glm::vec3 translationGroundPlane{0.0f, zFightingOffset + heigtWaterSurface - scaleGroundPlane.y, 0.0f};
            m_Physics->CreateGroundPlane(scaleGroundPlane, translationGroundPlane); // 100x50 plane, with a small thickness
        }
        m_Physics->LoadModels();
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
        m_GamepadInputController->MoveVehicle(timestep, m_VehicleControl);
        m_Physics->OnUpdate(timestep, m_VehicleControl);
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

    Camera& Reserved0Scene::GetCamera() { return m_CameraControllers.GetActiveCameraController()->GetCamera(); }

    std::shared_ptr<CameraController>& Reserved0Scene::CameraControllers::GetActiveCameraController()
    {
        return m_CameraController[m_ActiveCamera];
    }

    std::shared_ptr<CameraController>& Reserved0Scene::CameraControllers::operator[](int index)
    {
        if ((index >= CameraTypes::MaxCameraTypes))
        {
            LOG_APP_ERROR("wrong camera indexed");
        }
        return m_CameraController[index];
    }

    Reserved0Scene::CameraControllers& Reserved0Scene::CameraControllers::operator++()
    {
        int maxChecks = static_cast<int>(CameraTypes::MaxCameraTypes);
        int nextActiveCamera = m_ActiveCamera;
        for (int iterator = 0; iterator < maxChecks; ++iterator)
        {
            ++nextActiveCamera;
            if (nextActiveCamera < maxChecks)
            {
                if (m_CameraController[nextActiveCamera])
                {
                    m_ActiveCamera = nextActiveCamera;
                    break;
                }
            }
            else
            {
                // default camera is always there
                m_ActiveCamera = static_cast<int>(CameraTypes::DefaultCamera);
                break;
            }
        }
        LOG_APP_INFO("switching to camera {0}", m_ActiveCamera);
        return *this;
    }

    std::shared_ptr<CameraController>& Reserved0Scene::CameraControllers::SetActiveCameraController(CameraTypes cameraType)
    {
        return SetActiveCameraController(static_cast<int>(cameraType));
    }

    std::shared_ptr<CameraController>& Reserved0Scene::CameraControllers::SetActiveCameraController(int index)
    {
        if ((index < static_cast<int>(CameraTypes::MaxCameraTypes)) && m_CameraController[index])
        {
            m_ActiveCamera = index;
        }
        else
        {
            LOG_APP_ERROR("couldn't change camera");
        }
        return m_CameraController[m_ActiveCamera];
    }

    void Reserved0Scene::CameraControllers::SetProjectionAll()
    {
        for (uint index = 0; index < CameraTypes::MaxCameraTypes; ++index)
        {
            if (m_CameraController[index])
            {
                m_CameraController[index]->SetProjection();
            }
        }
    }
} // namespace LucreApp
