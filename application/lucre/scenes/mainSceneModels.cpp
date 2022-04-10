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
#include "auxiliary/file.h"

#include "mainScene.h"

namespace LucreApp
{
    void MainScene::LoadModels()
    {
        // --- glTF assets ---
        if (EngineCore::FileExists("application/lucre/models/dont_upload/multipleObjects/multipleObjects.gltf"))
        {
            Builder builder{"application/lucre/models/dont_upload/multipleObjects/multipleObjects.gltf"};
        
            builder.LoadGLTF(m_Registry, m_SceneHierarchy);
        }

        // --- sprites from the built-in texture atlas ---
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
                transform.SetTranslation(glm::vec3{-77.0f + 77.0f*i, 3.0f, 40.0f});
                transform.SetScale(glm::vec3{0.17f, 0.14f, 0.17f} * 0.7f);
                if (flip) transform.SetRotation(glm::vec3{0.0f, glm::pi<float>(), 0.0f});
                m_Registry.emplace<TransformComponent>(m_Vulcano[i], transform);

                DefaultDiffuseComponent defaultDiffuseComponent{0.1f, 0.1f};
                m_Registry.emplace<DefaultDiffuseComponent>(m_Vulcano[i], defaultDiffuseComponent);
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
                transform.SetTranslation(glm::vec3{-11.0f + 11.0f*i, 0.67f, -0.1f});
                transform.SetScale(glm::vec3{0.017f, 0.014f, 0.017f});
                transform.SetRotation(glm::vec3{-glm::half_pi<float>(), 0.0f, 0.0f});
                m_Registry.emplace<TransformComponent>(m_Walkway[i], transform);

                DefaultDiffuseComponent defaultDiffuseComponent{};
                m_Registry.emplace<DefaultDiffuseComponent>(m_Walkway[i], defaultDiffuseComponent);
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
                transform.SetTranslation(glm::vec3{-0.5f, 0.32f, 0.0f});
                transform.SetScale(glm::vec3{0.005f});
                m_Registry.emplace<TransformComponent>(m_Guybrush[i], transform);

                DefaultDiffuseComponent defaultDiffuseComponent{};
                m_Registry.emplace<DefaultDiffuseComponent>(m_Guybrush[i], defaultDiffuseComponent);
            }
        }
        // --- Obj files ---
        // --- Note: It is recommended to use the glTF file format for assets rather than Obj Wavefront ---
        {
            Builder builder{};
            m_Ground = CreateEntity();

            builder.LoadModel("application/lucre/models/colored_cube.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"ground", model};
            m_Registry.emplace<MeshComponent>(m_Ground, mesh);

            TransformComponent transform{};
            transform.SetTranslation(glm::vec3{0.0f, 0.7f, 0.0f});
            transform.SetScale(glm::vec3{0.01f, 100.0f, 2.0f});
            transform.SetRotation(glm::vec3{0.0f, 0.0f, glm::half_pi<float>()});
            m_Registry.emplace<TransformComponent>(m_Ground, transform);

            PbrNoMapComponent pbrNoMapComponent{};
            pbrNoMapComponent.m_Roughness = 0.2;
            pbrNoMapComponent.m_Metallic  = 0.0;
            m_Registry.emplace<PbrNoMapComponent>(m_Ground, pbrNoMapComponent);
        }
        {
            Builder builder{};
            m_Vase0 = CreateEntity();

            builder.LoadModel("application/lucre/models/flat_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"polygon vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase0, mesh);

            TransformComponent transform{};
            transform.SetTranslation(glm::vec3{-0.8f, -0.2f, 0.0f});
            transform.SetScale(glm::vec3{2.0f, 2.0f, 2.0f});
            m_Registry.emplace<TransformComponent>(m_Vase0, transform);

            PbrNoMapComponent pbrNoMapComponent{};
            pbrNoMapComponent.m_Roughness = 0.3;
            pbrNoMapComponent.m_Metallic  = 0.0;
            m_Registry.emplace<PbrNoMapComponent>(m_Vase0, pbrNoMapComponent);
        }
        {
            Builder builder{};
            m_Vase1 = CreateEntity();

            builder.LoadModel("application/lucre/models/smooth_vase.obj");
            auto model = Engine::m_Engine->LoadModel(builder);
            MeshComponent mesh{"smooth vase", model};
            m_Registry.emplace<MeshComponent>(m_Vase1, mesh);

            TransformComponent transform{};
            transform.SetTranslation(glm::vec3{0.8f, -0.2f, 0.0f});
            transform.SetScale(glm::vec3{2.0f, 2.0f, 2.0f});
            m_Registry.emplace<TransformComponent>(m_Vase1, transform);

            PbrNoMapComponent pbrNoMapComponent{};
            pbrNoMapComponent.m_Roughness = 0.08;
            pbrNoMapComponent.m_Metallic  = 0.5;
            m_Registry.emplace<PbrNoMapComponent>(m_Vase1, pbrNoMapComponent);
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
                    transform.SetTranslation(glm::vec3{-3.0f + 0.5 * i, 0.5f, -0.6f});
                }
                else
                {
                    transform.SetTranslation(glm::vec3{-3.0f + 0.5 * (i-12), 0.5f, 0.3f});
                }
                transform.SetScale(glm::vec3{0.02f});
                transform.SetRotation(glm::vec3{0.0f, 0.0f, 0.0f});
                m_Registry.emplace<TransformComponent>(m_Banana[i], transform);

                m_Registry.emplace<BananaComponent>(m_Banana[i], true);

                PbrNoMapComponent pbrNoMapComponent{};
                pbrNoMapComponent.m_Roughness = 0.1;
                pbrNoMapComponent.m_Metallic  = 0.2;
                m_Registry.emplace<PbrNoMapComponent>(m_Banana[i], pbrNoMapComponent);

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
                transform.SetTranslation(glm::vec3(rotateLight * glm::vec4(-1.0f, +0.3f, -1.0f, 0.0f)));
                m_Registry.emplace<TransformComponent>(m_PointLight[i], transform);
                m_Registry.emplace<Group1>(m_PointLight[i], true);
            }
        }
        {
            // light the vulcano
            m_PointLightVulcano = CreatePointLight(1.0f, 0.0f, {1.0f, 0.0f, 0.0f});
            TransformComponent transform{};
            transform.SetTranslation(glm::vec3(0.0f, -10.0f, 39.0f));
            m_Registry.emplace<TransformComponent>(m_PointLightVulcano, transform);
            m_Registry.emplace<Group2>(m_PointLightVulcano, true);
        }
    }
}
