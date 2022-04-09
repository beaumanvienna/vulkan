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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "core.h"
#include "auxiliary/random.h"
#include "scene/particleSystem.h"

namespace GfxRenderEngine
{
    ParticleSystem::ParticleSystem(uint poolSize, float zaxis, SpriteSheet* spritesheet, float amplification, int unlit)
        : m_ParticlePool{poolSize}, m_PoolIndex{0},
          m_Spritesheet{spritesheet}, m_Zaxis{zaxis}
    {
        ASSERT(poolSize);
        auto numberOfSprites = m_Spritesheet->GetNumberOfSprites();
        m_AnimationSprites.resize(numberOfSprites);
        for (uint i = 0; i < numberOfSprites; i++)
        {
            Builder builder{};

            auto sprite = m_Spritesheet->GetSprite(i);
            glm::mat4 position = sprite->GetScaleMatrix();
            builder.LoadSprite(sprite, position, amplification, unlit);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"particle animation", model};
            mesh.m_Enabled = false;

            m_AnimationSprites[i] = m_Registry.create();
            m_Registry.emplace<MeshComponent>(m_AnimationSprites[i], mesh);
        }
    }

    void ParticleSystem::Emit(const ParticleSystem::Specification& spec, const ParticleSystem::Specification& variation)
    {
        Particle& particle = m_ParticlePool[m_PoolIndex];

        particle.m_Velocity          = spec.m_Velocity + variation.m_Velocity.x * EngineCore::RandomPlusMinusOne();
        particle.m_Acceleration      = spec.m_Acceleration;

        particle.m_RotationSpeed     = spec.m_RotationSpeed;

        particle.m_StartColor        = spec.m_StartColor;

        particle.m_StartSize         = spec.m_StartSize;
        particle.m_FinalSize         = spec.m_FinalSize;

        particle.m_LifeTime          = spec.m_LifeTime;
        particle.m_RemainingLifeTime = particle.m_LifeTime;

        particle.m_Enabled = true;

        m_PoolIndex = (m_PoolIndex + 1) % m_ParticlePool.size();

        Builder builder{};
        particle.m_Entity = m_Registry.create();

        builder.LoadParticle(spec.m_StartColor);
        auto model = Engine::m_Engine->LoadModel(builder);
        MeshComponent mesh{"particle", model};
        m_Registry.emplace<MeshComponent>(particle.m_Entity, mesh);

        TransformComponent transform{};
        static uint cnt = 0;
        cnt = (cnt + 1) % 100;
        transform.SetTranslation(glm::vec3{spec.m_Position.x, spec.m_Position.y, m_Zaxis - 0.01f * cnt});
        transform.SetScale(glm::vec3{1.0f} * particle.m_StartSize);
        transform.SetRotation(glm::vec3{0.0f, 0.0f, spec.m_Rotation + variation.m_Rotation * EngineCore::RandomPlusMinusOne()});
        m_Registry.emplace<TransformComponent>(particle.m_Entity, transform);

        particle.m_SmokeAnimation.Create(100ms /* per frame */, m_Spritesheet);
        particle.m_SmokeAnimation.Start();
    }

    void ParticleSystem::OnUpdate(Timestep timestep)
    {
        for (auto& particle : m_ParticlePool)
        {
            if (!particle.m_Enabled)
            {
                continue;
            }

            if (particle.m_RemainingLifeTime <= 0.0ms)
            {
                particle.m_Enabled = false;
                continue;
            }

            auto& transform = m_Registry.get<TransformComponent>(particle.m_Entity);
            particle.m_Velocity += particle.m_Acceleration * static_cast<float>(timestep);
            transform.SetTranslationX(transform.GetTranslation().x + particle.m_Velocity.x * static_cast<float>(timestep));
            transform.SetTranslationY(transform.GetTranslation().y + particle.m_Velocity.y * static_cast<float>(timestep));

            transform.SetRotationZ(transform.GetRotation().z + particle.m_RotationSpeed * static_cast<float>(timestep));
            particle.m_RemainingLifeTime -= timestep;

            auto remainingLifeTime = static_cast<float>(particle.m_RemainingLifeTime);
            auto lifeTime = static_cast<float>(particle.m_LifeTime);
            auto normalizedRemainingLifeTime = remainingLifeTime / lifeTime;

            float size = glm::lerp(particle.m_FinalSize, particle.m_StartSize, normalizedRemainingLifeTime);
            transform.SetScaleX(size);
            transform.SetScaleY(size);

            {
                if (!particle.m_SmokeAnimation.IsRunning())
                {
                    particle.m_SmokeAnimation.Start();
                }

                if (particle.m_SmokeAnimation.IsNewFrame())
                {
                    uint currentFrame = particle.m_SmokeAnimation.GetCurrentFrame();
                    particle.m_SpriteEntity = m_AnimationSprites[currentFrame];
                }
            }
        }
    }
}
