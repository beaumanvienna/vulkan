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

#include <string>
#include <memory>
#include <bitset>

#include "engine.h"
#include "scene/sceneGraph.h"
#include "scene/particleSystem.h"
#include "renderer/camera.h"
#include "renderer/resourceDescriptor.h"

namespace GfxRenderEngine
{

    class Renderer
    {
    public:
        enum WaterPasses
        {
            REFRACTION = 0,
            REFLECTION,
            NUMBER_OF_WATER_PASSES
        };

    public:
        virtual ~Renderer() = default;

        virtual bool Init() = 0;

        virtual void SubmitShadows(Registry& registry,
                                   const std::vector<DirectionalLightComponent*>& directionalLights = {}) = 0;
        virtual void Submit(Scene& scene) = 0;
        virtual void SubmitWater(Scene& scene, bool reflection) = 0;
        virtual void NextSubpass() = 0;
        virtual void LightingPass() = 0;
        virtual void LightingPassIBL(float uMaxPrefilterMip,
                                     std::shared_ptr<ResourceDescriptor> const& resourceDescriptorIBL) = 0;
        virtual void LightingPassWater(bool reflection) = 0;
        virtual void PostProcessingRenderpass() = 0;
        virtual void TransparencyPass(Registry& registry, ParticleSystem* particleSystem = nullptr) = 0;
        virtual void TransparencyPassWater(Registry& registry, bool reflection) = 0;
        virtual void Submit2D(Camera* camera, Registry& registry) = 0;
        virtual void GUIRenderpass(Camera* camera) = 0;
        virtual uint GetFrameCounter() = 0;

        virtual void BeginFrame(Camera* camera) = 0;
        virtual void RenderpassWater(Registry& registry, Camera& camera, bool reflection,
                                     glm::vec4 const& clippingPlane) = 0;
        virtual void EndRenderpassWater() = 0;
        virtual void Renderpass3D(Registry& registry) = 0;
        virtual void EndScene() = 0;

        virtual void DrawWithTransform(const Sprite& sprite, const glm::mat4& transform) = 0;
        virtual void Draw(const Sprite& sprite, const glm::mat4& position, const glm::vec4& color,
                          const float textureID = 1.0f) = 0;

        virtual void SetAmbientLightIntensity(float ambientLightIntensity) = 0;
        virtual float GetAmbientLightIntensity() = 0;

        virtual void ShowDebugShadowMap(bool showDebugShadowMap) = 0;
        virtual void UpdateTransformCache(Scene& scene, uint const nodeIndex, glm::mat4 const& parentMat4,
                                          bool parentDirtyFlag) = 0;
        virtual void UpdateAnimations(Registry& registry, const Timestep& timestep) = 0;
        virtual std::shared_ptr<Texture> GetTextureAtlas() = 0;

        virtual float& Exposure() = 0;
        virtual std::bitset<32>& ShaderSettings0() = 0;
    };
} // namespace GfxRenderEngine
