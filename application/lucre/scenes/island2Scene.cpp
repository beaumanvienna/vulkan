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

#include "island2Scene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"
#include "animation/easingFunctions.h"

namespace LucreApp
{

    Island2Scene::Island2Scene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}, m_RunLightAnimation{true}
    {
    }

    Island2Scene::~Island2Scene() {}

    void Island2Scene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        { // set up camera

            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 500.0f;

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

        StartScripts();
        m_SceneGraph.TraverseLog(SceneGraph::ROOT_NODE);
        m_Dictionary.List();

        m_Camera[CameraTypes::AttachedToLight] =
            m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/lights/gltf/lights.glb::0::Scene::Camera");
        // set up 2nd camera
        if (m_Camera[CameraTypes::AttachedToLight] != entt::null)
        {
            auto& cameraComponent = m_Registry.get<PerspectiveCameraComponent>(m_Camera[CameraTypes::AttachedToLight]);
            m_CameraControllers[CameraTypes::AttachedToLight] = std::make_shared<CameraController>(cameraComponent);
            m_CameraControllers[CameraTypes::AttachedToLight]->GetCamera().SetName("camera attached to light");
        }
        // set up moving lights
        {
            int lightsIndex = 0;
            for (int index = 0; index < 3; ++index)
            {
                m_MovingLights[lightsIndex] = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/lights/gltf/lights.glb::0::Scene::LightModel" +
                    std::to_string(index + 1));
                ++lightsIndex;
            }
            for (int index = 0; index < 3; ++index)
            {
                m_MovingLights[lightsIndex] = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/lights/gltf/lights.glb::1::Scene::LightModel" +
                    std::to_string(index + 1));
                ++lightsIndex;
            }
            if (m_MovingLights[0] != entt::null)
            {
                for (auto& easingAnimation : m_EasingAnimation)
                {
                    AssignAnimation(easingAnimation);
                }
            }
        }

        {
            auto sceneLights = m_Dictionary.Retrieve("SceneLights");
            if (sceneLights != entt::null)
            {
                auto& transform = m_Registry.get<TransformComponent>(sceneLights);
                transform.SetTranslation({0.0f, 0.5f, 2.0f});
            }
        }
        m_Water = m_Dictionary.Retrieve(
            "SL::application/lucre/models/external_3D_files/Island scene/gltf/Island2.glb::0::Scene::Water");

        // get characters and start all animations
        m_Guybrush = m_Dictionary.Retrieve(
            "SL::application/lucre/models/guybrush_animated_gltf/animation/guybrush.glb::0::Scene::guybrush object");
        if (m_Guybrush != entt::null)
        {
            if (m_Registry.all_of<SkeletalAnimationTag>(m_Guybrush))
            {
                auto& mesh = m_Registry.get<MeshComponent>(m_Guybrush);
                SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
                animations.SetRepeatAll(true);
                animations.Start();
            }
            else
            {
                LOG_APP_CRITICAL("entity {0} must have skeletal animation tag", static_cast<int>(m_Guybrush));
            }
        }

        // start gamepad-based control for characters
        if (m_Guybrush != entt::null)
        {
            if (m_Registry.all_of<SkeletalAnimationTag>(m_Guybrush))
            {
                auto& mesh = m_Registry.get<MeshComponent>(m_Guybrush);
                SkeletalAnimations& animations = mesh.m_Model->GetAnimations();

                entt::entity model = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/guybrush_animated_gltf/animation/guybrush.glb::0::Scene::Armature");
                if (model != entt::null)
                {
                    m_CharacterAnimation = std::make_unique<CharacterAnimation>(m_Registry, model, animations);
                    m_CharacterAnimation->Start();
                }
            }
        }

        m_NonPlayableCharacters[NPC::Character2] =
            m_Dictionary.Retrieve("SL::application/lucre/models/Kaya/gltf/Kaya.glb::0::Scene::Kaya Body_Mesh");
        if (m_NonPlayableCharacters[NPC::Character2] != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacters[NPC::Character2]);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        m_NonPlayableCharacters[NPC::Character3] =
            m_Dictionary.Retrieve("SL::application/lucre/models/Kaya/gltf/Kaya.glb::1::Scene::Kaya Body_Mesh");
        if (m_NonPlayableCharacters[NPC::Character3] != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacters[NPC::Character3]);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        m_NonPlayableCharacters[NPC::Character1] =
            m_Dictionary.Retrieve("SL::application/lucre/models/dancing/gltf/Dancing Michelle.glb::0::Scene::Michelle");
        if (m_NonPlayableCharacters[NPC::Character1] != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacters[NPC::Character1]);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        m_NonPlayableCharacters[NPC::Character4] =
            m_Dictionary.Retrieve("SL::application/lucre/models/dancing/gltf/Dancing Michelle.glb::1::Scene::Michelle");
        if (m_NonPlayableCharacters[NPC::Character4] != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacters[NPC::Character4]);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start(0 /*dancing 1*/);
        }

        {
            // place static lights
            float intensity = 5.0f;
            float lightRadius = 0.1f;
            float height1 = 1.785f;
            std::vector<glm::vec3> lightPositions = {{-0.285, height1, -2.8}, {-3.2, height1, -2.8}, {-6.1, height1, -2.8},
                                                     {2.7, height1, -2.8},    {5.6, height1, -2.8},  {-0.285, height1, 0.7},
                                                     {-3.2, height1, 0.7},    {-6.1, height1, 0.7},  {2.7, height1, 0.7},
                                                     {5.6, height1, 0.7}};

            for (size_t i = 0; i < lightPositions.size(); i++)
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
        m_SceneStartTime = Engine::m_Engine->GetTime();
    }

    void Island2Scene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);

        LoadModels();
        LoadScripts();
    }

    void Island2Scene::LoadModels()
    {
        { // cube map / skybox
            std::vector<std::string> faces = {"application/lucre/models/external_3D_files/night/right.png",
                                              "application/lucre/models/external_3D_files/night/left.png",
                                              "application/lucre/models/external_3D_files/night/top.png",
                                              "application/lucre/models/external_3D_files/night/bottom.png",
                                              "application/lucre/models/external_3D_files/night/front.png",
                                              "application/lucre/models/external_3D_files/night/back.png"};

            Builder builder;
            m_Skybox = builder.LoadCubemap(faces, m_Registry);
            auto view = m_Registry.view<TransformComponent>();
            auto& skyboxTransform = view.get<TransformComponent>(m_Skybox);
            skyboxTransform.SetScale(250.0f);
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
                SetLightView(m_Lightbulb1, m_LightView1);
            }
        }
    }

    void Island2Scene::LoadScripts() {}

    void Island2Scene::StartScripts() {}

    void Island2Scene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void Island2Scene::OnUpdate(const Timestep& timestep)
    {
        if (m_RunLightAnimation)
        {
            auto animateLight = [&](int light, Duration delay)
            {
                TimePoint currenTime = Engine::m_Engine->GetTime();
                if (!m_EasingAnimation[light].IsRunning() && (currenTime - m_SceneStartTime > delay))
                {
                    m_EasingAnimation[light].Start();
                }
                if (m_EasingAnimation[light].IsRunning())
                {
                    float speedXY[ANIMATE_X_Y] = {0.0f, 0.0f};
                    m_EasingAnimation[light].Run(speedXY);
                    auto& transform = m_Registry.get<TransformComponent>(m_MovingLights[light]);
                    float speedFactor = timestep * 2.0f;
                    transform.AddTranslation({speedXY[0] * speedFactor, speedXY[1] * speedFactor, 0.0f});
                }
            };
            std::array<Duration, NUMBER_OF_MOVING_LIGHTS> startDelays{3s, 2s, 1s, 3s, 2s, 1s};
            int light{0};
            for (auto& startDelay : startDelays)
            {
                if (m_MovingLights[light] != entt::null)
                {
                    animateLight(light, startDelay);
                }
                ++light;
            }
        }
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraControllers.GetActiveCameraController()->SetView(cameraTransform.GetMat4Global());
        }

        if (m_Water != entt::null)
        {
            auto& transform = m_Registry.get<TransformComponent>(m_Water);
            transform.AddRotation({0.0f, 0.1f * timestep, 0.0f});
        }

        AnimateHero(timestep);
        if (m_CharacterAnimation)
        {
            m_CharacterAnimation->OnUpdate(timestep);
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
                    .m_Width = 10.0f, .m_LightBulbDistanceInCameraPlane = 5.0f, .m_LightBulbHeightOffset = 10.0f};
                lightBulbUpdate(m_DirectionalLight0, m_Lightbulb0, m_LightView0, ShadowRenderPass::HIGH_RESOLUTION,
                                parameters);
            }
            { // low-res shadow map (2nd cascade)
                Parameters parameters{
                    .m_Width = 40.0f, .m_LightBulbDistanceInCameraPlane = 40.0f, .m_LightBulbHeightOffset = 20.0f};
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
        m_Renderer->Renderpass3D(m_Registry);

        RotateLights(timestep);
        ApplyDebugSettings();

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

    void Island2Scene::OnEvent(Event& event)
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
            [this](KeyPressedEvent l_Event)
            {
                switch (l_Event.GetKeyCode())
                {
                    case ENGINE_KEY_N:
                    {
                        ++m_CameraControllers;
                        m_CameraControllers.GetActiveCameraController()->SetProjection();
                        break;
                    }
                    case ENGINE_KEY_R:
                    {
                        m_RunLightAnimation = true;
                        auto& transform = m_Registry.get<TransformComponent>(m_MovingLights[0]);
                        transform.SetTranslation({0.0f, 0.0f, 0.0f});
                        m_EasingAnimation[0].Start();
                        break;
                    }
                }
                return false;
            });
    }

    void Island2Scene::OnResize() { m_CameraControllers.SetProjectionAll(); }

    void Island2Scene::ResetScene()
    {
        m_CameraControllers.SetActiveCameraController(CameraTypes::DefaultCamera);
        m_CameraControllers[CameraTypes::DefaultCamera]->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[CameraTypes::DefaultCamera]);

        cameraTransform.SetTranslation({0.0f, 3.0f, 10.0f});
        cameraTransform.SetRotation({0.0f, 0.0f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraControllers[CameraTypes::DefaultCamera]->SetView(cameraTransform.GetMat4Local());
    }

    void Island2Scene::RotateLights(const Timestep& timestep)
    {
        float time = 0.3f * timestep;
        auto rotateLight = glm::rotate(glm::mat4(1.f), time, {0.f, -1.f, 0.f});

        auto view = m_Registry.view<PointLightComponent, TransformComponent, Group1>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            transform.SetTranslation(glm::vec3(rotateLight * glm::vec4(transform.GetTranslation(), 1.f)));
        }
    }

    void Island2Scene::AnimateHero(const Timestep& timestep) {}

    void Island2Scene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
            lightView->SetView(lightbulbTransform.GetMat4Global());
        }
    }

    void Island2Scene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                           const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightView->GetDirection();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void Island2Scene::ApplyDebugSettings()
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

    Camera& Island2Scene::GetCamera() { return m_CameraControllers.GetActiveCameraController()->GetCamera(); }

    std::shared_ptr<CameraController>& Island2Scene::CameraControllers::GetActiveCameraController()
    {
        return m_CameraController[m_ActiveCamera];
    }

    std::shared_ptr<CameraController>& Island2Scene::CameraControllers::operator[](int index)
    {
        if ((index >= CameraTypes::MaxCameraTypes))
        {
            LOG_APP_ERROR("wrong camera indexed");
        }
        return m_CameraController[index];
    }

    Island2Scene::CameraControllers& Island2Scene::CameraControllers::operator++()
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

    std::shared_ptr<CameraController>& Island2Scene::CameraControllers::SetActiveCameraController(CameraTypes cameraType)
    {
        return SetActiveCameraController(static_cast<int>(cameraType));
    }

    std::shared_ptr<CameraController>& Island2Scene::CameraControllers::SetActiveCameraController(int index)
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

    void Island2Scene::CameraControllers::SetProjectionAll()
    {
        for (uint index = 0; index < CameraTypes::MaxCameraTypes; ++index)
        {
            if (m_CameraController[index])
            {
                m_CameraController[index]->SetProjection();
            }
        }
    }

    void Island2Scene::AssignAnimation(EasingAnimations<ANIMATE_X_Y>& easingAnimation)
    {
        float speedOffset = 1.0f;
        float speed = 1.0f;
        float speedXLeft = -speed;
        float s = 1.0f; // time stretch
        // go left (x: from -1 to -2; y: 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("1 X EaseInOutQuart", -speedOffset /*scale*/, speedXLeft /*offset*/);
            std::shared_ptr<EasingAnimation> animationY = std::make_shared<EaseConstant>("1 Y Constant", 0.0f, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left and up (x: -2, y: from 0 to 2)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseConstant>("2 X Constant", -speedOffset + speedXLeft /*scale*/, 0.0f /*offset*/);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("2 Y EaseInOutQuart", 0.0f, 2.0f * speed);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left and up (x: -2, y: from 2 to 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseConstant>("3 X Constant", -speedOffset + speedXLeft, 0.0f);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("3 Y EaseInOutQuart", 0.0f, 2.0f * speed, INVERT_EASE);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left and down (x: -2, y from 0 to -2)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseConstant>("4 X Constant", -speedOffset + speedXLeft, 0.0f);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("4 Y EaseInOutQuart", 0.0f, -2.0f * speed);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left and down (x: -2, y from -2 to 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseConstant>("5 X Constant", -speedOffset + speedXLeft, 0.0f);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("5 Y EaseInOutQuart", 0.0f, -2.0f * speed, INVERT_EASE);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        /////////// go up
        // go left vertical (x: -2 to 0, y: 0 to 2)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("6 X EaseInOutQuart", 0.0f, -speedOffset + speedXLeft, INVERT_EASE);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("6 Y EaseInOutQuart", 0.0f, 2 * speed);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go straight up (x: 0, y: 2)
        {
            std::shared_ptr<EasingAnimation> animationX = std::make_shared<EaseConstant>("7 X EaseConstant", 0.0f, 0.0f);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseConstant>("7 Y EaseConstant", 0.0f, 2.0f * speed);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 2s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        /////////// go right
        // go right horizontally (x: 0 to 2, y: 2 to 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("8 X EaseInOutQuart", 0.0f, 2.0f * speed);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("8 Y EaseInOutQuart", 0.0f, 2.0f * speed, INVERT_EASE);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go right fast (X: 2 to 20, y:0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("9 X EaseInOutQuart", 2.0f * speedOffset, 18.0f * speed);
            std::shared_ptr<EasingAnimation> animationY = std::make_shared<EaseConstant>("9 Y EaseConstant", 0.0f, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go right and slow down (x: 20 to 2, y:0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("10 X EaseInOutQuart", 2 * speedOffset, 20.0f * speed, INVERT_EASE);
            std::shared_ptr<EasingAnimation> animationY = std::make_shared<EaseConstant>("10 Y EaseConstant", 0.0f, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 0.5s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go down (x: 2 to 0, y: 0 to -2)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("11 X EaseInOutQuart", 0.0f, 2.0f * speed, INVERT_EASE);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("11 Y EaseInOutQuart", 0.0f, -2.0f * speed);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go straight down (x: 0, y: -2)
        {
            std::shared_ptr<EasingAnimation> animationX = std::make_shared<EaseConstant>("12 X EaseConstant", 0.0f, 0.0f);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseConstant>("12 Y EaseConstant", -2.0f * speed, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 2s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left (x: 0 to -2, y: -2 to 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("13 X EaseInOutQuart", 0.0f, -2.0f * speed);
            std::shared_ptr<EasingAnimation> animationY =
                std::make_shared<EaseInOutQuart>("13 Y EaseInOutQuart", 0.0f, -2.0f * speed, INVERT_EASE);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 1s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go straight left (x: -2 to -6, y: 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("14 X EaseInOutQuart", -2.0f * speedOffset, -4.0f * speed);
            std::shared_ptr<EasingAnimation> animationY = std::make_shared<EaseConstant>("14 Y EaseConstant", 0.0f, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 0.7s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        // go left, slow down (x: -6 to -1, y: 0)
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("15 X EaseInOutQuart", -speedOffset, -5.0f * speed, INVERT_EASE);
            std::shared_ptr<EasingAnimation> animationY = std::make_shared<EaseConstant>("15 Y EaseConstant", 0.0f, 0.0f);
            EasingAnimations<ANIMATE_X_Y>::AnimationsXY animation(s * 3s, {animationX, animationY});
            easingAnimation.PushAnimation(animation);
        }
        easingAnimation.SetLoop(true);
    }

} // namespace LucreApp
