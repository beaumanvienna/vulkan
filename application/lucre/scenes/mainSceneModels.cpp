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
#include "renderer/texture.h"
#include "auxiliary/file.h"

#include "mainScene.h"

namespace LucreApp
{
    void MainScene::LoadModels()
    {
        {
            std::vector<std::string> faces = {
                "application/lucre/models/assets/Skybox/right.png", "application/lucre/models/assets/Skybox/left.png",
                "application/lucre/models/assets/Skybox/top.png",   "application/lucre/models/assets/Skybox/bottom.png",
                "application/lucre/models/assets/Skybox/front.png", "application/lucre/models/assets/Skybox/back.png"};

            Builder builder;
            m_Skybox = builder.LoadCubemap(faces, m_Registry);
            auto view = m_Registry.view<TransformComponent>();
            auto& skyboxTransform = view.get<TransformComponent>(m_Skybox);
            skyboxTransform.SetScale(300.0f);
        }
        {
            float scaleHero = 0.0038f;
            m_SpritesheetHorn.SetScale(scaleHero);
            for (uint i = 0; i < HORN_ANIMATION_SPRITES; i++)
            {
                auto sprite = m_SpritesheetHorn.GetSprite(i);
                float width = sprite.GetWidth();
                float height = sprite.GetHeight();

                Builder builder{};
                builder.LoadSprite(sprite, 1.0f /*amplification*/);

                auto model = Engine::m_Engine->LoadModel(builder);
                MeshComponent mesh{"horn animation", model};
                mesh.m_Enabled = false;

                m_Guybrush[i] = m_Registry.Create();
                m_Registry.emplace<MeshComponent>(m_Guybrush[i], mesh);

                TransformComponent transform{};
                transform.SetTranslation(glm::vec3{-0.5f, 0.37f, 0.0f});
                transform.SetScale({width, height, 1.0f});
                m_Registry.emplace<TransformComponent>(m_Guybrush[i], transform);

                SpriteRendererComponent spriteRendererComponent{};
                m_Registry.emplace<SpriteRendererComponent>(m_Guybrush[i], spriteRendererComponent);
            }
        }
        {
            FastgltfBuilder builder("application/lucre/models/external_3D_files/banana/banana.gltf", *this);
            builder.SetDictionaryPrefix("mainScene");
            builder.Load(MAX_B /*instance(s)*/);

            for (uint i = 0; i < MAX_B; i++)
            {
                m_Banana[i] = m_Dictionary.Retrieve(
                    "mainScene::application/lucre/models/external_3D_files/banana/banana.gltf::" + std::to_string(i) +
                    "::root");

                TransformComponent transform{};
                if (i < 12)
                {
                    transform.SetTranslation(glm::vec3{-3.0f + 0.5 * i, 0.5f, -0.6f});
                }
                else
                {
                    transform.SetTranslation(glm::vec3{-3.0f + 0.5 * (i - 12), 0.5f, 0.3f});
                }
                m_Registry.emplace<BananaComponent>(m_Banana[i], true);

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

            std::vector<glm::vec3> lightColors = {{1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
                                                  {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f}};

            for (size_t i = 0; i < lightColors.size(); i++)
            {
                m_PointLight[i] = CreatePointLight(POINT_LIGHT_INTENSITY, 0.1f, lightColors[i]);
                auto rotateLight =
                    glm::rotate(glm::mat4(1.0f), (i * glm::two_pi<float>()) / lightColors.size(), {0.f, -1.f, 0.f});
                auto& transform = m_Registry.get<TransformComponent>(m_PointLight[i]);
                glm::vec3 translation = glm::vec3(rotateLight * glm::vec4(-1.0f, 0.25f, 1.0f, 0.0f));
                transform.SetTranslation(translation);
                m_Registry.emplace<Group1>(m_PointLight[i], true);
            }
        }
    }
} // namespace LucreApp
