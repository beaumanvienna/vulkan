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

#pragma once

#include "engine.h"
#include "scene/scene.h"
#include "scene/components.h"
#include "scene/sceneLoaderJSON.h"
#include "renderer/cameraController.h"
#include "renderer/renderer.h"
#include "renderer/cubemap.h"

#include "lucre.h"
#include "gamepadInputController.h"
#include "keyboardInputController.h"

namespace LucreApp
{
    class BeachScene : public Scene
    {

    public:
        BeachScene(const std::string& filepath, const std::string& alternativeFilepath);
        ~BeachScene() override {}

        virtual void Start() override;
        virtual void Stop() override;
        virtual void OnUpdate(const Timestep& timestep) override;
        virtual Camera& GetCamera() override { return m_CameraController->GetCamera(); }
        virtual void OnEvent(Event& event) override;
        virtual void OnResize() override;

        virtual void Load() override;
        virtual void Save() override {}
        virtual void LoadScripts() override;
        virtual void StartScripts() override;

    private:
        void LoadModels();
        void ResetScene();
        void RotateLights(const Timestep& timestep);
        void AnimateHero(const Timestep& timestep);
        void SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView);
        void SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                 const std::shared_ptr<Camera>& lightView, int renderpass);
        void ApplyDebugSettings();

    private:
        Renderer* m_Renderer;
        SceneLoaderJSON m_SceneLoaderJSON;

        // the camera is keyboard-controlled
        std::shared_ptr<CameraController> m_CameraController;
        std::shared_ptr<KeyboardInputController> m_KeyboardInputController;
        std::shared_ptr<Camera> m_LightView0, m_LightView1;

        // game objects
        entt::entity m_Camera, m_Skybox, m_NonPlayableCharacter, m_Lightbulb0, m_Lightbulb1;
        std::vector<DirectionalLightComponent*> m_DirectionalLights;
        entt::entity m_DirectionalLight0, m_DirectionalLight1;

        // some game objects can be controlled with a gamepad
        std::unique_ptr<GamepadInputController> m_GamepadInputController;

    private:
        struct BananaComponent
        {
            bool m_IsOnTheGround;
        };
        struct Group1
        {
            bool m_Rotated;
        };
        struct Group2
        {
            bool m_Rotated;
        };
    };
} // namespace LucreApp
