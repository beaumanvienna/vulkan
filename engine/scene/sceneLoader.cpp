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

// the YAML loader is deprecated, use the JSON loader

#include <fstream>

#include "scene/sceneLoader.h"
#include "auxiliary/file.h"
#include "scene/components.h"
#include "renderer/builder/gltfBuilder.h"

namespace GfxRenderEngine
{
    SceneLoader::SceneLoader(Scene& scene)
        : m_Scene(scene)
    {
    }

    void SceneLoader::Deserialize()
    {

        YAML::Node yamlNode;

        if (EngineCore::FileExists(m_Scene.m_Filepath))
        {
            LOG_CORE_INFO("Loading scene {0}", m_Scene.m_Filepath);
            yamlNode = YAML::LoadFile(m_Scene.m_Filepath);
        }
        else if (EngineCore::FileExists(m_Scene.m_AlternativeFilepath))
        {
            LOG_CORE_INFO("Loading scene {0}", m_Scene.m_AlternativeFilepath);
            yamlNode = YAML::LoadFile(m_Scene.m_AlternativeFilepath);
        }
        else
        {
            LOG_CORE_CRITICAL("Scene loader could neither find file {0} nor file {1}", m_Scene.m_Filepath, m_Scene.m_AlternativeFilepath);
            return;
        }

        for(const auto& gltfFile : yamlNode["glTF-files"])
        {
            std::string filename = gltfFile.first.as<std::string>();
            bool sucessful = false;
            if (EngineCore::FileExists(filename))
            {
                LOG_CORE_INFO("Scene loader found {0}", filename);
                GltfBuilder builder(filename, m_Scene);
                sucessful = builder.Load();

                if (sucessful)
                {
                    std::string entityName = filename + std::string("::0::root");
                    entt::entity entity = m_Scene.m_Dictionary.Retrieve(entityName);
                    Gltf::GltfFile gltfFileFromScene(filename);
                    Gltf::Instance gltfFileInstance(entity);
                    gltfFileFromScene.m_Instances.push_back(gltfFileInstance);
                    m_GltfFiles.m_GltfFilesFromScene.push_back(gltfFileFromScene);

                    switch (gltfFile.second.Type())
                    {
                        case YAML::NodeType::Map:
                        {
                            auto& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                            for(const auto& attribute : gltfFile.second)
                            {
                                if (attribute.first.as<std::string>() == "translation")
                                {
                                    auto translation = ConvertToVec3(attribute.second);
                                    transform.SetTranslation(translation);
                                }
                                else if (attribute.first.as<std::string>() == "scale")
                                {
                                    auto scale = ConvertToVec3(attribute.second);
                                    transform.SetScale(scale);
                                }
                                else if (attribute.first.as<std::string>() == "rotation")
                                {
                                    auto rotation = ConvertToVec3(attribute.second);
                                    transform.SetRotation(rotation);
                                } 
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            else
            {
                LOG_CORE_CRITICAL("Scene loader could not find file {0}", filename);
            }
        }

        if (yamlNode["prefabs"])
        {
            const auto& prefabsFileList = yamlNode["prefabs"];
            for (const auto& prefab : prefabsFileList)
            {
                auto filename = prefab.as<std::string>();
                LoadPrefab(filename);
                m_PrefabFiles.push_back(filename);
            }
        }
        if (yamlNode["script-components"])
        {
            const auto& scriptFileList = yamlNode["script-components"];
            for(YAML::const_iterator it=scriptFileList.begin();it!=scriptFileList.end();++it)
            {
                std::string entityName = it->first.as<std::string>();
                std::string filepath = it->second.as<std::string>();
                LOG_CORE_INFO("found script '{0}' for entity '{1}' in scene description", filepath, entityName);
                entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(entityName);

                ScriptComponent scriptComponent(filepath);
                m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
            }
        }
    }

    glm::vec3 SceneLoader::ConvertToVec3(const YAML::Node& node)
    {
        float values[3] = {0.0f, 0.0f, 0.0f};

        uint i = 0;
        for(const auto& vectorElement : node)
        {
            values[i] = vectorElement.as<float>();
            i++;
        }
        return glm::vec3(values[0], values[1], values[2]);
    }

    void SceneLoader::LoadPrefab(const std::string& filepath)
    {
        YAML::Node yamlNode;

        if (EngineCore::FileExists(filepath))
        {
            LOG_CORE_INFO("Scene loader found {0}", filepath);
            yamlNode = YAML::LoadFile(filepath);
        }
        else
        {
            LOG_CORE_CRITICAL("Scene loader could not find file {0}", filepath);
            return;
        }

        if (yamlNode["glTF-files"])
        {
            const auto& gltfFileList = yamlNode["glTF-files"];
            for (const auto& gltfFile : gltfFileList)
            {
                auto filename = gltfFile.as<std::string>();
                bool sucessful = false;
                if (EngineCore::FileExists(filename))
                {
                    LOG_CORE_INFO("Scene loader found {0}", filename);
                    GltfBuilder builder(filename, m_Scene);
                    sucessful = builder.Load();
                }
                else
                {
                    LOG_CORE_CRITICAL("Scene loader could not find file {0}", filename);
                }

                if (sucessful)
                {
                    std::string entityName = filename + std::string("::0::root");
                    entt::entity entity = m_Scene.m_Dictionary.Retrieve(entityName);
                    Gltf::GltfFile gltfFileFromScene(filename);
                    Gltf::Instance gltfFileInstance(entity);
                    gltfFileFromScene.m_Instances.push_back(gltfFileInstance);
                    m_GltfFiles.m_GltfFilesFromPreFabs.push_back(gltfFileFromScene);
                }
            }
        }

        if (yamlNode["prefabs"])
        {
            const auto& prefabsFileList = yamlNode["prefabs"];
            for (const auto& prefab : prefabsFileList)
            {
                LoadPrefab(prefab.as<std::string>());
            }
        }

        if (yamlNode["script-components"])
        {
            const auto& scriptFileList = yamlNode["script-components"];
            for(YAML::const_iterator it=scriptFileList.begin();it!=scriptFileList.end();++it)
            {
                std::string entityName = it->first.as<std::string>();
                std::string filename = it->second.as<std::string>();
                LOG_CORE_INFO("found script '{0} for entity '{1}' in prefab", filename, entityName);
                entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(entityName);

                ScriptComponent scriptComponent(filename);
                m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
            }
        }
    }

    void SceneLoader::Serialize()
    {
        auto& filepath = m_Scene.m_Filepath;
        YAML::Emitter out;

        out << YAML::Comment("Lucre scene description file");

        out << YAML::BeginMap;

        // glTF-files
        {
            out << YAML::Key << "glTF-files";
            out << YAML::BeginMap;
 
            auto& registry = m_Scene.GetRegistry();
            for (const auto& gltfFile : m_GltfFiles.m_GltfFilesFromScene)
            {
                auto& filename = gltfFile.m_Filename;
                auto& entity = gltfFile.m_Instances[0].m_Entity;

                auto& transform = registry.get<TransformComponent>(entity);
                auto& translation = transform.GetTranslation();
                auto& scale = transform.GetScale();
                auto& rotation = transform.GetRotation();
                out << YAML::Key << filename;
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "translation" 
                        << YAML::Value << YAML::Flow
                        << YAML::BeginSeq
                            << translation.x 
                            << translation.y
                            << translation.z
                        << YAML::EndSeq;
                        out << YAML::Key << "scale" 
                        << YAML::Value << YAML::Flow
                        << YAML::BeginSeq
                            << scale.x
                            << scale.y
                            << scale.z
                        << YAML::EndSeq;
                        out << YAML::Key << "rotation"
                        << YAML::Value << YAML::Flow
                        << YAML::BeginSeq
                            << rotation.x
                            << rotation.y
                            << rotation.z
                        << YAML::EndSeq;
                    out << YAML::EndMap;
                }
            }    
            out << YAML::EndMap;
        }

        // prefabs
        {
            out << YAML::Key << "prefabs";
            out << YAML::BeginSeq;
    
            for (const auto& filename : m_PrefabFiles)
            {
                out << YAML::Key << filename;
            }    
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();

    }
}
