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

#pragma once

#include "engine.h"
#include "scene/scene.h"
#include "scene/components.h"
#include "scene/particleSystem.h"
#include "scene/sceneLoaderJSON.h"
#include "renderer/cameraController.h"
#include "renderer/renderer.h"
#include "renderer/cubemap.h"

#include "lucre.h"
#include "gamepadInputController.h"
#include "keyboardInputController.h"
#include "characterAnimation.h"

#include "animation/easingAnimations.h"

namespace LucreApp
{
    class NightScene : public Scene
    {

    public:
        NightScene(const std::string& filepath, const std::string& alternativeFilepath);
        ~NightScene() override {}

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
        void AnimateVulcan(const Timestep& timestep);
        void SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView);
        void SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                 const std::shared_ptr<Camera>& lightView, int renderpass);
        void ApplyDebugSettings();
        void EmitVolcanoSmoke();

    private:
        Renderer* m_Renderer;
        SceneLoaderJSON m_SceneLoaderJSON;

        // the camera is keyboard-controlled
        std::shared_ptr<CameraController> m_CameraController;
        std::shared_ptr<KeyboardInputController> m_KeyboardInputController;
        std::shared_ptr<GamepadInputController> m_GamepadInputController;
        std::shared_ptr<Camera> m_LightView0, m_LightView1;

        // game objects
        entt::entity m_Camera, m_Skybox, m_NonPlayableCharacter1, m_NonPlayableCharacter2, m_NonPlayableCharacter3;
        entt::entity m_Hero, m_Lightbulb0, m_Lightbulb1, m_Guybrush;
        entt::entity m_DirectionalLight0, m_DirectionalLight1;
        entt::entity m_PointLight[MAX_LIGHTS];
        std::vector<DirectionalLightComponent*> m_DirectionalLights;

        // volcano
        Timer m_LaunchVolcanoTimer;
        std::shared_ptr<ParticleSystem> m_VolcanoSmoke;
        SpriteSheet m_SpritesheetSmoke;

        // animation
        std::unique_ptr<CharacterAnimation> m_CharacterAnimation;
        static constexpr int NUMBER_OF_MOVING_CAMERAS = 1;
        enum CameraAnimation
        {
            X = 0,
            Z,
            CAMROT,
            ANIMATE_X_Z_CAMROT
        };
        static constexpr bool INVERT_EASE = true;
        std::array<EasingAnimations<ANIMATE_X_Z_CAMROT>, NUMBER_OF_MOVING_CAMERAS> m_EasingAnimation;
        void AssignAnimation(EasingAnimations<ANIMATE_X_Z_CAMROT>& easingAnimation);
        bool m_RunCameraAnimation;

    private:
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
