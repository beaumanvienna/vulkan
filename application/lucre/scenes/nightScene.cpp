/* Engine Copyright (c) 2022 Engine Development Team 
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

namespace LucreApp
{

    NightScene::NightScene(const std::string& filepath, const std::string& alternativeFilepath)
            : Scene(filepath, alternativeFilepath), m_SceneLoader{*this}
    {
    }

    void NightScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        ImGUI::m_AmbientLightIntensity = 0.177;
        m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);

        {  // set up camera
            m_CameraController = std::make_shared<CameraController>();
            m_CameraController->SetTranslationSpeed(400.0f);
            m_CameraController->SetRotationSpeed(0.5f);

            m_Camera = CreateEntity();
            TransformComponent cameraTransform{};
            m_Registry.emplace<TransformComponent>(m_Camera, cameraTransform);
            ResetScene();

            KeyboardInputControllerSpec keyboardInputControllerSpec{};
            m_KeyboardInputController = std::make_shared<KeyboardInputController>(keyboardInputControllerSpec);
        }

        StartScripts();
        TreeNode::TraverseInfo(m_SceneHierarchy);
        m_Dictionary.List();

        // get characters and start all animations
        m_NonPlayableCharacter1 = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/monkey01/monkey01.gltf::Scene::1");
        m_Hero = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/CesiumMan/animations/CesiumManAnimations.gltf::Scene::Cesium_Man");
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
        m_Guybrush = m_Dictionary.Retrieve("application/lucre/models/guybrush_animated_gltf/animation/guybrush_animation.gltf::Scene::guybrush object");
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

                entt::entity model = m_Dictionary.Retrieve("application/lucre/models/guybrush_animated_gltf/animation/guybrush_animation.gltf::Scene::root");

                m_CharacterAnimation = std::make_unique<CharacterAnimation>(m_Registry, model, animations);
                m_CharacterAnimation->Start();
            }
        }
        else
        {
            if (m_Registry.all_of<SkeletalAnimationTag>(m_Hero))
            {
                auto& mesh = m_Registry.get<MeshComponent>(m_Hero);
                SkeletalAnimations& animations = mesh.m_Model->GetAnimations();

                entt::entity model = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/CesiumMan/animations/CesiumManAnimations.gltf::Scene::root");

                m_CharacterAnimation = std::make_unique<CharacterAnimation>(m_Registry, model, animations);
                m_CharacterAnimation->Start();
            }
        }

        m_NonPlayableCharacter2 = m_Dictionary.Retrieve("application/lucre/models/Kaya/gltf/Kaya.gltf::Scene::Kaya BrowsAnimGeo");
        if (m_NonPlayableCharacter2 != entt::null)
        {
            auto& mesh = m_Registry.get<MeshComponent>(m_NonPlayableCharacter2);
            SkeletalAnimations& animations = mesh.m_Model->GetAnimations();
            animations.SetRepeatAll(true);
            animations.Start();
        }

        {
            // place static lights for beach scene
            float intensity = 5.0f;
            float lightRadius = 0.1f;
            float height1 = 0.4f;
            std::vector<glm::vec3> lightPositions =
            {
                {-0.285, height1, -2.8},
                {-3.2,   height1, -2.8},
                {-6.1,   height1, -2.8},
                { 2.7,   height1, -2.8},
                { 5.6,   height1, -2.8},
                {-0.285, height1,  0.7},
                {-3.2,   height1,  0.7},
                {-6.1,   height1,  0.7},
                { 2.7,   height1,  0.7},
                { 5.6,   height1,  0.7}
            };

            for (size_t i = 0; i < lightPositions.size(); i++)
            {
                auto entity = CreatePointLight(intensity, lightRadius);
                TransformComponent transform{};
                transform.SetTranslation(lightPositions[i]);
                m_Registry.emplace<TransformComponent>(entity, transform);
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

    void NightScene::Load()
    {
        m_SceneLoader.Deserialize();
        ImGUI::SetupSlider(m_SceneLoader.GetGltfFiles());

        LoadModels();
        LoadScripts();
    }

    void NightScene::LoadModels()
    {
        {
            std::vector<std::string> faces =
            {
                "application/lucre/models/external_3D_files/night/right.png",
                "application/lucre/models/external_3D_files/night/left.png",
                "application/lucre/models/external_3D_files/night/top.png",
                "application/lucre/models/external_3D_files/night/bottom.png",
                "application/lucre/models/external_3D_files/night/front.png",
                "application/lucre/models/external_3D_files/night/back.png"
            };

            Builder builder;
            m_Skybox = builder.LoadCubemap(faces, m_Registry);
            auto view = m_Registry.view<TransformComponent>();
            auto& skyboxTransform  = view.get<TransformComponent>(m_Skybox);
            skyboxTransform.SetScale(20.0f);
        }
        {
            {
                m_Lightbulb0 = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/lightBulb/lightBulb.gltf::Scene::lightbulb");
                m_LightView0 = std::make_shared<Camera>();
                float left   =  -4.0f;
                float right  =   4.0f;
                float bottom =  -4.0f;
                float top    =   4.0f;
                float near   =   0.1f;
                float far    =  10.0f;
                m_LightView0->SetOrthographicProjection3D(left, right, bottom, top, near, far);
                SetLightView(m_Lightbulb0, m_LightView0);
            }

            {
                m_Lightbulb1 = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/lightBulb/lightBulb2.gltf::Scene::arrow");
                m_LightView1 = std::make_shared<Camera>();
                float left   = -20.0f;
                float right  =  20.0f;
                float bottom = -14.0f;
                float top    =  14.0f;
                float near   =   0.1f;
                float far    =  40.0f;
                m_LightView1->SetOrthographicProjection3D(left, right, bottom, top, near, far);
                SetLightView(m_Lightbulb1, m_LightView1);
            }
        }
    }

    void NightScene::LoadScripts()
    {
    }

    void NightScene::StartScripts()
    {
        auto duck = m_Dictionary.Retrieve("application/lucre/models/duck/duck.gltf::SceneWithDuck::duck");
        if (duck != entt::null)
        {
            auto& duckScriptComponent = m_Registry.get<ScriptComponent>(duck);

            duckScriptComponent.m_Script = std::make_shared<DuckScript>(duck, this);
            LOG_APP_INFO("scripts loaded");
        }
    }

    void NightScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoader.Serialize();
    }

    void NightScene::OnUpdate(const Timestep& timestep)
    {
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform  = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetViewYXZ(cameraTransform.GetTranslation(), cameraTransform.GetRotation());
        }

        AnimateHero(timestep);
        m_CharacterAnimation->OnUpdate(timestep);
        SetLightView(m_Lightbulb0, m_LightView0);
        SetLightView(m_Lightbulb1, m_LightView1);
        SetDirectionalLight(m_DirectionalLight0, m_Lightbulb0, m_LightView0, 0 /*shadow renderpass*/);
        SetDirectionalLight(m_DirectionalLight1, m_Lightbulb1, m_LightView1, 1 /*shadow renderpass*/);

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera());
        m_Renderer->UpdateAnimations(m_Registry, timestep);
        m_Renderer->ShowDebugShadowMap(ImGUI::m_ShowDebugShadowMap);
        m_Renderer->SubmitShadows(m_Registry, m_DirectionalLights);
        m_Renderer->Renderpass3D(m_Registry);

        RotateLights(timestep);
        ApplyDebugSettings();

        // opaque objects
        m_Renderer->Submit(m_Registry, m_SceneHierarchy);

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

    void NightScene::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent l_Event)
            {
                auto zoomFactor = m_CameraController->GetZoomFactor();
                zoomFactor -= l_Event.GetY()*0.1f;
                m_CameraController->SetZoomFactor(zoomFactor);
                return true;
            }
        );
    }

    void NightScene::OnResize()
    {
        m_CameraController->SetProjection();
    }

    void NightScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& cameraTransform = m_Registry.get<TransformComponent>(m_Camera);

        cameraTransform.SetTranslation({-0.8f, 2.0f, 2.30515f});
        cameraTransform.SetRotation({0.0610371f, 6.2623f, 0.0f});

        m_CameraController->SetViewYXZ(cameraTransform.GetTranslation(), cameraTransform.GetRotation());
    }

    void NightScene::RotateLights(const Timestep& timestep)
    {
        float time = 0.3f * timestep;
        auto rotateLight = glm::rotate(glm::mat4(1.f), time, {0.f, -1.f, 0.f});

        auto view = m_Registry.view<PointLightComponent, TransformComponent, Group1>();
        for (auto entity : view)
        {
            auto& transform  = view.get<TransformComponent>(entity);
            transform.SetTranslation(glm::vec3(rotateLight * glm::vec4(transform.GetTranslation(), 1.f)));
        }
    }

    void NightScene::AnimateHero(const Timestep& timestep)
    {
        auto view = m_Registry.view<TransformComponent>();
        auto& heroTransform  = view.get<TransformComponent>(m_NonPlayableCharacter1);

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
            auto& lightbulbTransform  = m_Registry.get<TransformComponent>(lightbulb);

            glm::vec3 position  = lightbulbTransform.GetTranslation();
            glm::vec3 rotation  = lightbulbTransform.GetRotation();
            lightView->SetViewYXZ(position, rotation);
        }
    }

    void NightScene::SetDirectionalLight
    (
        const entt::entity directionalLight,
        const entt::entity lightbulb,
        const std::shared_ptr<Camera>& lightView,
        int renderpass
    )
    {
        auto& lightbulbTransform         = m_Registry.get<TransformComponent>(lightbulb);
        auto& directionalLightComponent  = m_Registry.get<DirectionalLightComponent>(directionalLight);
        directionalLightComponent.m_Direction  = lightbulbTransform.GetRotation();
        directionalLightComponent.m_LightView  = lightView.get();
        directionalLightComponent.m_RenderPass = renderpass;
    }

    void NightScene::ApplyDebugSettings()
    {
        if (ImGUI::m_UseAmbientLightIntensity)
        {
            m_Renderer->SetAmbientLightIntensity(ImGUI::m_AmbientLightIntensity);
        }
    }
}
