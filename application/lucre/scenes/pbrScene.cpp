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
#include <bitset>

#include "auxiliary/debug.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "auxiliary/math.h"
#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "gui/Common/UI/screen.h"
#include "resources/resources.h"

#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"
#include "pbrScene.h"

namespace LucreApp
{

    PBRScene::PBRScene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}, m_CandleParticleSystem{*this, "candles.json"},
          m_UseIBL{true}
    {
    }

    PBRScene::~PBRScene() {}

    void PBRScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        {
            // set up camera
            float aspectRatio = 1.777f;
            float yfov = 1.0f; // 57.3°
            float znear = 0.1f;
            float zfar = 250.0f;

            PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, znear, zfar);
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

        {
            // set up car follow camera
            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 1500.0f;

            PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, znear, zfar);
            m_CameraControllers[CameraTypes::CarFollow] = std::make_shared<CameraController>(perspectiveCameraComponent);
            m_CameraControllers[CameraTypes::CarFollow]->GetCamera().SetName("car follow camera");

            m_Camera[CameraTypes::CarFollow] = m_Registry.Create();
            TransformComponent cameraTransform{};
            cameraTransform.SetScale(glm::vec3(1.0f));
            m_Registry.emplace<TransformComponent>(m_Camera[CameraTypes::CarFollow], cameraTransform);
            m_SceneGraph.CreateNode(SceneGraph::ROOT_NODE, m_Camera[CameraTypes::CarFollow], "Car follow camera",
                                    m_Dictionary);
        }

        StartScripts();
        m_SceneGraph.TraverseLog(SceneGraph::ROOT_NODE);
        m_Dictionary.List();

        {
            std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
            Engine::m_Engine->QueueEvent(event);
        }

        {
            // place static lights
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

        { // physics
            // car
            m_Car = m_Dictionary.Retrieve("SL::application/lucre/models/mario/car10.glb::0::root");
            m_Wheels[0] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::0::root");
            m_Wheels[1] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::1::root");
            m_Wheels[2] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::2::root");
            m_Wheels[3] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheel.glb::3::root");
            if ((m_Car != entt::null) && (m_Wheels[0] != entt::null) && (m_Wheels[1] != entt::null) &&
                (m_Wheels[2] != entt::null) && (m_Wheels[3] != entt::null))
            {
                // set up 2nd camera
                m_Camera[CameraTypes::AttachedToCar] =
                    m_Dictionary.Retrieve("SL::application/lucre/models/mario/car10.glb::0::Scene::CarCamera2");

                if (m_Camera[CameraTypes::AttachedToCar] != entt::null)
                {
                    auto& cameraComponent = m_Registry.get<PerspectiveCameraComponent>(m_Camera[CameraTypes::AttachedToCar]);
                    m_CameraControllers[CameraTypes::AttachedToCar] = std::make_shared<CameraController>(cameraComponent);
                    m_CameraControllers[CameraTypes::AttachedToCar]->GetCamera().SetName("camera attached to car");
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
                    float liftWheels = 0.0f;
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.0f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_FRONT_LEFT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_FRONT_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.0f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_FRONT_RIGHT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_FRONT_RIGHT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.0f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_REAR_LEFT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_REAR_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.0f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetWheelTranslation(Physics::WHEEL_REAR_RIGHT, wheelTranslationTransform);
                        m_Physics->SetWheelScale(Physics::WHEEL_REAR_RIGHT, wheelScaleTransform);
                    }
                }
                m_Physics->SetCarHeightOffset(0.6f);

                auto loadColliderMesh = [&](std::string const& retrieve, float friction, std::string colliderMesh)
                {
                    auto gameObject = m_Dictionary.Retrieve(retrieve);
                    if (gameObject != entt::null)
                    {
                        m_Physics->CreateMeshTerrain(gameObject, colliderMesh, friction);
                    }
                };

                loadColliderMesh("SL::application/lucre/models/mario/kicker long.glb::0::root", 2.0f,
                                 "application/lucre/models/mario/kicker long collider.glb");
            }

            // kart
            m_Kart = m_Dictionary.Retrieve("SL::application/lucre/models/mario/kart.glb::0::root");
            m_WheelsKart[0] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheelKart.glb::0::root");
            m_WheelsKart[1] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheelKart.glb::1::root");
            m_WheelsKart[2] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheelKart.glb::2::root");
            m_WheelsKart[3] = m_Dictionary.Retrieve("SL::application/lucre/models/mario/wheelKart.glb::3::root");
            if ((m_Kart != entt::null) && (m_WheelsKart[0] != entt::null) && (m_WheelsKart[1] != entt::null) &&
                (m_WheelsKart[2] != entt::null) && (m_WheelsKart[3] != entt::null))
            {

                // set up camera attached to kart
                m_Camera[CameraTypes::AttachedToKart] =
                    m_Dictionary.Retrieve("SL::application/lucre/models/mario/kart.glb::0::Scene::camera1");

                if (m_Camera[CameraTypes::AttachedToKart] != entt::null)
                {
                    auto& cameraComponent =
                        m_Registry.get<PerspectiveCameraComponent>(m_Camera[CameraTypes::AttachedToKart]);
                    m_CameraControllers[CameraTypes::AttachedToKart] = std::make_shared<CameraController>(cameraComponent);
                    m_CameraControllers[CameraTypes::AttachedToKart]->GetCamera().SetName("camera attached to kart");
                }

                m_Physics->SetGameObject(Physics::GAME_OBJECT_KART, m_Kart);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_KART_WHEEL_FRONT_LEFT, m_WheelsKart[0]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_KART_WHEEL_FRONT_RIGHT, m_WheelsKart[1]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_KART_WHEEL_REAR_LEFT, m_WheelsKart[2]);
                m_Physics->SetGameObject(Physics::GAME_OBJECT_KART_WHEEL_REAR_RIGHT, m_WheelsKart[3]);

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
                    float liftWheels = 0.0f;
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{-0.85f, liftWheels, -0.17f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetKartWheelTranslation(Physics::WHEEL_FRONT_LEFT, wheelTranslationTransform);
                        m_Physics->SetKartWheelScale(Physics::WHEEL_FRONT_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.85f, liftWheels, -0.17f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetKartWheelTranslation(Physics::WHEEL_FRONT_RIGHT, wheelTranslationTransform);
                        m_Physics->SetKartWheelScale(Physics::WHEEL_FRONT_RIGHT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{-wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{-0.85f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetKartWheelTranslation(Physics::WHEEL_REAR_LEFT, wheelTranslationTransform);
                        m_Physics->SetKartWheelScale(Physics::WHEEL_REAR_LEFT, wheelScaleTransform);
                    }
                    {
                        glm::vec3 scale{wheelScale, wheelScale, wheelScale};
                        glm::vec3 translation{0.85f, liftWheels, 0.0f};
                        glm::mat4 wheelTranslationTransform = createWheelTranslation(translation);
                        glm::mat4 wheelScaleTransform = createWheelScale(scale);
                        m_Physics->SetKartWheelTranslation(Physics::WHEEL_REAR_RIGHT, wheelTranslationTransform);
                        m_Physics->SetKartWheelScale(Physics::WHEEL_REAR_RIGHT, wheelScaleTransform);
                    }
                }
                m_Physics->SetKartHeightOffset(-0.1f);
            }
        }

        // set initial position for camera "CameraTypes::CarFollow"
        if (m_Car != entt::null)
        {
            m_CameraControllers.SetActiveCameraController(CameraTypes::CarFollow);
            SetCameraTransform();
            m_CameraControllers.SetActiveCameraController(CameraTypes::DefaultCamera);
        }

        // IBL and skybox HDRI
        {
            IBLBuilder::IBLTextureFilenames iblTextureFilenames{
                "application/lucre/models/assets/pbrScene/BRDFIntegrationMap.exr",                // BRDFIntegrationMap
                "application/lucre/models/assets/pbrScene/TeatroMassimo4k.hdr",                   // environment
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredDiffuse.exr", // envPrefilteredDiffuse
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel0.exr", // envPrefilteredSpecularLevel0
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel1.exr", // envPrefilteredSpecularLevel1
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel2.exr", // envPrefilteredSpecularLevel2
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel3.exr", // envPrefilteredSpecularLevel3
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel4.exr", // envPrefilteredSpecularLevel4
                "application/lucre/models/assets/pbrScene/TeatroMassimo1kPrefilteredSpecularLevel5.exr" // envPrefilteredSpecularLevel5
            };
            m_IBLBuilder = std::make_shared<IBLBuilder>(iblTextureFilenames);
            m_SkyboxHDRI = m_IBLBuilder->LoadSkyboxHDRI(m_Registry);
        }
    }

    void PBRScene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);
        InitPhysics();
        LoadModels();
        LoadTerrain();
        LoadScripts();
    }

    void PBRScene::LoadTerrain() {}

    void PBRScene::LoadModels()
    {
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
                SetLightView(m_Lightbulb1, m_LightView1);
            }
        }
    }

    void PBRScene::LoadScripts() {}

    void PBRScene::StartScripts() {}

    void PBRScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void PBRScene::OnUpdate(const Timestep& timestep)
    {
        ZoneScopedNC("PBRScene", 0x0000ff);

        {
            Physics::VehicleType vehicleType = m_CameraControllers.GetActiveCameraIndex() == CameraTypes::AttachedToKart
                                                   ? Physics::VehicleType::KART
                                                   : Physics::VehicleType::CAR;
            SimulatePhysics(timestep, vehicleType);
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

        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            SetCameraTransform(timestep);
        }

        { // set camera view
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);
            m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
            m_CameraControllers.GetActiveCameraController()->SetView(cameraTransform.GetMat4Global());
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
                lightView->SetOrthographicProjection(left, right, bottom, top, near, far);
                { // put the directional light in front of the currently active camera
                    // retrieve camera position and camera look at direction
                    int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
                    auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);
                    auto& cameraMat4 = cameraTransform.GetMat4Global();
                    glm::vec3 cameraPosition{cameraMat4[3]};
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
                    .m_Width = 80.0f, .m_LightBulbDistanceInCameraPlane = 20.0f, .m_LightBulbHeightOffset = 40.0f};
                lightBulbUpdate(m_DirectionalLight0, m_Lightbulb0, m_LightView0, ShadowRenderPass::HIGH_RESOLUTION,
                                parameters);
            }
            { // low-res shadow map (2nd cascade)
                Parameters parameters{
                    .m_Width = 250.0f, .m_LightBulbDistanceInCameraPlane = 100.0f, .m_LightBulbHeightOffset = 80.0f};
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

        { // 3D
            m_Renderer->Renderpass3D(m_Registry);

            ApplyDebugSettings();

            // opaque objects
            m_Renderer->Submit(*this);

            // light opaque objects
            m_Renderer->NextSubpass();
            if (m_UseIBL)
            {
                float& exposure = m_Renderer->Exposure();
                exposure = ImGUI::m_Exposure;
                std::bitset<32>& shaderSettings0 = m_Renderer->ShaderSettings0();
                shaderSettings0[0] = ImGUI::m_UseNewACES;
                shaderSettings0[1] = ImGUI::m_DoNotMultiplyColorOutWithAlbedo;
                shaderSettings0[2] = ImGUI::m_Reserved0;
                shaderSettings0[3] = ImGUI::m_Reserved1;
                m_Renderer->LightingPassIBL(m_IBLBuilder->NumMipLevelsSpecular() - 1, // uMaxPrefilterMip, number of mips - 1
                                            m_IBLBuilder->GetResourceDescriptor());
            }
            else
            {
                m_Renderer->LightingPass();
            }

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

    void PBRScene::OnEvent(Event& event)
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
                        m_CameraControllers.GetActiveCameraController()->SetProjection();
                        break;
                    }
                    case ENGINE_KEY_B:
                    {
                        m_DrawDebugMesh = !m_DrawDebugMesh;
                        break;
                    }
                    case ENGINE_KEY_I:
                    {
                        m_UseIBL = !m_UseIBL;
                        break;
                    }
                    case ENGINE_KEY_R:
                    {
                        ResetScene();
                        break;
                    }
                }
                return false;
            });
    }

    void PBRScene::OnResize() { m_CameraControllers.GetActiveCameraController()->SetProjection(); }

    void PBRScene::ResetScene()
    {
        m_CameraControllers.SetActiveCameraController(CameraTypes::DefaultCamera);
        m_CameraControllers[CameraTypes::DefaultCamera]->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[CameraTypes::DefaultCamera]);

        cameraTransform.SetTranslation({-12.314f, 11.4f, 44.0f});
        cameraTransform.SetRotation({glm::radians(-15.3f), 0.0f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraControllers[CameraTypes::DefaultCamera]->SetView(cameraTransform.GetMat4Local());
    }

    void PBRScene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
        lightView->SetView(lightbulbTransform.GetMat4Global());
    }

    void PBRScene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                       const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightView->GetDirection();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void PBRScene::ApplyDebugSettings()
    {
        if (ImGUI::m_UseNormalMapIntensity)
        {
            Model::m_NormalMapIntensity = ImGUI::m_NormalMapIntensity;
        }
        else
        {
            Model::m_NormalMapIntensity = 1.0f;
        }

        if (ImGUI::m_UseAmbientLightIntensity)
        {
            m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);
        }
    }

    void PBRScene::InitPhysics()
    {
        // Jolt
        m_Physics = Physics::Create(*this);
        {
            float heigtWaterSurface{TERRAIN_HEIGHT};
            float zOffset{2.0f};
            float scaleY{0.4f};
            Physics::GroundSpec groundSpec{.m_Scale{5.0f, scaleY, 50.0f},                                 //
                                           .m_Position{0.0f, zOffset + heigtWaterSurface - scaleY, 0.0f}, //
                                           .m_Filepath{"application/lucre/models/mario/debug box.glb"},   //
                                           .m_Friction{2.0f}};                                            //
            m_Physics->CreateGroundPlane(groundSpec); // 5x50 plane, with a small thickness
        }
        {
            float heigtWaterSurface{TERRAIN_HEIGHT};
            float zFightingOffset{-0.050f};
            float scaleY{0.4f};
            Physics::GroundSpec groundSpec{.m_Scale{500.0f, scaleY, 500.0f},                                      //
                                           .m_Position{0.0f, zFightingOffset + heigtWaterSurface - scaleY, 0.0f}, //
                                           .m_Filepath{},                                                         //
                                           .m_Friction{2.0f}};                                                    //
            m_Physics->CreateGroundPlane(groundSpec); // 500x500 plane, with a small thickness
        }

        Physics::CarParameters carParameters //
            {
                .m_Position = glm::vec3(2.0f, 20.0f, 30.0f),                        //
                .m_Rotation = glm::vec3(0.0f, TransformComponent::DEGREES_90, 0.0f) //
            };

        Physics::CarParameters kartParameters{
            .m_Position = glm::vec3(2.1f, 5.0f, 32.0f),                         //
            .m_Rotation = glm::vec3(0.0f, TransformComponent::DEGREES_90, 0.0f) //
        };
        m_Physics->LoadModels(carParameters, kartParameters);
    }

    void PBRScene::SimulatePhysics(const Timestep& timestep, Physics::VehicleType vehicleType)
    {
        // Jolt
        m_GamepadInputController->MoveVehicle(timestep, m_VehicleControl);
        m_Physics->OnUpdate(timestep, m_VehicleControl, vehicleType);
    }

    Camera& PBRScene::GetCamera() { return m_CameraControllers.GetActiveCameraController()->GetCamera(); }

    std::shared_ptr<CameraController>& PBRScene::CameraControllers::GetActiveCameraController()
    {
        return m_CameraController[m_ActiveCamera];
    }

    std::shared_ptr<CameraController>& PBRScene::CameraControllers::operator[](int index)
    {
        if ((index >= CameraTypes::MaxCameraTypes))
        {
            LOG_APP_ERROR("wrong camera indexed");
        }
        return m_CameraController[index];
    }

    PBRScene::CameraControllers& PBRScene::CameraControllers::operator++()
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

    PBRScene::CameraControllers& PBRScene::CameraControllers::operator--()
    {
        int maxChecks = static_cast<int>(CameraTypes::MaxCameraTypes);
        int nextActiveCamera = m_ActiveCamera;
        for (int iterator = 0; iterator < maxChecks; ++iterator)
        {
            --nextActiveCamera;
            if (nextActiveCamera < 0)
            {
                nextActiveCamera = maxChecks - 1;
            }
            if (m_CameraController[nextActiveCamera])
            {
                m_ActiveCamera = nextActiveCamera;
                break;
            }
        }
        LOG_APP_INFO("switching to camera {0}", m_ActiveCamera);
        return *this;
    }

    std::shared_ptr<CameraController>& PBRScene::CameraControllers::SetActiveCameraController(CameraTypes cameraType)
    {
        return SetActiveCameraController(static_cast<int>(cameraType));
    }

    std::shared_ptr<CameraController>& PBRScene::CameraControllers::SetActiveCameraController(int index)
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

    void PBRScene::CameraControllers::SetProjectionAll()
    {
        for (uint index = 0; index < CameraTypes::MaxCameraTypes; ++index)
        {
            if (m_CameraController[index])
            {
                m_CameraController[index]->SetProjection();
            }
        }
    }

    void PBRScene::SetCameraTransform(const Timestep& timestep)
    {
        int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);
        if (activeCameraIndex == CameraTypes::CarFollow)
        {
            if (m_Car != entt::null)
            {
                float followDistance{-10.0f};
                float followHeight{1.0};
                auto& carTransform = m_Registry.get<TransformComponent>(m_Car);
                auto const& carMat4 = carTransform.GetMat4Local(); // assuming it has no parent
                glm::vec3 forward{0.0f, 0.0f, -1.0f};              // For right-handed
                glm::vec3 carForward = glm::normalize(glm::mat3(carMat4) * forward);
                glm::vec3 newPosition = carForward * followDistance + carTransform.GetTranslation();
                newPosition.y += followHeight;
                cameraTransform.SetTranslation(newPosition);
                glm::vec3 newRotation = carTransform.GetRotation();
                cameraTransform.SetRotation(newRotation);
            }
        }
        else
        {
            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
        }
    }
} // namespace LucreApp
