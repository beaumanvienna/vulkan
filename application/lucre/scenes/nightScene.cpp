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

#include "nightScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"
#include "animation/easingFunctions.h"

namespace LucreApp
{

    NightScene::NightScene(const std::string& filepath, const std::string& alternativeFilepath)
        : Scene(filepath, alternativeFilepath), m_SceneLoaderJSON{*this}, m_LaunchVolcanoTimer(1500),
          m_RunCameraAnimation{false}
    {
    }

    void NightScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Renderer->UpdateTransformCache(*this, SceneGraph::ROOT_NODE, glm::mat4(1.0f), false);
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        { // set up camera // set up camera
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

            for (auto& easingAnimation : m_EasingAnimation)
            {
                AssignAnimation(easingAnimation);
            }
        }

        StartScripts();
        m_SceneGraph.TraverseLog(SceneGraph::ROOT_NODE);
        m_Dictionary.List();

        // get characters and start all animations
        m_NonPlayableCharacter1 =
            m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/monkey01/monkey01.glb::0::root");
        m_Hero = m_Dictionary.Retrieve("SL::application/lucre/models/external_3D_files/CesiumMan/animations/"
                                       "CesiumManAnimations.gltf::0::Scene::Cesium_Man");
        if (m_Hero != entt::null)
        {
            if (m_Registry.all_of<SkeletalAnimationTag>(m_Hero))
            {
                auto& mesh = m_Registry.get<MeshComponent>(m_Hero);
                SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
                animations.SetRepeatAll(true);
                animations.Start();
            }
            else
            {
                LOG_APP_CRITICAL("entity {0} must have skeletal animation tag", static_cast<int>(m_Hero));
            }
        }
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

                m_CharacterAnimation = std::make_unique<CharacterAnimation>(m_Registry, model, animations);
                m_CharacterAnimation->Start();
            }
        }
        else
        {
            if ((m_Hero != entt::null) && m_Registry.all_of<SkeletalAnimationTag>(m_Hero))
            {
                auto& mesh = m_Registry.get<MeshComponent>(m_Hero);
                SkeletalAnimations& animations = mesh.m_Model->GetAnimations();

                entt::entity model = m_Dictionary.Retrieve(
                    "SL::application/lucre/models/external_3D_files/CesiumMan/animations/CesiumManAnimations.gltf::0::root");
                if (model != entt::null)
                {
                    m_CharacterAnimation = std::make_unique<CharacterAnimation>(m_Registry, model, animations);
                    m_CharacterAnimation->Start();
                }
            }
        }

        m_NonPlayableCharacter2 =
            m_Dictionary.Retrieve("SL::application/lucre/models/Kaya/gltf/Kaya.glb::0::Scene::Kaya Body_Mesh");
        if (m_NonPlayableCharacter2 != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacter2);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        m_NonPlayableCharacter3 =
            m_Dictionary.Retrieve("SL::application/lucre/models/Kaya/gltf/Kaya.glb::1::Scene::Kaya Body_Mesh");
        if (m_NonPlayableCharacter3 != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacter3);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        {
            // place static lights for beach scene
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

        {
            m_LaunchVolcanoTimer.SetEventCallback(
                [](uint in, void* data)
                {
                    std::unique_ptr<Event> event = std::make_unique<KeyPressedEvent>(ENGINE_KEY_G);
                    Engine::m_Engine->QueueEvent(event);
                    return 0u;
                });
            m_LaunchVolcanoTimer.Start();

            // volcano smoke animation
            int poolSize = 50;
            m_SpritesheetSmoke.AddSpritesheetTile(Lucre::m_Spritesheet->GetSprite(I_VOLCANO_SMOKE),
                                                  "volcano smoke sprite sheet", 8, 8, /* rows, columns */
                                                  0,                                  /* margin */
                                                  0.01f                               /* scale) */
            );
            m_VolcanoSmoke = std::make_shared<ParticleSystem>(poolSize, &m_SpritesheetSmoke, 5.0f /*amplification*/);
        }
    }

    void NightScene::Load()
    {
        m_SceneLoaderJSON.Deserialize(m_Filepath, m_AlternativeFilepath);
        ImGUI::SetupSlider(this);

        LoadModels();
        LoadScripts();
    }

    void NightScene::LoadModels()
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

                    transform.SetScale({0.00999978, 0.0100001, 0.0100001});
                    transform.SetRotation({-0.888632, -0.571253, -0.166816});
                    transform.SetTranslation({1.5555, 4, -4.13539});

                    m_Registry.emplace<TransformComponent>(m_Lightbulb0, transform);
                }
                m_LightView0 = std::make_shared<Camera>(Camera::ProjectionType::ORTHOGRAPHIC_PROJECTION);
                float left = -4.0f;
                float right = 4.0f;
                float bottom = -4.0f;
                float top = 4.0f;
                float near = 0.1f;
                float far = 10.0f;
                m_LightView0->SetOrthographicProjection(left, right, bottom, top, near, far);
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
                m_LightView1->SetOrthographicProjection(left, right, bottom, top, near, far);
                SetLightView(m_Lightbulb1, m_LightView1);
            }
        }
    }

    void NightScene::LoadScripts()
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

    void NightScene::StartScripts()
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

    void NightScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoaderJSON.Serialize();
    }

    void NightScene::OnUpdate(const Timestep& timestep)
    {
        if (m_RunCameraAnimation)
        {
            static auto animateCamera = [&]()
            {
                float speedXZCamRot[ANIMATE_X_Z_CAMROT] = {0.0f, 0.0f};
                m_RunCameraAnimation = m_EasingAnimation[0].Run(speedXZCamRot);
                if (m_RunCameraAnimation)
                {
                    auto& transform = m_Registry.get<TransformComponent>(m_Camera);
                    float speedFactor = timestep;
                    transform.AddTranslation({speedXZCamRot[X] * speedFactor, 0.0f, speedXZCamRot[Z] * speedFactor});
                    transform.SetRotationY(speedXZCamRot[CAMROT]);
                }
            };

            animateCamera();
        }
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_GamepadInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetView(cameraTransform.GetMat4Global());
        }

        AnimateHero(timestep);
        if (m_CharacterAnimation)
            m_CharacterAnimation->OnUpdate(timestep);
        SetLightView(m_Lightbulb0, m_LightView0);
        SetLightView(m_Lightbulb1, m_LightView1);
        SetDirectionalLight(m_DirectionalLight0, m_Lightbulb0, m_LightView0, 0 /*shadow renderpass*/);
        SetDirectionalLight(m_DirectionalLight1, m_Lightbulb1, m_LightView1, 1 /*shadow renderpass*/);

        // volcano
        EmitVolcanoSmoke();
        m_VolcanoSmoke->OnUpdate(timestep);

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera());
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
        m_Renderer->TransparencyPass(m_Registry, m_VolcanoSmoke.get());

        // post processing
        m_Renderer->PostProcessingRenderpass();

        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
    }

    void NightScene::OnEvent(Event& event)
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
            [this](KeyPressedEvent l_Event)
            {
                switch (l_Event.GetKeyCode())
                {
                    case ENGINE_KEY_N:
                    {
                        break;
                    }
                    case ENGINE_KEY_B:
                    {
                        m_RunCameraAnimation = !m_RunCameraAnimation;
                        if (m_RunCameraAnimation)
                        {
                            {
                                auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);
                                cameraTransform.SetTranslation({0.1f, 2.7f, 12.4f});
                                cameraTransform.SetRotation({-0.055f, 0.0f, 0.0f});
                            }
                            m_EasingAnimation[0].Start();
                        }
                        break;
                    }
                }
                return false;
            });
    }

    void NightScene::OnResize() { m_CameraController->SetProjection(); }

    void NightScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);

        cameraTransform.SetTranslation({1.714f, 3.275f, 12.956f});
        cameraTransform.SetRotation({-0.055f, 0.0f, 0.0f});

        // global camera transform is not yet available
        // because UpdateTransformCache didn't run yet
        // for default camera: global == local transform
        m_CameraController->SetView(cameraTransform.GetMat4Local());
    }

    void NightScene::RotateLights(const Timestep& timestep)
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

    void NightScene::AnimateHero(const Timestep& timestep)
    {
        if (m_NonPlayableCharacter1 == entt::null)
            return;

        auto& heroTransform = m_Registry.get<TransformComponent>(m_NonPlayableCharacter1);

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

    void NightScene::SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView)
    {
        {
            auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
            lightView->SetView(lightbulbTransform.GetMat4Global());
        }
    }

    void NightScene::SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                         const std::shared_ptr<Camera>& lightView, int renderpass)
    {
        auto& lightbulbTransform = m_Registry.get<TransformComponent>(lightbulb);
        auto& directionalLightComponent = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction = lightbulbTransform.GetRotation();
        directionalLightComponent.m_LightView = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void NightScene::EmitVolcanoSmoke()
    {
        static auto start = Engine::m_Engine->GetTime();
        if ((Engine::m_Engine->GetTime() - start) > 1000ms)
        {
            start = Engine::m_Engine->GetTime();

            ParticleSystem::Specification spec = {
                {4.09f, 2.641f, -1.338f}, // glm::vec3 m_Position
                {0.0f, 0.0125f, 0.0f},    // glm::vec3 m_Velocity
                {0.0f, 0.0f, 0.0f},       // glm::vec3 m_Acceleration

                {0.0f, TransformComponent::DEGREES_90, 0.0f}, // glm::vec3 m_Rotation
                {0.0f, 0.0f, 0.0f},                           // float m_RotationSpeed

                {1.0f, 1.0f, 1.0f, 1.0f}, // glm::vec4 m_StartColor
                {1.0f, 1.0f, 1.0f, 0.0f}, // glm::vec4 m_EndColor

                0.005f, // float m_StartSize
                0.07f,  // float m_FinalSize

                {6s}, // Timestep m_LifeTime
            };

            ParticleSystem::Specification variation{};
            variation.m_Position = {0.0001f, 0.0f, 0.0f}; // a little x against z-fighting
            variation.m_Velocity = {0.0f, 0.002f, 0.0f};
            variation.m_Rotation = {0.0f, 0.5f, 0.0f};
            m_VolcanoSmoke->Emit(spec, variation);
        }
    }

    void NightScene::ApplyDebugSettings()
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

    void NightScene::AssignAnimation(EasingAnimations<ANIMATE_X_Z_CAMROT>& easingAnimation)
    {
        float speed = 1.44f;
        float s = 1.0f; // time stretch
        // go forward: z speed from -0.2f * speed to -1.2 * speed
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseConstant>("1 X Constant", 0.0f /*scale*/, 0.0f /*offset*/);
            std::shared_ptr<EasingAnimation> animationZ =
                std::make_shared<EaseInOutQuart>("1 Z EaseInOutQuart", speed * -1.0f /*scale*/, speed * -0.2f /*offset*/);
            std::shared_ptr<EasingAnimation> animationCamRot =
                std::make_shared<EaseConstant>("1 C Constant", 0.0f /*scale*/, 0.0f /*offset*/);

            EasingAnimations<ANIMATE_X_Z_CAMROT>::AnimationsXY animation(s * 2s, {animationX, animationZ, animationCamRot});
            easingAnimation.PushAnimation(animation);
        }

        // turn right: z speed -1.2 * speed
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("1 X Constant", speed * 0.1f /*scale*/, 0.0f /*offset*/);
            std::shared_ptr<EasingAnimation> animationZ =
                std::make_shared<EaseConstant>("1 Z Constant", 0.0f /*scale*/, speed * -1.2f /*offset*/);
            std::shared_ptr<EasingAnimation> animationCamRot = std::make_shared<EaseInOutQuart>(
                "1 C EaseInOutQuart", -TransformComponent::DEGREES_90 / 2.0f /*scale*/, 0.0f /*offset*/);

            EasingAnimations<ANIMATE_X_Z_CAMROT>::AnimationsXY animation(s * 5s, {animationX, animationZ, animationCamRot});
            easingAnimation.PushAnimation(animation);
        }

        // turn right: z speed from -1.2 * speed to 0
        {
            std::shared_ptr<EasingAnimation> animationX =
                std::make_shared<EaseInOutQuart>("1 X Constant", -speed * 0.1f /*scale*/, speed * 0.1f /*offset*/);
            std::shared_ptr<EasingAnimation> animationZ =
                std::make_shared<EaseInOutQuart>("2 Z EaseInOutQuart", speed * 1.2f /*scale*/, -speed * 1.2f /*offset*/);
            std::shared_ptr<EasingAnimation> animationCamRot =
                std::make_shared<EaseInOutQuart>("1 C EaseInOutQuart", -TransformComponent::DEGREES_90 / 2.0f /*scale*/,
                                                 -TransformComponent::DEGREES_90 / 2.0f /*offset*/);

            EasingAnimations<ANIMATE_X_Z_CAMROT>::AnimationsXY animation(s * 5s, {animationX, animationZ, animationCamRot});
            easingAnimation.PushAnimation(animation);
        }
        easingAnimation.SetLoop(false);
    }
} // namespace LucreApp
