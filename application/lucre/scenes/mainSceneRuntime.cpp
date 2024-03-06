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

#include <stdlib.h>
#include <time.h>

#include "core.h"
#include "events/event.h"
#include "events/mouseEvent.h"
#include "resources/resources.h"

#include "mainScene.h"

namespace LucreApp
{
    void MainScene::RotateLights(const Timestep& timestep)
    {
        float time = 0.3f * timestep;
        auto rotateLight = glm::rotate(glm::mat4(1.f), time, {0.f, -1.f, 0.f});

        auto view = m_Registry.view<PointLightComponent, TransformComponent, Group1>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            transform.SetTranslation(glm::vec3(rotateLight * glm::vec4(transform.GetTranslation(), 1.f)));
        }
    }

    void MainScene::UpdateBananas(const Timestep& timestep)
    {
        auto view = m_Registry.view<BananaComponent, TransformComponent, RigidbodyComponent>();

        static constexpr float ROTATIONAL_SPEED = 3.0f;
        auto rotationDelta = ROTATIONAL_SPEED * timestep;
        for (auto banana : view)
        {
            auto& transform = view.get<TransformComponent>(banana);
            auto& rigidbody = view.get<RigidbodyComponent>(banana);
            auto body = static_cast<b2Body*>(rigidbody.m_Body);
            b2Vec2 position = body->GetPosition();
            transform.SetTranslationX(position.x);
            transform.SetTranslationY(position.y);
            transform.SetRotationY(transform.GetRotation().y + rotationDelta);
        }

        static uint index = 0;
        if (m_Fire) // from volcano
        {
            static auto start = Engine::m_Engine->GetTime();
            if ((Engine::m_Engine->GetTime() - start) > 100ms)
            {
                if (index < MAX_B)
                {
                    // random values in [-1.0f, 1.0f]
                    float rVal = 2 * (static_cast<float>(rand()) / RAND_MAX) - 1.0f;
                    // get new start time
                    start = Engine::m_Engine->GetTime();

                    // move to backgound on z-axis
                    auto& transform = m_Registry.get<TransformComponent>(m_Banana[index]);
                    transform.SetTranslationZ(5.0f);

                    auto& rigidbody = m_Registry.get<RigidbodyComponent>(m_Banana[index]);
                    auto body = static_cast<b2Body*>(rigidbody.m_Body);
                    body->SetLinearVelocity(b2Vec2(0.1f + rVal * 4, 5.0f));
                    body->SetTransform(b2Vec2(0.0f, 3.2f), 0.0f);

                    index++;
                }
                else if ((Engine::m_Engine->GetTime() - start) > 1500ms)
                {
                    ResetBananas();
                    m_Fire = false;
                }
            }
        }
        else
        {
            index = 0;
        }
    }

    void MainScene::ResetBananas()
    {
        m_GroundBody->SetTransform(b2Vec2(0.0f, 0.0f), 0.0f);
        auto view = m_Registry.view<BananaComponent, TransformComponent, RigidbodyComponent>();

        uint i = 0;
        for (auto banana : view)
        {
            auto& transform = view.get<TransformComponent>(banana);
            auto& rigidbody = view.get<RigidbodyComponent>(banana);
            auto body = static_cast<b2Body*>(rigidbody.m_Body);
            body->SetLinearVelocity(b2Vec2(0.0f, 0.01f));
            body->SetAngularVelocity(0.0f);
            if (i < 12)
            {
                body->SetTransform(b2Vec2(-3.0f + 0.5 * i, 2.0f + i * 1.0f), 0.0f);
                transform.SetTranslationZ(-0.6f);
            }
            else
            {
                body->SetTransform(b2Vec2(-3.0f + 0.5 * (i - 12), 2.0f + i * 1.0f), 0.0f);
                transform.SetTranslationZ(0.3f);
            }
            i++;
        }
    }

    void MainScene::SimulatePhysics(const Timestep& timestep)
    {
        float step = timestep;

        int velocityIterations = 6;
        int positionIterations = 2;
        m_World->Step(step, velocityIterations, positionIterations);
    }
} // namespace LucreApp
