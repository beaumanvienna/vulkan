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
#include "sprite/sprite.h"

namespace LucreApp
{
    class SettingsScene : public Scene
    {

    public:
        SettingsScene(const std::string& filepath, const std::string& alternativeFilepath)
            : Scene(filepath, alternativeFilepath)
        {
        }
        ~SettingsScene() override {}

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

    private:
        void Init();

    private:
        static constexpr int NUM_BARRELS = 4;
        Renderer* m_Renderer;
        std::shared_ptr<CameraController> m_CameraController;

        entt::entity m_BackGround, m_Barrel[NUM_BARRELS];
        Sprite2D m_BarrelSprite;
        float m_BarrelTranslationSpeed[NUM_BARRELS], m_BarrelRotationSpeed[NUM_BARRELS];
    };
} // namespace LucreApp
