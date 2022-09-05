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

#include "beachScene.h"
#include "application/lucre/UI/imgui.h"
#include "application/lucre/scripts/duck/duckScript.h"

namespace LucreApp
{

    BeachScene::BeachScene(const std::string& filepath, const std::string& alternativeFilepath)
            : Scene(filepath, alternativeFilepath), m_GamepadInput{}, m_SceneLoader{*this}
    {
    }

    void BeachScene::Start()
    {
        m_IsRunning = true;

        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Renderer->SetAmbientLightIntensity(0.06f);

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

        StartScripts();
        TreeNode::Traverse(m_SceneHierarchy);
        m_Dictionary.List();
        m_Dune = m_Dictionary.Retrieve("application/lucre/models/external_3D_files/dune/dune.gltf::Scene::dune");

        {
            // place static lights for beach scene
            float intensity = 5.0f;
            float lightRadius = 0.1f;
            float height1 = 0.2f;
            std::vector<glm::vec3> lightPositions =
            {
                {-0.285, height1, -1.542},
                {-3.2,   height1, -1.5420},
                {-6.1,   height1, -1.5420},
                { 2.7,   height1, -1.5420},
                { 5.6,   height1, -1.5420},
                {-0.285, height1, 1.2},
                {-3.2,   height1, 1.2},
                {-6.1,   height1, 1.2},
                { 2.7,   height1, 1.2},
                { 5.6,   height1, 1.2}
            };

            for (int i = 0; i < lightPositions.size(); i++)
            {
                auto entity = CreatePointLight(intensity, lightRadius);
                TransformComponent transform{};
                transform.SetTranslation(lightPositions[i]);
                m_Registry.emplace<TransformComponent>(entity, transform);
                m_Registry.emplace<Group2>(entity, true);
            }
        }
    }

    void BeachScene::Load()
    {
        ImGUI::m_MaxGameObjects = (entt::entity)0;
        m_SceneLoader.Deserialize(ImGUI::m_MaxGameObjects);

        LoadModels();
        LoadScripts();
    }

    void BeachScene::LoadModels()
    {
    }

    void BeachScene::LoadScripts()
    {
    }

    void BeachScene::StartScripts()
    {        
        auto duck = m_Dictionary.Retrieve("application/lucre/models/duck/duck.gltf::SceneWithDuck::duck");
        if (duck != entt::null)
        {
            auto& duckScriptComponent = m_Registry.get<ScriptComponent>(duck);

            duckScriptComponent.m_Script = std::make_shared<DuckScript>(duck, this);
            LOG_APP_INFO("scripts loaded");
        }
    }

    void BeachScene::Stop()
    {
        m_IsRunning = false;
        m_SceneLoader.Serialize();
    }

    void BeachScene::OnUpdate(const Timestep& timestep)
    {
        if (Lucre::m_Application->KeyboardInputIsReleased())
        {
            auto view = m_Registry.view<TransformComponent>();
            auto& cameraTransform  = view.get<TransformComponent>(m_Camera);

            m_KeyboardInputController->MoveInPlaneXZ(timestep, cameraTransform);
            m_CameraController->SetViewYXZ(cameraTransform.GetTranslation(), cameraTransform.GetRotation());
        }

        // draw new scene
        m_Renderer->BeginFrame(&m_CameraController->GetCamera(), m_Registry);

        auto frameRotation = static_cast<const float>(timestep) * 0.6f;

        RotateLights(timestep);

        // opaque objects
        m_Renderer->Submit(m_Registry, m_SceneHierarchy);

        // light opaque objects
        m_Renderer->NextSubpass();
        m_Renderer->LightingPass();

        // transparent objects
        m_Renderer->NextSubpass();

        // scene must switch to gui renderpass
        m_Renderer->GUIRenderpass(&SCREEN_ScreenManager::m_CameraController->GetCamera());
    }

    void BeachScene::OnEvent(Event& event)
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
    }

    void BeachScene::OnResize()
    {
        m_CameraController->SetProjection();
    }

    void BeachScene::ResetScene()
    {
        m_CameraController->SetZoomFactor(1.0f);
        auto& transform = m_Registry.get<TransformComponent>(m_Camera);
        transform.SetTranslation({3.1, 1.08, -1.6});
        transform.SetRotation({-0.04, 1.9, 0});
    }

    void BeachScene::RotateLights(const Timestep& timestep)
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
}
