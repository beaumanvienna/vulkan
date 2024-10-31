/* Engine Copyright (c) 2024 Engine Development Team
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

#include "dessertScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"
#include "animation/easingFunctions.h"

namespace LucreApp
{

    DessertScene::DessertScene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}
    {
    }

    DessertScene::~DessertScene() {}

    void DessertScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        { // set up camera

            float aspectRatio = 1.777f;
            float yfov = 0.51f;
            float znear = 0.1f;
            float zfar = 500.0f;

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

        {
            auto sceneLights = m_Dictionary.Retrieve("SceneLights");
            if (sceneLights != entt::null)
            {
                auto& transform = m_Registry.get<TransformComponent>(sceneLights);
                transform.SetTranslation({0.0f, 0.5f, 2.0f});
            }
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
    }

    void DessertScene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);

        LoadModels();
        LoadScripts();
    }

    void DessertScene::LoadModels()
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
    }

    void DessertScene::LoadScripts() {}

    void DessertScene::StartScripts() {}

    void DessertScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void DessertScene::OnUpdate(const Timestep& timestep)
    {
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            int activeCameraIndex = m_CameraControllers.GetActiveCameraIndex();
            auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera[activeCameraIndex]);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraControllers.GetActiveCameraController()->SetView(cameraTransform.GetMat4Global());
        }

        AnimateHero(timestep);
        if (m_CharacterAnimation)
        {
            m_CharacterAnimation->OnUpdate(timestep);
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
        m_Renderer->BeginFrame(&m_CameraControllers.GetActiveCameraController()->GetCamera());
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

    void DessertScene::OnEvent(Event& event)
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
                        break;
                    }
                }
                return false;
            });
    }

    void DessertScene::OnResize() { m_CameraControllers.SetProjectionAll(); }

    void DessertScene::ResetScene()
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

    void DessertScene::RotateLights(const Timestep& timestep)
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

    void DessertScene::AnimateHero(const Timestep& timestep) {}

    void DessertScene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);

            glm::vec3 position = lightbulbTransform.GetTranslation();
            glm::vec3 rotation = lightbulbTransform.GetRotation();
            lightView->SetViewYXZ(position, rotation);
        }
    }

    void DessertScene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                           const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightbulbTransform.GetRotation();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void DessertScene::ApplyDebugSettings()
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

    Camera& DessertScene::GetCamera() { return m_CameraControllers.GetActiveCameraController()->GetCamera(); }

    std::shared_ptr<CameraController>& DessertScene::CameraControllers::GetActiveCameraController()
    {
        return m_CameraController[m_ActiveCamera];
    }

    std::shared_ptr<CameraController>& DessertScene::CameraControllers::operator[](int index)
    {
        if ((index >= CameraTypes::MaxCameraTypes))
        {
            LOG_APP_ERROR("wrong camera indexed");
        }
        return m_CameraController[index];
    }

    DessertScene::CameraControllers& DessertScene::CameraControllers::operator++()
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

    std::shared_ptr<CameraController>& DessertScene::CameraControllers::SetActiveCameraController(CameraTypes cameraType)
    {
        return SetActiveCameraController(static_cast<int>(cameraType));
    }

    std::shared_ptr<CameraController>& DessertScene::CameraControllers::SetActiveCameraController(int index)
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

    void DessertScene::CameraControllers::SetProjectionAll()
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
