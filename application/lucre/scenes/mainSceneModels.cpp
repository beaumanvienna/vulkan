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

#include <stdlib.h>
#include <time.h>

#include "core.h"
#include "events/event.h"
#include "events/mouseEvent.h"
#include "resources/resources.h"
#include "renderer/texture.h"

#include "mainScene.h"

namespace LucreApp
{
    void MainScene::LoadModels()
    {
        {
            Builder builder{};

            m_BarramundiFish = builder.LoadGLTF("application/lucre/models/BarramundiFish/BarramundiFish.gltf", m_Registry);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"metal plate", model};
            m_Registry.emplace<MeshComponent>(m_BarramundiFish, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{1.6f, -0.5f, 0.0f};
            transform.m_Rotation = glm::vec3{glm::pi<float>(), 0.0f, 0.0f};
            m_Registry.emplace<TransformComponent>(m_BarramundiFish, transform);

        }
        {
            Builder builder{};

            m_Duck = builder.LoadGLTF("application/lucre/models/duck/duck.gltf", m_Registry);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"duck", model};
            m_Registry.emplace<MeshComponent>(m_Duck, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{-1.6f, 0.5f, 0.0f};
            transform.m_Scale = glm::vec3{0.001f, 0.001f, 0.001f};
            transform.m_Rotation = glm::vec3{0.0f, glm::pi<float>(), glm::pi<float>()};
            m_Registry.emplace<TransformComponent>(m_Duck, transform);

        }

        {
            Builder builder{};

            auto sprite = Lucre::m_Spritesheet->GetSprite(I_BLOOD_ISLAND);
            glm::mat4 position = sprite->GetScaleMatrix();
            builder.LoadSprite(sprite, position, 20.0f/*amplification*/);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"vulcano", model};

            bool flip = false;
            for (uint i =0; i < 3; i++)
            {
                flip = !flip;
                m_Vulcano[i] = CreateEntity();
                m_Registry.emplace<MeshComponent>(m_Vulcano[i], mesh);

                TransformComponent transform{};
                transform.m_Translation = glm::vec3{-77.0f + 77.0f*i, 3.0f, 40.0f};
                transform.m_Scale = glm::vec3{0.17f, 0.14f, 0.17f} * 0.7f;
                if (flip) transform.m_Rotation = glm::vec3{0.0f, glm::pi<float>(), 0.0f};
                m_Registry.emplace<TransformComponent>(m_Vulcano[i], transform);

                DiffuseMapComponent diffuse{};
                m_Registry.emplace<DiffuseMapComponent>(m_Vulcano[i], diffuse);
            }
        }
        {
            Builder builder{};

            auto sprite = Lucre::m_Spritesheet->GetSprite(I_WALKWAY);
            glm::mat4 position = sprite->GetScaleMatrix();
            builder.LoadSprite(sprite, position, 1.0f/*amplification*/);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"walkway", model};

            for (uint i =0; i < 3; i++)
            {
                m_Walkway[i] = CreateEntity();
                m_Registry.emplace<MeshComponent>(m_Walkway[i], mesh);

                TransformComponent transform{};
                transform.m_Translation = glm::vec3{-11.0f + 11.0f*i, 0.67f, -0.1f};
                transform.m_Scale = glm::vec3{0.017f, 0.014f, 0.017f};
                transform.m_Rotation = glm::vec3{-glm::half_pi<float>(), 0.0f, 0.0f};
                m_Registry.emplace<TransformComponent>(m_Walkway[i], transform);

                DiffuseMapComponent diffuse{};
                m_Registry.emplace<DiffuseMapComponent>(m_Walkway[i], diffuse);
            }
        }
        {
            for (uint i = 0; i < HORN_ANIMATION_SPRITES; i++)
            {
                Builder builder{};

                auto sprite = m_SpritesheetHorn.GetSprite(i);
                glm::mat4 position = sprite->GetScaleMatrix();
                builder.LoadSprite(sprite, position, 10.0f /*amplification*/);
                auto model = Engine::m_Engine->LoadModel(builder);
                MeshComponent mesh{"horn animation", model};
                mesh.m_Enabled = false;

                m_Guybrush[i] = CreateEntity();
                m_Registry.emplace<MeshComponent>(m_Guybrush[i], mesh);

                TransformComponent transform{};
                transform.m_Translation = glm::vec3{-0.5f, 0.32f, 0.0f};
                transform.m_Scale = glm::vec3{0.005f};
                m_Registry.emplace<TransformComponent>(m_Guybrush[i], transform);

                DiffuseMapComponent diffuse{};
                m_Registry.emplace<DiffuseMapComponent>(m_Guybrush[i], diffuse);
            }
        }
        {
            Builder builder{};
            m_Ground = CreateEntity();

            builder.LoadModel("application/lucre/models/colored_cube.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"ground", model};
            m_Registry.emplace<MeshComponent>(m_Ground, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, 0.7f, 0.0f};
            transform.m_Scale = glm::vec3{0.01f, 100.0f, 2.0f};
            transform.m_Rotation = glm::vec3{0.0f, 0.0f, glm::half_pi<float>()};
            m_Registry.emplace<TransformComponent>(m_Ground, transform);

            DiffuseMapComponent diffuse{};
            m_Registry.emplace<DiffuseMapComponent>(m_Ground, diffuse);
        }
        {
            Builder builder{};
            m_Vase0 = CreateEntity();

            builder.LoadModel("application/lucre/models/flat_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"polygon vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase0, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{-0.8f, -0.2f, 0.0f};
            transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
            m_Registry.emplace<TransformComponent>(m_Vase0, transform);

            DiffuseMapComponent diffuse{};
            m_Registry.emplace<DiffuseMapComponent>(m_Vase0, diffuse);
        }
        {
            Builder builder{};

            m_Barrel = builder.LoadGLTF("application/lucre/models/barrel/barrel.gltf", m_Registry);
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"barrel", model};
            m_Registry.emplace<MeshComponent>(m_Barrel, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.0f, -0.2f, 0.0f};
            transform.m_Scale = glm::vec3{0.05f, 0.05f, 0.05f};
            m_Registry.emplace<TransformComponent>(m_Barrel, transform);

        }
        {
            Builder builder{};
            m_Vase1 = CreateEntity();

            builder.LoadModel("application/lucre/models/smooth_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"smooth vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase1, mesh);

            TransformComponent transform{};
            transform.m_Translation = glm::vec3{0.8f, -0.2f, 0.0f};
            transform.m_Scale = glm::vec3{2.0f, 2.0f, 2.0f};
            m_Registry.emplace<TransformComponent>(m_Vase1, transform);

            DiffuseMapComponent diffuse{};
            m_Registry.emplace<DiffuseMapComponent>(m_Vase1, diffuse);
        }

        {
            Builder builder{};
            builder.LoadModel("application/lucre/models/banana.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"banana", model};

            for (uint i = 0; i < MAX_B; i++)
            {
                m_Banana[i] = CreateEntity();

                m_Registry.emplace<MeshComponent>(m_Banana[i], mesh);

                TransformComponent transform{};
                if (i < 12)
                {
                    transform.m_Translation = glm::vec3{-3.0f + 0.5 * i, 0.5f, -0.6f};
                }
                else
                {
                    transform.m_Translation = glm::vec3{-3.0f + 0.5 * (i-12), 0.5f, 0.3f};
                }
                transform.m_Scale = glm::vec3{0.02f};
                transform.m_Rotation = glm::vec3{0.0f, 0.0f, 0.0f};
                m_Registry.emplace<TransformComponent>(m_Banana[i], transform);

                m_Registry.emplace<BananaComponent>(m_Banana[i], true);

                DiffuseMapComponent diffuse{};
                m_Registry.emplace<DiffuseMapComponent>(m_Banana[i], diffuse);

                b2BodyDef bodyDef;
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(0.0f, -1.0f);
                auto body = m_World->CreateBody(&bodyDef);

                b2CircleShape circle;
                circle.m_radius = 0.001f;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circle;
                fixtureDef.density = 1.0f;
                fixtureDef.friction = 0.2f;
                fixtureDef.restitution = 0.4f;
                body->CreateFixture(&fixtureDef);
                m_Registry.emplace<RigidbodyComponent>(m_Banana[i], RigidbodyComponent::DYNAMIC, body);

            }
        }
        {

            std::vector<glm::vec3> lightColors
            {
                {1.f, .1f, .1f},
                {.1f, .1f, 1.f},
                {.1f, 1.f, .1f},
                {1.f, 1.f, .1f},
                {.1f, 1.f, 1.f},
                {1.f, 1.f, 1.f}
            };

            for (int i = 0; i < lightColors.size(); i++)
            {
                m_PointLight[i] = CreatePointLight(0.2f, 0.1f, lightColors[i]);
                auto rotateLight = glm::rotate
                (
                    glm::mat4(1.0f),
                    (i * glm::two_pi<float>()) / lightColors.size(),
                    {0.f, -1.f, 0.f}
                );
                TransformComponent transform{};
                transform.m_Translation = glm::vec3(rotateLight * glm::vec4(-1.0f, +0.3f, -1.0f, 0.0f));
                m_Registry.emplace<TransformComponent>(m_PointLight[i], transform);
                m_Registry.emplace<Group1>(m_PointLight[i], true);
            }
        }
        {
            // light the vulcano
            m_PointLightVulcano = CreatePointLight(1.0f, 0.0f, {1.0f, 0.0f, 0.0f});
            TransformComponent transform{};
            transform.m_Translation = glm::vec3(0.0f, -10.0f, 39.0f);
            m_Registry.emplace<TransformComponent>(m_PointLightVulcano, transform);
            m_Registry.emplace<Group2>(m_PointLightVulcano, true);
        }
    }
}
