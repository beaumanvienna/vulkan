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

#include <string>
#include <memory>

#include "engine.h"
#include "scene/entity.h"
#include "scene/treeNode.h"
#include "scene/particleSystem.h"
#include "renderer/camera.h"

namespace GfxRenderEngine
{
    class Renderer
    {
    public:

        Renderer();
        virtual ~Renderer() {}

        virtual void Submit(entt::registry& registry, TreeNode& sceneHierarchy) = 0;
        virtual void NextSubpass() = 0;
        virtual void LightingPass() = 0;
        virtual void TransparencyPass(entt::registry& registry, std::shared_ptr<ParticleSystem>& particleSystem) = 0;
        virtual void Submit2D(entt::registry& registry) = 0;
        virtual void GUIRenderpass(Camera* camera) = 0;
        virtual uint GetFrameCounter() = 0;

        virtual void BeginFrame(Camera* camera, entt::registry& registry) = 0;
        virtual void EndScene() = 0;

        virtual void Draw(Sprite* sprite, const glm::mat4& position, const float depth = 0.0f, const glm::vec4& color = glm::vec4(1.0f)) = 0;
        virtual void Draw(std::shared_ptr<Texture> texture, const glm::mat4& position, const glm::vec4 textureCoordinates, const float depth, const glm::vec4& color = glm::vec4(1.0f)) = 0;

        virtual void SetAmbientLightIntensity(float ambientLightIntensity) = 0;
        virtual float GetAmbientLightIntensity() = 0;

    };
}
