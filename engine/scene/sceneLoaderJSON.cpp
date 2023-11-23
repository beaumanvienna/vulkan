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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>
#include "simdjson.h"

#include "core.h"
#include "auxiliary/file.h"
#include "scene/sceneLoaderJSON.h"

namespace GfxRenderEngine
{

    using namespace simdjson;

    namespace JSON
    {
        void PrintType(ondemand::value elementJSON);
        glm::vec3 ConvertToVec3(ondemand::array arrayJSON);
    }

    SceneLoaderJSON::SceneLoaderJSON(Scene& scene)
        : m_Scene(scene)
    {
    }

    void SceneLoaderJSON::Deserialize(std::string& filepath, std::string& alternativeFilepath, bool isPrefab)
    {
        ondemand::parser parser;
        padded_string json;

        if (EngineCore::FileExists(filepath))
        {
            json = padded_string::load(filepath);
            isPrefab ? LOG_CORE_INFO("Loading prefab {0}", filepath) : LOG_CORE_INFO("Loading scene {0}", filepath);
        }
        else if (EngineCore::FileExists(alternativeFilepath))
        {
            json = padded_string::load(alternativeFilepath);
            isPrefab ? LOG_CORE_INFO("Loading prefab {0}", alternativeFilepath) : LOG_CORE_INFO("Loading scene {0}", alternativeFilepath);
        }
        else
        {
            LOG_CORE_CRITICAL("Scene loader could neither find file {0} nor file {1}", filepath, alternativeFilepath);
            return;
        }

        ondemand::document sceneDocument = parser.iterate(json);
        ondemand::object sceneObjects = sceneDocument.get_object();

        for (auto sceneObject : sceneObjects)
        {
            std::string_view keyv = sceneObject.unescaped_key();

            if (keyv == "file format identifier")
            {
                CORE_ASSERT( (sceneObject.value().type() == ondemand::json_type::number), "type must be number" );
                CORE_ASSERT( (sceneObject.value().get_int64() == 1), "The scene description version must be 1" );
            }
            else if (keyv == "description")
            {
                LOG_CORE_INFO("description: {0}", sceneObject.value().get_string());
                CORE_ASSERT( (sceneObject.value().type() == ondemand::json_type::string), "type must be string" );
            }
            else if (keyv == "author")
            {
                LOG_CORE_INFO("author: {0}", sceneObject.value().get_string());
                CORE_ASSERT( (sceneObject.value().type() == ondemand::json_type::string), "type must be string" );
            }
            else if (keyv == "gltf files")
            {
                CORE_ASSERT( (sceneObject.value().type() == ondemand::json_type::array), "type must be array" );
                auto gltfFiles = sceneObject.value().get_array();
                {
                    int fileCnt = gltfFiles.count_elements();
                    fileCnt == 1 ? LOG_CORE_INFO("loading 1 gltf file") : LOG_CORE_INFO("loading {0} gltf files", fileCnt);
                }
                for (auto gltfFileJSON : gltfFiles)
                {
                    CORE_ASSERT( (gltfFileJSON.value().type() == ondemand::json_type::object), "type must be object" );
                    std::string_view gltfFilenameStringView = gltfFileJSON.find_field("filename").get_string();
                    std::string gltfFilename = std::string(gltfFilenameStringView);

                    if (EngineCore::FileExists(gltfFilename))
                    {
                        LOG_CORE_INFO("Scene loader found {0}", gltfFilename);

                        Builder builder{gltfFilename};
                        auto entity = builder.LoadGLTF(m_Scene.m_Registry, m_Scene.m_SceneHierarchy, m_Scene.m_Dictionary);

                        if ((entity != entt::null) && (!isPrefab))
                        {
                            m_GltfFiles.m_GltfFilesFromScene.push_back({gltfFilename, entity});
                        }

                        auto instances = gltfFileJSON.find_field("instances").get_array();
                        int instanceCount = instances.count_elements();

                        CORE_ASSERT( (instanceCount > 0), "no instances found");
                        {
                            uint index = 0;
                            for (auto instance : instances)
                            {
                                // transform
                                TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                                ondemand::object transformJSON = instance.find_field("transform");

                                ondemand::array scaleJSON = transformJSON.find_field("scale");
                                glm::vec3 scale = JSON::ConvertToVec3(scaleJSON);
                                transform.SetScale(scale);

                                ondemand::array rotationJSON = transformJSON.find_field("rotation");
                                glm::vec3 rotation = JSON::ConvertToVec3(rotationJSON);
                                transform.SetRotation(rotation);

                                ondemand::array translationJSON = transformJSON.find_field("translation");
                                glm::vec3 translation = JSON::ConvertToVec3(translationJSON);
                                transform.SetTranslation(translation);

                                // nodes
                                std::string_view entityName;
                                std::string_view scriptComponentStringView;
                                double walkSpeed;
                                bool rigidBody;
                                bool scriptComponentFound = true;
                                try
                                {
                                    ondemand::array nodesJSON = instance.find_field("nodes");
                                    for (auto nodeJSON : nodesJSON)
                                    {
                                        CORE_ASSERT( (nodeJSON.value().type() == ondemand::json_type::object), "type must be object" );
                                        entityName = nodeJSON.find_field("name");
                                        walkSpeed = nodeJSON.find_field("walkSpeed");
                                        rigidBody = nodeJSON.find_field("rigidBody");
                                        scriptComponentStringView = nodeJSON.find_field("script-component");
                                    }
                                } catch(simdjson_error& e)
                                {
                                    scriptComponentFound = false;
                                }
                                if (scriptComponentFound)
                                {
                                    std::string fullEntityName = gltfFilename + std::string("::" + std::string(entityName));
                                    entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(std::string(fullEntityName));
                                    LOG_CORE_INFO("found script '{0}' for entity '{1}' in scene description", scriptComponentStringView, fullEntityName);

                                    ScriptComponent scriptComponent(scriptComponentStringView);
                                    m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
                                }
                                ++index;
                                if ( index == 1) break; // only one instance supported right now
                            }
                        }
                    }
                }
            }
            else if (keyv == "prefabs")
            {
                CORE_ASSERT( (sceneObject.value().type() == ondemand::json_type::array), "type must be array" );
                auto prefabsJSON = sceneObject.value().get_array();
                for (auto prefabJSON : prefabsJSON)
                {
                    CORE_ASSERT( (prefabJSON.value().type() == ondemand::json_type::object), "type must be object" );
                    std::string_view prefabFilenameStringView = prefabJSON.find_field("filename").get_string();
                    std::string prefabFilename = std::string(prefabFilenameStringView);
                    if (EngineCore::FileExists(prefabFilename))
                    {
                        LOG_CORE_INFO("Scene loader found prefab {0}", prefabFilename);
                        Deserialize(prefabFilename, prefabFilename, true /*bool isPrefab*/);
                        m_PrefabFiles.push_back(prefabFilename);
                    }
                }
            }
        }
    }

    void SceneLoaderJSON::Serialize()
    {
        //auto& filepath = m_Scene.m_Filepath;
    }

    namespace JSON
    {
        glm::vec3 ConvertToVec3(ondemand::array arrayJSON)
        {
            glm::vec3 returnVec3{0.0f};
            uint index = 0;
            for (auto component : arrayJSON)
            {
                switch (index)
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
                        LOG_CORE_ERROR("JSON::CConvertToVec3(...) argument must have 3 components");
                        break;
                    }
                }
                ++index;
            }
            return returnVec3;
        }

        void PrintType(ondemand::value elementJSON)
        {
            switch (elementJSON.type())
            {
                case ondemand::json_type::array:
                {
                    LOG_APP_INFO("array");
                    break;
                }
                case ondemand::json_type::object:
                {
                    LOG_APP_INFO("object");
                    break;
                }
                case ondemand::json_type::number:
                {
                    LOG_APP_INFO("number");
                    break;
                }
                case ondemand::json_type::string:
                {
                    LOG_APP_INFO("string");
                    break;
                }
                case ondemand::json_type::boolean:
                {
                    LOG_APP_INFO("boolean");
                    break;
                }
                case ondemand::json_type::null:
                {
                    LOG_APP_INFO("null");
                    break;
                }
                default:
                {
                    LOG_APP_INFO("type not found");
                    break;
                }
            }
        }
    }
}
