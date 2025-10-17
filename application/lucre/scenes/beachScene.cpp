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

#include "core.h"
#include "events/event.h"
#include "events/keyEvent.h"
#include "events/mouseEvent.h"
#include "resources/resources.h"
#include "gui/Common/UI/screen.h"
#include "auxiliary/math.h"

#include "beachScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"

namespace LucreApp
{

    BeachScene::BeachScene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}
    {
    }

    void BeachScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        {
            // set up camera
            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 500.0f;

            PerspectiveCameraComponent perspectiveCameraComponent(aspectRatio, yfov, znear, zfar);
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
        m_NonPlayableCharacter =
            m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/monkey01/monkey01.glb::0::root");

        {
            // place static lights for beach scene
            float intensity = 5.0f;
            float lightRadius = 0.1f;
            float height1 = 0.4f;
            std::vector<glm::vec3> lightPositions = {{-0.285, height1, -2.8}, {-3.2, height1, -2.8}, {-6.1, height1, -2.8},
                                                     {2.7, height1, -2.8},    {5.6, height1, -2.8},  {-0.285, height1, 0.7},
                                                     {-3.2, height1, 0.7},    {-6.1, height1, 0.7},  {2.7, height1, 0.7},
                                                     {5.6, height1, 0.7}};

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
    }

    void BeachScene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);

        LoadModels();
        LoadScripts();
    }

    void BeachScene::LoadModels()
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

    void BeachScene::LoadScripts()
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

    void BeachScene::StartScripts()
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

    void BeachScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void BeachScene::OnUpdate(const Timestep& timestep)
    {
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetView(cameraTransform.GetMat4Global());
        }

        AnimateHero(timestep);
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
                    auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);
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
                    .m_Width = 20.0f, .m_LightBulbDistanceInCameraPlane = 10.0f, .m_LightBulbHeightOffset = 10.0f};
                lightBulbUpdate(m_DirectionalLight0, m_Lightbulb0, m_LightView0, ShadowRenderPass::HIGH_RESOLUTION,
                                parameters);
            }
            { // low-res shadow map (2nd cascade)
                Parameters parameters{
                    .m_Width = 75.0f, .m_LightBulbDistanceInCameraPlane = 75.0f, .m_LightBulbHeightOffset = 20.0f};
                lightBulbUpdate(m_DirectionalLight1, m_Lightbulb1, m_LightView1, ShadowRenderPass::LOW_RESOLUTION,
                                parameters);
            }
        }

        // draw new scene
        if (!m_Renderer->BeginFrame(&m_CameraController->GetCamera()))
        {
            return;
        }
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
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

    void BeachScene::OnEvent(Event& event)
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
    }

    void BeachScene::OnResize() { m_CameraController->SetProjection(); }

    void BeachScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);

        cameraTransform.SetTranslation({-0.8f, 2.0f, 7.5f});
        cameraTransform.SetRotation({0.0f, 0.0f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraController->SetView(cameraTransform.GetMat4Local());
    }

    void BeachScene::RotateLights(const Timestep& timestep)
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

    void BeachScene::AnimateHero(const Timestep& timestep)
    {
        if (m_NonPlayableCharacter == entt::null)
            return;
        auto view = m_Registry.view<TransformComponent>();
        auto& heroTransform = view.get<TransformComponent>(m_NonPlayableCharacter);

        static float deltaX = 0.5f;
        static float deltaY = 0.5f;
        static float deltaZ = 0.5f;

        constexpr float DEFORM_X_SPEED = 0.2f;
        static float deformX = DEFORM_X_SPEED;

        if (deltaX > 0.55f)
        {
            deformX = -DEFORM_X_SPEED;
        }
        else if (deltaX < 0.45f)
        {
            deformX = DEFORM_X_SPEED;
        }

        deltaX += deformX * timestep;
        heroTransform.SetScale({deltaX, deltaY, deltaZ});
    }

    void BeachScene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
            lightView->SetView(lightbulbTransform.GetMat4Global());
        }
    }

    void BeachScene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                         const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightView->GetDirection();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void BeachScene::ApplyDebugSettings()
    {
        if (ImGUI::m_UseAmbientLightIntensity)
        {
            m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);
        }
    }
} // namespace LucreApp
