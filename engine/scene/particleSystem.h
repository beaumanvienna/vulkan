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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "engine.h"
#include "scene/scene.h"
#include "auxiliary/timestep.h"
#include "sprite/spriteAnimation.h"

namespace GfxRenderEngine
{
    class ParticleSystem
    {

    public:

        // particles wil be rendered as billboards:
        // two dimensions are sufficient

        struct Specification
        {
            glm::vec3 m_Position{0.0f};
            glm::vec3 m_Velocity{0.0f};
            glm::vec3 m_Acceleration{0.0f};

            glm::vec3 m_Rotation{0.0f};
            glm::vec3 m_RotationSpeed{0.0f};

            glm::vec4 m_StartColor{0.0f};
            glm::vec4 m_EndColor{0.0f};

            float m_StartSize{0.0f};
            float m_FinalSize{0.0f};

            Timestep m_LifeTime{0ms};
        };

    public:

        ParticleSystem(uint poolSize /* = f(emitter rate, lifetime)*/, SpriteSheet* spritesheet, float amplification);

        void Emit(const ParticleSystem::Specification& spec, const ParticleSystem::Specification& variation);
        void OnUpdate(Timestep timestep);

    public:

        struct Particle
        {
            glm::vec3 m_Velocity;
            glm::vec3 m_Acceleration;

            glm::vec3 m_RotationSpeed;

            glm::vec4 m_StartColor;
            glm::vec4 m_EndColor;

            float m_StartSize;
            float m_FinalSize;

            Timestep m_LifeTime{0ms};
            Timestep m_RemainingLifeTime{0ms};

            SpriteAnimation m_SmokeAnimation;

            bool m_Enabled;
            entt::entity m_Entity;
            entt::entity m_SpriteEntity;
        };

        std::vector<Particle> m_ParticlePool;
        Registry m_Registry;

    private:

        uint m_PoolIndex;

        std::vector<entt::entity> m_AnimationSprites;
        SpriteSheet* m_Spritesheet;

    };
}
