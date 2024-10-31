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

#include "lucre.h"
#include "engine.h"
#include "scene/scene.h"
#include "renderer/cameraController.h"

namespace LucreApp
{
    class CutScene : public Scene
    {

    public:
        CutScene(const std::string& filepath, const std::string& alternativeFilepath) : Scene(filepath, alternativeFilepath)
        {
        }
        ~CutScene() override {}

        virtual void Start() override;
        virtual void Stop() override;
        virtual void OnUpdate(const Timestep& timestep) override;
        virtual Camera& GetCamera() override { return m_CameraController->GetCamera(); }
        virtual void OnEvent(Event& event) override;
        virtual void OnResize() override;

        virtual void Load() override {}
        virtual void Save() override {}
        virtual void LoadScripts() override {}
        virtual void StartScripts() override {}
        virtual void ResetTimer() override;

    private:
        static constexpr std::chrono::duration<float, std::chrono::seconds::period> MIN_TIME_IN_CUTSCENE = 0.7ms;
        void Init();
        void MoveClouds(const Timestep& timestep);
        void Draw();

    private:
        Renderer* m_Renderer;
        std::shared_ptr<CameraController> m_CameraController;

        static constexpr uint WALK_ANIMATION_SPRITES = 6;
        entt::entity m_Guybrush[WALK_ANIMATION_SPRITES], m_Beach, m_Clouds[2];
        Sprite2D m_CloudSprite, m_BeachSprite;
        SpriteSheet m_SpritesheetWalk;
        SpriteAnimation m_WalkAnimation;
        float m_GuybrushWalkDelta;
        float m_InitialPositionX, m_EndPositionX;
        float m_Scale;
        float m_TranslationX0;
        float m_TranslationX1;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    };
} // namespace LucreApp
