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

#include "core.h"
#include "auxiliary/file.h"
#include "scene/sceneLoaderJSON.h"

namespace GfxRenderEngine
{

    SceneLoaderJSON::SceneLoaderJSON(Scene& scene)
        : m_Scene(scene)
    {
    }

    void SceneLoaderJSON::Deserialize(std::string& filepath, std::string& alternativeFilepath)
    {
        if (EngineCore::FileExists(filepath))
        {
            LOG_CORE_INFO("Loading scene {0}", filepath);
            Deserialize(filepath);
        }
        else if (EngineCore::FileExists(alternativeFilepath))
        {
            LOG_CORE_INFO("Loading scene {0}", alternativeFilepath);
            Deserialize(alternativeFilepath);
        }
        else
        {
            LOG_CORE_CRITICAL("Scene loader could neither find file {0} nor file {1}", filepath, alternativeFilepath);
            return;
        }
    }

    void SceneLoaderJSON::Deserialize(std::string& filepath)
    {
        ondemand::parser parser;
        padded_string json = padded_string::load(filepath);

        ondemand::document sceneDocument = parser.iterate(json);
        ondemand::object sceneObjects = sceneDocument.get_object();

        for (auto sceneObject : sceneObjects)
        {
            std::string_view sceneObjectKey = sceneObject.unescaped_key();

            if (sceneObjectKey == "file format identifier")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::number), "type must be number" );

                // only check major version of "file format identifier"
                m_SceneDescriptionFile.m_FileFormatIdentifier = sceneObject.value().get_double();
                CORE_ASSERT
                (
                    (std::trunc(m_SceneDescriptionFile.m_FileFormatIdentifier) == std::trunc(SUPPORTED_FILE_FORMAT_VERSION)),
                    "The scene description major version does not match"
                );
            }
            else if (sceneObjectKey == "description")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string" );
                std::string_view sceneDescription = sceneObject.value().get_string();
                m_SceneDescriptionFile.m_Description = std::string(sceneDescription);
                LOG_CORE_INFO("description: {0}", m_SceneDescriptionFile.m_Description);
            }
            else if (sceneObjectKey == "author")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string" );
                std::string_view sceneAuthor = sceneObject.value().get_string();
                m_SceneDescriptionFile.m_Author = std::string(sceneAuthor);
                LOG_CORE_INFO("author: {0}", m_SceneDescriptionFile.m_Author);
            }
            else if (sceneObjectKey == "gltf files")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array" );
                auto gltfFiles = sceneObject.value().get_array();
                {
                    int gltfFileCount = gltfFiles.count_elements();
                    gltfFileCount == 1 ? LOG_CORE_INFO("loading 1 gltf file") : LOG_CORE_INFO("loading {0} gltf files", gltfFileCount);
                }

                for (auto gltfFileJSON : gltfFiles)
                {
                    ParseGltfFileJSON(gltfFileJSON);
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized scene object");
            }
        }
        m_Scene.CreateLinearMap();
    }

    void SceneLoaderJSON::ParseGltfFileJSON(ondemand::object gltfFileJSON)
    {
        std::string gltfFilename;
        entt::entity entity = entt::null;
        std::vector<Gltf::GltfFile>* gltfFiles = nullptr;

        for (auto gltfFileObject : gltfFileJSON)
        {
            std::string_view gltfFileObjectKey = gltfFileObject.unescaped_key();

            if (gltfFileObjectKey == "filename")
            {
                std::string_view gltfFilenameStringView = gltfFileObject.value().get_string();
                gltfFilename = std::string(gltfFilenameStringView);
                if (EngineCore::FileExists(gltfFilename))
                {
                    LOG_CORE_INFO("Scene loader found {0}", gltfFilename);

                    Builder builder{gltfFilename};
                    entity = builder.LoadGLTF(m_Scene.m_Registry, m_Scene.m_SceneHierarchy, m_Scene.m_Dictionary);

                    if (entity != entt::null)
                    {
                        Gltf::GltfFile gltfFile(gltfFilename);
                        gltfFiles = &m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene;

                        gltfFiles->push_back(gltfFile);
                    }
                }
                else
                {
                    LOG_CORE_ERROR("gltf file not found: {0}", gltfFilename);
                    return;
                }
            }
            else if (gltfFileObjectKey == "instances")
            {
                if (!gltfFiles)
                {
                    LOG_CORE_CRITICAL("ParseGltfFileJSON(...): gltf file '{0}' is not valid", gltfFilename);
                    return;
                }
                ondemand::array instances = gltfFileObject.value();
                int instanceCount = instances.count_elements();
                CORE_ASSERT((instanceCount > 0), "no instances found");

                std::vector<Gltf::Instance>& gltfFileInstances = gltfFiles->back().m_Instances;
                gltfFileInstances.resize(instanceCount);

                uint index = 0;
                for (auto instance : instances)
                {
                    Gltf::Instance& gltfFileInstance = gltfFileInstances[index];
                    if (entity != entt::null)
                    {
                        gltfFileInstance = entity;
                    }
                    ondemand::object instanceObjects = instance.value();
                    for (auto instanceObject : instanceObjects)
                    {
                        std::string_view instanceObjectKey = instanceObject.unescaped_key();

                        if (instanceObjectKey == "transform")
                        {
                            CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::object), "type must be object" );
                            ParseTransformJSON(instanceObject.value(), entity);
                        }
                        else if (instanceObjectKey == "nodes")
                        {
                            CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::array), "type must be object" );
                            ParseNodesJSON(instanceObject.value(), gltfFilename, gltfFileInstance);
                        }
                        else
                        {
                            LOG_CORE_CRITICAL("unrecognized gltf instance object");
                        }
                    }
                    ++index;
                    if ( index == 1) break; // only one instance supported right now
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized gltf file object");
            }
        }
    }

    void SceneLoaderJSON::ParseTransformJSON(ondemand::object transformJSON, entt::entity entity)
    {
        // transform
        TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
        glm::vec3 scale{1.0f};
        glm::vec3 rotation{1.0f};
        glm::vec3 translation{1.0f};

        for (auto transformComponent : transformJSON)
        {
            std::string_view transformComponentKey = transformComponent.unescaped_key();
            if (transformComponentKey == "scale")
            {
                ondemand::array scaleJSON = transformComponent.value();
                scale = ConvertToVec3(scaleJSON);
            }
            else if (transformComponentKey == "rotation")
            {
                ondemand::array rotationJSON = transformComponent.value();
                rotation = ConvertToVec3(rotationJSON);
            }
            else if (transformComponentKey == "translation")
            {
                ondemand::array translationJSON = transformComponent.value();
                translation = ConvertToVec3(translationJSON);
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized transform component");
            }
        }

        transform.SetScale(scale);
        transform.SetRotation(rotation);
        transform.SetTranslation(translation);
    }

    void SceneLoaderJSON::ParseNodesJSON(ondemand::array nodesJSON, std::string const& gltfFilename, Gltf::Instance& gltfFileInstance)
    {
        uint nodeCount = nodesJSON.count_elements();
        if (!nodeCount) return;

        gltfFileInstance.m_Nodes.resize(nodeCount);

        uint index = 0;
        for (auto nodeJSON : nodesJSON)
        {
            CORE_ASSERT((nodeJSON.value().type() == ondemand::json_type::object), "type must be object" );
            ondemand::object nodeObjects = nodeJSON.value();

            Gltf::Node& gltfNode = gltfFileInstance.m_Nodes[index];
            gltfNode.m_WalkSpeed = 0.0;
            gltfNode.m_RigidBody = false;

            for (auto nodeObject : nodeObjects)
            {
                std::string_view nodeObjectKey = nodeObject.unescaped_key();
                if (nodeObjectKey == "name")
                {
                    std::string_view nodeObjectStringView = nodeObject.value().get_string();
                    gltfNode.m_Name = std::string(nodeObjectStringView);
                }
                else if (nodeObjectKey == "walkSpeed")
                {
                    gltfNode.m_WalkSpeed = nodeObject.value().get_double();
                }
                else if (nodeObjectKey == "rigidBody")
                {
                    gltfNode.m_RigidBody = nodeObject.value().get_bool();
                }
                else if (nodeObjectKey == "script-component")
                {
                    std::string_view scriptComponentStringView = nodeObject.value().get_string();
                    gltfNode.m_ScriptComponent = std::string(scriptComponentStringView);

                    std::string fullEntityName = gltfFilename + std::string("::" + gltfNode.m_Name);
                    entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(fullEntityName);
                    LOG_CORE_INFO("found script '{0}' for entity '{1}' in scene description", scriptComponentStringView, fullEntityName);

                    ScriptComponent scriptComponent(scriptComponentStringView);
                    m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
                }
                else
                {
                    LOG_CORE_CRITICAL("unrecognized node component");
                }
            }
            ++index;
        }
    }

    glm::vec3 SceneLoaderJSON::ConvertToVec3(ondemand::array arrayJSON)
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

    void SceneLoaderJSON::PrintType(ondemand::value elementJSON)
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

    void SceneLoaderJSON::Serialize()
    {
        m_OutputFile.open(m_Scene.m_Filepath);
        SerializeScene(0);
        m_OutputFile << "\n";
        m_OutputFile.close();
    }

    void SceneLoaderJSON::SerializeScene(int indent)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeNumber(indent, "file format identifier", SUPPORTED_FILE_FORMAT_VERSION);
        SerializeString(indent, "description", m_SceneDescriptionFile.m_Description);
        SerializeString(indent, "author", m_SceneDescriptionFile.m_Author);
        SerializeGltfFiles(indent);
        m_OutputFile << indentStr << "}";
    }

    void SceneLoaderJSON::SerializeString(int indent, std::string const& key, std::string const& value, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"" << key << "\": \"" << value << "\"" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeBool(int indent, std::string const& key, bool value, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"" << key << "\": " << (value ? "true" : "false") << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeNumber(int indent, std::string const& key, double const value)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"" << key << "\": " << value << ",\n";
    }

    void SceneLoaderJSON::SerializeGltfFiles(int indent)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"gltf files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t gltfFileCount = m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene.size();
        size_t index = 0;
        for (auto& gltfFile : m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene)
        {
            bool noComma = ((index+1) == gltfFileCount);
            SerializeGltfFile(indent, gltfFile, noComma);
            ++index;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeGltfFile(int indent, Gltf::GltfFile const& gltfFile, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeString(indent, "filename", gltfFile.m_Filename);
        SerializeInstances(indent, gltfFile.m_Instances);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeInstances(int indent, std::vector<Gltf::Instance> const& instances)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"instances\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t instanceCount = instances.size();
        size_t index = 0;
        for (auto& instance : instances)
        {
            bool noComma = ((index+1) == instanceCount);
            SerializeInstance(indent, instance, noComma);
            ++index;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeInstance(int indent, Gltf::Instance const& instance, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        if (instance.m_Nodes.size())
        {
            SerializeTransform(indent, instance.m_Entity);
            SerializeNodes(indent, instance.m_Nodes);
        }
        else
        {
            SerializeTransform(indent, instance.m_Entity, NO_COMMA);
        }
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeTransform(int indent, entt::entity const& entity, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"transform\":\n";
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
        glm::vec3 scale = transform.GetScale();
        glm::vec3 rotation = transform.GetRotation();
        glm::vec3 translation = transform.GetTranslation();
        SerializeVec3(indent, "scale", scale);
        SerializeVec3(indent, "rotation", rotation);
        SerializeVec3(indent, "translation", translation, NO_COMMA);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeNodes(int indent, std::vector<Gltf::Node> const& nodes)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"nodes\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t nodeCount = nodes.size();
        size_t index = 0;
        for (auto& node : nodes)
        {
            bool noComma = ((index+1) == nodeCount);
            SerializeNode(indent, node, noComma);
            ++index;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeNode(int indent, Gltf::Node const& node, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeString(indent, "name", node.m_Name);
        SerializeNumber(indent, "walkSpeed", node.m_WalkSpeed);
        if (node.m_ScriptComponent.length())
        {
            SerializeBool(indent, "rigidBody", node.m_RigidBody);
            SerializeString(indent, "script-component", node.m_ScriptComponent, NO_COMMA);
        }
        else
        {
            SerializeBool(indent, "rigidBody", node.m_RigidBody, NO_COMMA);
        }
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeVec3(int indent, std::string name, glm::vec3 const& vec3, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"" << name << "\":\n";
        m_OutputFile << indentStr << "[\n";
        m_OutputFile << indentStr << "    " << vec3.x << ", " << vec3.y << ", " << vec3.z << "\n";
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
    }
}
