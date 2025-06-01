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

#pragma once

#include "box2d/box2d.h"

#include "engine.h"
#include "renderer/cameraController.h"
#include "renderer/cubemap.h"
#include "renderer/renderer.h"
#include "scene/components.h"
#include "scene/scene.h"
#include "scene/sceneLoader.h"
#include "scene/sceneLoaderJSON.h"
#include "particleSystem/candles.h"
#include "physics/physics.h"

#include "gamepadInputController.h"
#include "keyboardInputController.h"
#include "characterAnimation.h"
#include "lucre.h"

namespace LucreApp
{
    class Reserved0Scene : public Scene
    {

    public:
        Reserved0Scene(const std::string& filepath, const std::string& alternativeFilepath);
        virtual ~Reserved0Scene() override;

        virtual void Start() override;
        virtual void Stop() override;

        virtual void OnUpdate(const Timestep& timestep) override;
        virtual Camera& GetCamera() override;
        virtual void OnEvent(Event& event) override;
        virtual void OnResize() override;

        virtual void Load() override;
        virtual void Save() override {}
        virtual void LoadScripts() override;
        virtual void StartScripts() override;

    private:
        void LoadModels();
        void ResetScene();
        void InitPhysics();
        void FireVolcano();
        void ResetBananas();
        void UpdateBananas(const Timestep& timestep);
        void SimulatePhysics(const Timestep& timestep);
        void SetLightView(const entt::entity lightbulb, const std::shared_ptr<Camera>& lightView);
        void SetDirectionalLight(const entt::entity directionalLight, const entt::entity lightbulb,
                                 const std::shared_ptr<Camera>& lightView, int renderpass);
        void ApplyDebugSettings();

    private:
        enum CameraTypes
        {
            DefaultCamera = 0,
            AttachedToCar1,
            AttachedToCar2,
            ShadowMapHiRes,
            ShadowMapLowRes,
            MaxCameraTypes
        };

        class CameraControllers
        {
        public:
            CameraControllers() = default;
            std::shared_ptr<CameraController>& GetActiveCameraController();
            int GetActiveCameraIndex() { return m_ActiveCamera; };
            std::shared_ptr<CameraController>& SetActiveCameraController(int index);
            std::shared_ptr<CameraController>& SetActiveCameraController(CameraTypes cameraType);
            void SetProjectionAll();

            std::shared_ptr<CameraController>& operator[](int index);
            CameraControllers& operator++();

        private:
            int m_ActiveCamera = static_cast<uint>(CameraTypes::DefaultCamera);
            std::shared_ptr<CameraController> m_CameraController[CameraTypes::MaxCameraTypes];
        };

        Renderer* m_Renderer;
        SceneLoaderJSON m_SceneLoaderJSON;

        // all things camera
        CameraControllers m_CameraControllers;
        std::shared_ptr<Camera> m_LightView0, m_LightView1;
        Camera m_ReflectionCamera;
        std::shared_ptr<KeyboardInputController> m_KeyboardInputController;

        // game objects
        entt::entity m_Camera[CameraTypes::MaxCameraTypes];
        entt::entity m_Skybox, m_Lightbulb0, m_Lightbulb1;
        std::vector<DirectionalLightComponent*> m_DirectionalLights;
        entt::entity m_DirectionalLight0, m_DirectionalLight1;
        entt::entity m_Penguin, m_Terrain1, m_Mario, m_Car;
        std::array<entt::entity, 4> m_Wheels;

        //------
        void LoadTerrain();
        //------

        // some game objects can be controlled with a gamepad
        std::unique_ptr<GamepadInputController> m_GamepadInputController;

        // animation
        std::unique_ptr<CharacterAnimation> m_CharacterAnimation;
        Candles m_CandleParticleSystem;

        // physics box2D
        const b2Vec2 GRAVITY{0.0f, -9.81f};
        std::unique_ptr<b2World> m_World;
        b2Body* m_GroundBody{nullptr};
        bool m_Fire{false};
        bool m_StartTimer{true};
        Timer m_LaunchVolcanoTimer;
        static constexpr uint MAX_B = 24;
        entt::entity m_Banana[MAX_B];

        // physics Jolt
        std::unique_ptr<Physics> m_Physics;
        Physics::VehicleControl m_VehicleControl{};
        bool m_DrawDebugMesh{true};

    private:
        struct BananaComponent
        {
            bool m_IsOnTheGround;
        };
        struct Group2
        {
            bool m_Rotated;
        };
    };
} // namespace LucreApp
