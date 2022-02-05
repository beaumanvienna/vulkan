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

#include "scene/particleSystem.h"

namespace GfxRenderEngine
{
    ParticleSystem::ParticleSystem(uint poolSize)
        : m_ParticlePool{poolSize}, m_PoolIndex{0}
    {
        ASSERT(poolSize);
    }

    void ParticleSystem::Emit(const ParticleSystem::Specification& spec, const ParticleSystem::Specification& variation)
    {
        Particle& particle = m_ParticlePool[m_PoolIndex];

        particle.m_Position          = spec.m_Position;
        particle.m_Velocity          = spec.m_Velocity;
        particle.m_Acceleration      = spec.m_Acceleration;

        particle.m_Rotation          = spec.m_Rotation;
        particle.m_RotationSpeed     = spec.m_RotationSpeed;

        particle.m_StartColor        = spec.m_StartColor;
        particle.m_FinalColor        = spec.m_FinalColor;

        particle.m_StartSize         = spec.m_StartSize;
        particle.m_FinalSize         = spec.m_FinalSize;

        particle.m_LifeTime          = spec.m_LifeTime;
        particle.m_RemainingLifeTime = particle.m_LifeTime;

        particle.m_Enabled = true;

        m_PoolIndex = (m_PoolIndex + 1) % m_ParticlePool.size();
    }

    void ParticleSystem::OnUpdate(Timestep timestep, Renderer& renderer)
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

            particle.m_Velocity += particle.m_Acceleration * static_cast<float>(timestep);
            particle.m_Position += particle.m_Velocity * static_cast<float>(timestep);
            particle.m_Rotation += particle.m_RotationSpeed * static_cast<float>(timestep);

            particle.m_RemainingLifeTime -= timestep;
            
            auto remainingLifeTime = static_cast<float>(particle.m_RemainingLifeTime);
            auto lifeTime = static_cast<float>(particle.m_LifeTime);
            auto normalizedRemainingLifeTime = remainingLifeTime / lifeTime;

            glm::vec4 color = glm::lerp(particle.m_FinalColor, particle.m_StartColor, normalizedRemainingLifeTime);
            float size = glm::lerp(particle.m_FinalSize, particle.m_StartSize, normalizedRemainingLifeTime);

            glm::mat4 transform = glm::translate(glm::mat4{1.0f}, { particle.m_Position.x, particle.m_Position.y, 0.0f })
                * glm::scale(glm::mat4{1.0f}, {size, size, 1.0f});

            //Sprite* sprite = m_Sprites[spriteIndex];
            //renderer->Draw(sprite, transform);
        }
    }
}
