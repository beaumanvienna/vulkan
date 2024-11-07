/* Engine Copyright (c) 2024 Engine Development Team
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

#include "simdjson.h"
#include "auxiliary/file.h"
#include "auxiliary/random.h"
#include "particleSystem/snow.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/fastgltfBuilder.h"

using namespace simdjson;

namespace GfxRenderEngine
{
    Snow::Snow(Scene& scene, std::string const& jsonFile)
        : m_Scene{scene}, m_Registry{scene.GetRegistry()}, m_Dictionary{scene.GetDictionary()}
    {
        // load JSON particle system description
        ParseSysDescription(jsonFile);
        if (!m_Initialized)
        {
            LOG_CORE_CRITICAL("Snow::Snow failed to initialize! (ParseSysDescription)");
            return;
        }

        // load model with m_PoolSize instances
        std::vector<entt::entity> snowflakeFirstInstances;
        {
            auto& dictionaryPrefix = m_SysDescription.m_DictionaryPrefix.value();
            auto& model = m_SysDescription.m_Model.value();
            auto& sceneGraph = m_Scene.GetSceneGraph();
            auto& numberOfInstances = m_SysDescription.m_PoolSize.value();

            // create group node
            auto entity = m_Registry.Create();
            auto name = dictionaryPrefix + "::" + model + "::root";
            int groupNode = sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
            TransformComponent transform{};
            m_Registry.emplace<TransformComponent>(entity, transform);

            FastgltfBuilder builder(model, m_Scene, groupNode);
            builder.SetDictionaryPrefix(dictionaryPrefix);
            m_Initialized = builder.Load(numberOfInstances, snowflakeFirstInstances, NO_SCENE_GRAPH);
        }

        if (!m_Initialized || !snowflakeFirstInstances.size())
        {
            LOG_CORE_CRITICAL("Snow::Snow failed to initialize! (load model)");
            return;
        }

        // set up particles
        {
            m_ParticlePool.resize(m_SysDescription.m_PoolSize.value());
            auto prefix = std::string(m_SysDescription.m_DictionaryPrefix.value()) + "::";
            auto& vertex1 = m_SysDescription.m_Vertex1.value();
            auto& vertex2 = m_SysDescription.m_Vertex2.value();
            auto volumeSize = (vertex2 - vertex1) / 2.0f;
            auto& snowflake = snowflakeFirstInstances[0];

            if (snowflake != entt::null)
            {
                auto& instances = m_Registry.get<InstanceTag>(snowflake).m_Instances;
                uint index{0};
                for (auto& particle : m_ParticlePool)
                {
                    particle.m_RotationSpeed = {0.0f, 0.0f, EngineCore::RandomPlusMinusOne()};
                    particle.m_Velocity = {0.0f, -1.0f + EngineCore::RandomPlusMinusOne(), 0.0f};

                    auto& transform = m_Registry.get<TransformComponent>(instances[index]);
                    particle.m_Transform = &transform;
                    transform.SetRotation(
                        {glm::half_pi<float>(), 0.0f, glm::pi<float>() * EngineCore::RandomPlusMinusOne()});
                    transform.SetTranslation(
                        {(vertex1.x + volumeSize.x) + (volumeSize.x * EngineCore::RandomPlusMinusOne()),
                         (vertex1.y + volumeSize.y) + (volumeSize.y * EngineCore::RandomPlusMinusOne()),
                         (vertex1.z + volumeSize.z) + (volumeSize.z * EngineCore::RandomPlusMinusOne())});
                    transform.SetScale(0.014f);
                    ++index;
                }
            }
        }
    }

    void Snow::OnUpdate(Timestep timestep, TransformComponent& cameraTransform)
    {
        ZoneScopedNC("Snow::OnUpdate", 0x00ff00);
        auto& vertex1 = m_SysDescription.m_Vertex1.value();
        auto& vertex2 = m_SysDescription.m_Vertex2.value();
        for (auto& particle : m_ParticlePool)
        {
            auto& transform = *particle.m_Transform;
            float rotationSpeedZ = particle.m_RotationSpeed.z;
            transform.AddRotation({0.0f, 0.0f, timestep * rotationSpeedZ});
            transform.SetRotationY(cameraTransform.GetRotation().y);

            transform.AddTranslation(timestep * particle.m_Velocity);
            if (transform.GetTranslation().y <= vertex1.y)
            {
                transform.AddTranslation({0.0f, vertex2.y + vertex1.y, 0.0f});
            }
            transform.SetMat4Global();
        }
    }

    void Snow::ParseSysDescription(std::string const& jsonFile)
    {
        auto path = std::string("application/lucre/particleSystem/") + jsonFile;
        if (!EngineCore::FileExists(path))
        {
            LOG_CORE_CRITICAL("particle system description not found: {0}", path);
            return;
        }

        auto convertToVec3 = [](ondemand::array arrayJSON)
        {
            glm::vec3 returnVec3{0.0f};
            uint componentIndex = 0;
            for (auto component : arrayJSON)
            {
                switch (componentIndex)
                {
                    case 0:
                    {
                        returnVec3.x = component.get_double();
                        break;
                    }
                    case 1:
                    {
                        returnVec3.y = component.get_double();
                        break;
                    }
                    case 2:
                    {
                        returnVec3.z = component.get_double();
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                ++componentIndex;
            }
            CORE_ASSERT(componentIndex == 3, "convertToVec3(...) argument must have 3 components");
            return returnVec3;
        };

        LOG_CORE_INFO("loading particle system: {0}", path);
        ondemand::parser parser;
        padded_string json = padded_string::load(path);

        ondemand::document sceneDocument = parser.iterate(json);
        ondemand::object sceneObjects = sceneDocument.get_object();

        for (auto sceneObject : sceneObjects)
        {
            std::string_view sceneObjectKey = sceneObject.unescaped_key();

            if (sceneObjectKey == "file format identifier")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::number), "type must be number");
                // only check major version of "file format identifier"
                double fileFormatIdentifier = sceneObject.value().get_double();
                CORE_ASSERT((std::trunc(fileFormatIdentifier) == std::trunc(SUPPORTED_FILE_FORMAT_VERSION)),
                            "The scene description major version does not match");
            }
            else if (sceneObjectKey == "description")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view sceneDescription = sceneObject.value().get_string();
                LOG_CORE_INFO("description: {0}", sceneDescription);
            }
            else if (sceneObjectKey == "author")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view sceneAuthor = sceneObject.value().get_string();
                LOG_CORE_INFO("author: {0}", sceneAuthor);
            }
            else if (sceneObjectKey == "model")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view model = sceneObject.value().get_string();
                m_SysDescription.m_Model = model;
                LOG_CORE_INFO("author: {0}", model);
            }
            else if (sceneObjectKey == "pool size")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::number), "type must be number");
                int64 poolSize = sceneObject.value().get_int64();
                m_SysDescription.m_PoolSize = poolSize;
            }
            else if (sceneObjectKey == "prefix dictionary")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view prefix = sceneObject.value().get_string();
                m_SysDescription.m_DictionaryPrefix = prefix;
            }
            else if (sceneObjectKey == "cubic volume vertex 0,0,0")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                m_SysDescription.m_Vertex1 = convertToVec3(sceneObject.value());
            }
            else if (sceneObjectKey == "cubic volume vertex 1,1,1")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                m_SysDescription.m_Vertex2 = convertToVec3(sceneObject.value());
            }
        }
        auto& descr = m_SysDescription;
        m_Initialized = descr.m_Model.has_value() &&            //
                        descr.m_PoolSize.has_value() &&         //
                        descr.m_DictionaryPrefix.has_value() && //
                        descr.m_Vertex1.has_value() &&          //
                        descr.m_Vertex2.has_value();            //
        CORE_ASSERT(m_Initialized, "JSON particle system description did not load properly");
    }
} // namespace GfxRenderEngine
