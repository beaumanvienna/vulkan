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
#include "scene/particleSystem.h"
#include "sprite/spriteAnimation.h"
#include "renderer/renderer.h"
#include "renderer/cubemap.h"
#include "renderer/texture.h"

#include "renderer/cameraController.h"
#include "platform/SDL/timer.h"

#include "lucre.h"
#include "gamepadInputController.h"
#include "keyboardInputController.h"

#include "box2d/box2d.h"

namespace LucreApp
{
    class MainScene : public Scene
    {

    public:
        MainScene(const std::string& filepath, const std::string& alternativeFilepath);
        ~MainScene() override {}

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
        void InitPhysics();
        void FireVolcano();
        void ResetBananas();
        void RotateLights(const Timestep& timestep);
        void UpdateBananas(const Timestep& timestep);
        void SimulatePhysics(const Timestep& timestep);
        void ApplyDebugSettings();

    private:
        static constexpr float POINT_LIGHT_INTENSITY = 2.7f;

        Renderer* m_Renderer;
        SceneLoaderJSON m_SceneLoaderJSON;

        // the camera is keyboard-controlled
        std::shared_ptr<CameraController> m_CameraController;
        std::shared_ptr<KeyboardInputController> m_KeyboardInputController;

        // game objects
        entt::entity m_Camera, m_Barrel, m_Sponza;
        entt::entity m_PointLight[MAX_LIGHTS], m_Duck, m_BarramundiFish;
        entt::entity m_GoldenDuck, m_Helmet, m_ToyCar, m_Skybox;

        static constexpr uint MAX_B = 24;
        entt::entity m_Banana[MAX_B];

        // some game objects can be controlled with a gamepad
        std::unique_ptr<GamepadInputController> m_GamepadInputController;

        const b2Vec2 GRAVITY{0.0f, -9.81f};
        std::unique_ptr<b2World> m_World;
        b2Body* m_GroundBody;
        bool m_Fire, m_StartTimer;
        Timer m_LaunchVolcanoTimer;

        static constexpr uint HORN_ANIMATION_SPRITES = 25;
        entt::entity m_Guybrush[HORN_ANIMATION_SPRITES];
        SpriteSheet m_SpritesheetHorn;
        SpriteAnimation m_HornAnimation;

        SpriteSheet m_SpritesheetSmoke;

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
