/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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
#include "scene/entity.h"
#include "sprite/sprite.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"
#include "renderer/cameraController.h"

#include "lucre.h"
#include "gamepadInputController.h"
#include "keyboardInputController.h"

#include "box2d/box2d.h"

namespace LucreApp
{
    class MainScene : public Scene
    {

    public:

        MainScene();
        ~MainScene() override {}

        void Start() override;
        void Stop() override;
        void OnUpdate(const Timestep& timestep) override;
        void OnEvent(Event& event) override;
        void OnResize() override;

    private:

        void InitPhysics();
        void LoadModels();
        void RotateLights(const Timestep& timestep);
        void RotateBananas(const Timestep& timestep);
        void AnimateVulcan(const Timestep& timestep);
        void SimulatePhysics(const Timestep& timestep);

    private:

        std::shared_ptr<Renderer> m_Renderer;

        // the camera is keyboard-controlled
        std::shared_ptr<CameraController> m_CameraController;
        std::shared_ptr<KeyboardInputController> m_KeyboardInputController;

        // game objects
        entt::entity m_Camera, m_Ground, m_Vase0, m_Vase1, m_PointLightVulcano;
        entt::entity m_PointLight[MAX_LIGHTS], m_Vulcano[3], m_Walkway[3];

        static constexpr uint MAX_B = 24;
        entt::entity m_Banana[MAX_B];

        // some game objects can be controlled with a gamepad
        std::unique_ptr<GamepadInputController> m_GamepadInputController;
        TransformComponent m_GamepadInput;

        std::shared_ptr<Sprite> m_VulcanoSprite;
        std::shared_ptr<Sprite> m_WalkwaySprite;

        std::unique_ptr<b2World> m_World;

    private:

        const b2Vec2 GRAVITY{0.0f, -9.81f};

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
}
