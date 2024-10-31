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

#include "core.h"
#include "auxiliary/random.h"
#include "scene/particleSystem.h"
#include "scene/components.h"

namespace GfxRenderEngine
{
    ParticleSystem::ParticleSystem(uint poolSize, SpriteSheet* spritesheet, float amplification)
        : m_ParticlePool{poolSize}, m_PoolIndex{0}, m_Spritesheet{spritesheet}
    {
        CORE_ASSERT(poolSize, "pool size is zero");
        auto numberOfSprites = m_Spritesheet->GetNumberOfSprites();
        m_AnimationSprites.resize(numberOfSprites);
        for (uint i = 0; i < numberOfSprites; i++)
        {
            Builder builder{};

            auto sprite = m_Spritesheet->GetSprite(i);
            builder.LoadSprite(sprite, amplification);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"particle animation", model};
            mesh.m_Enabled = false;

            m_AnimationSprites[i] = m_Registry.Create();
            m_Registry.emplace<MeshComponent>(m_AnimationSprites[i], mesh);
        }
    }

    void ParticleSystem::Emit(const ParticleSystem::Specification& spec, const ParticleSystem::Specification& variation)
    {
        Particle& particle = m_ParticlePool[m_PoolIndex];

        glm::vec3 variationVelocity = glm::vec3{
            variation.m_Velocity.x * EngineCore::RandomPlusMinusOne(),
            variation.m_Velocity.y * EngineCore::RandomPlusMinusOne(),
            variation.m_Velocity.z * EngineCore::RandomPlusMinusOne(),
        };
        particle.m_Velocity = spec.m_Velocity + variationVelocity;
        particle.m_Acceleration = spec.m_Acceleration;

        particle.m_RotationSpeed = spec.m_RotationSpeed;

        particle.m_StartColor = spec.m_StartColor;
        particle.m_EndColor = spec.m_EndColor;

        particle.m_StartSize = spec.m_StartSize;
        particle.m_FinalSize = spec.m_FinalSize;

        particle.m_LifeTime = spec.m_LifeTime;
        particle.m_RemainingLifeTime = particle.m_LifeTime;

        particle.m_Enabled = true;

        m_PoolIndex = (m_PoolIndex + 1) % m_ParticlePool.size();

        Builder builder{};
        particle.m_Entity = m_Registry.Create();

        builder.LoadParticle(spec.m_StartColor);
        auto model = Engine::m_Engine->LoadModel(builder);
        MeshComponent mesh{"particle", model};
        m_Registry.emplace<MeshComponent>(particle.m_Entity, mesh);

        TransformComponent transform{};
        transform.SetTranslation(glm::vec3{
            spec.m_Position.x + variation.m_Position.x * EngineCore::RandomPlusMinusOne(),
            spec.m_Position.y + variation.m_Position.y * EngineCore::RandomPlusMinusOne(),
            spec.m_Position.z + variation.m_Position.z * EngineCore::RandomPlusMinusOne(),
        });
        transform.SetScale(glm::vec3{1.0f} * particle.m_StartSize);
        transform.SetRotation(glm::vec3{spec.m_Rotation.x, spec.m_Rotation.y,
                                        spec.m_Rotation.z + variation.m_Rotation.z * EngineCore::RandomPlusMinusOne()});
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
            transform.SetTranslationZ(transform.GetTranslation().z + particle.m_Velocity.z * static_cast<float>(timestep));

            transform.SetRotationX(transform.GetRotation().x + particle.m_RotationSpeed.x * static_cast<float>(timestep));
            transform.SetRotationY(transform.GetRotation().y + particle.m_RotationSpeed.y * static_cast<float>(timestep));
            transform.SetRotationZ(transform.GetRotation().z + particle.m_RotationSpeed.z * static_cast<float>(timestep));
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
} // namespace GfxRenderEngine
