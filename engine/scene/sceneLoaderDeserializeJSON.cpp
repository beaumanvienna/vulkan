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

#include "core.h"
#include "auxiliary/file.h"
#include "scene/sceneLoaderJSON.h"
#include "scene/terrainLoaderJSON.h"

namespace GfxRenderEngine
{

    SceneLoaderJSON::SceneLoaderJSON(Scene& scene) : m_Scene(scene) {}

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
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::number), "type must be number");

                // only check major version of "file format identifier"
                m_SceneDescriptionFile.m_FileFormatIdentifier = sceneObject.value().get_double();
                CORE_ASSERT(
                    (std::trunc(m_SceneDescriptionFile.m_FileFormatIdentifier) == std::trunc(SUPPORTED_FILE_FORMAT_VERSION)),
                    "The scene description major version does not match");
            }
            else if (sceneObjectKey == "terrain")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                ondemand::array terrainDescriptions = sceneObject.value().get_array();
                {
                    int count = terrainDescriptions.count_elements();
                    if (count == 1)
                    {
                        LOG_CORE_INFO("loading 1 terrain");
                    }
                    else
                    {
                        LOG_CORE_INFO("loading {0} terrain descriptions", count);
                    }
                }

                m_TerrainInfos.resize(terrainDescriptions.count_elements());
                uint terrainCounter = 0;
                for (auto terrainDescription : terrainDescriptions)
                {
                    ParseTerrainDescription(terrainDescription, m_SceneDescriptionFile.m_TerrainDescriptions,
                                            m_TerrainInfos[terrainCounter]);
                    ++terrainCounter;
                }
            }

            else if (sceneObjectKey == "description")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view sceneDescription = sceneObject.value().get_string();
                m_SceneDescriptionFile.m_Description = std::string(sceneDescription);
                LOG_CORE_INFO("description: {0}", m_SceneDescriptionFile.m_Description);
            }
            else if (sceneObjectKey == "author")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view sceneAuthor = sceneObject.value().get_string();
                m_SceneDescriptionFile.m_Author = std::string(sceneAuthor);
                LOG_CORE_INFO("author: {0}", m_SceneDescriptionFile.m_Author);
            }
            else if (sceneObjectKey == "gltf files")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                ondemand::array gltfFiles = sceneObject.value().get_array();
                {
                    int gltfFileCount = gltfFiles.count_elements();
                    if (gltfFileCount == 1)
                    {
                        LOG_CORE_INFO("loading 1 gltf file (tinygltf)");
                    }
                    else
                    {
                        LOG_CORE_INFO("loading {0} gltf files (tinygltf)", gltfFileCount);
                    }
                }

                std::vector<GltfInfo> gltfInfoVector;
                gltfInfoVector.resize(gltfFiles.count_elements());
                {
                    uint fileCount{0};
                    for (auto gltfFileJSON : gltfFiles)
                    {
                        ParseGltfFile(gltfFileJSON, false /*tinygltf loader*/, gltfInfoVector[fileCount]);
                        ++fileCount;
                    }
                }

                {
                    std::vector<Gltf::GltfFile>& gltfFilesFromScene =
                        m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene;
                    for (auto& gltfInfo : gltfInfoVector)
                    {
                        if (!gltfInfo.m_LoadFuture.has_value())
                        {
                            // file was not loaded (probably not found on disk)
                            continue;
                        }
                        auto& loadFuture = gltfInfo.m_LoadFuture.value();
                        if (!loadFuture.get())
                        {
                            LOG_CORE_CRITICAL("gltf file did not load properly: {0}", gltfInfo.m_GltfFile.m_Filename);
                            continue;
                        }
                        gltfFilesFromScene.push_back(gltfInfo.m_GltfFile);

                        std::vector<Gltf::Instance>& gltfFileInstances = gltfFilesFromScene.back().m_Instances;
                        gltfFileInstances.resize(gltfInfo.m_InstanceCount);
                        uint instanceIndex = 0;
                        for (auto& gltfFileInstance : gltfFileInstances)
                        {
                            gltfFileInstance = gltfInfo.m_GltfFile.m_Instances[instanceIndex];

                            { // transform
                                std::string fullEntityName = std::string("SL::") + gltfInfo.m_GltfFile.m_Filename +
                                                             std::string("::" + std::to_string(instanceIndex) + "::root");
                                entt::entity entity = m_Scene.m_Dictionary.Retrieve(fullEntityName);
                                CORE_ASSERT(entity != entt::null, "couldn't find entity");
                                gltfFileInstance.m_Entity = entity;
                                TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                                transform.SetScale(gltfInfo.m_InstanceTransforms[instanceIndex].GetScale());
                                transform.SetRotation(gltfInfo.m_InstanceTransforms[instanceIndex].GetRotation());
                                transform.SetTranslation(gltfInfo.m_InstanceTransforms[instanceIndex].GetTranslation());
                            }

                            for (auto& gltfNode : gltfFileInstance.m_Nodes)
                            {
                                // script component
                                if (!gltfNode.m_ScriptComponent.empty())
                                {
                                    std::string fullEntityName = std::string("SL::") + gltfInfo.m_GltfFile.m_Filename +
                                                                 "::" + std::to_string(instanceIndex) +
                                                                 "::" + gltfNode.m_Name;
                                    entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(fullEntityName);

                                    if (gameObject != entt::null)
                                    {
                                        LOG_CORE_INFO("found script '{0}' for entity '{1}' in scene description",
                                                      gltfNode.m_ScriptComponent, fullEntityName);
                                        ScriptComponent scriptComponent(gltfNode.m_ScriptComponent);
                                        m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
                                    }
                                    else
                                    {
                                        LOG_CORE_WARN("could not find script '{0}' for entity '{1}' in scene description",
                                                      gltfNode.m_ScriptComponent, fullEntityName);
                                    }
                                }
                            }

                            ++instanceIndex;
                        }
                    }
                }
            }
            else if (sceneObjectKey == "fastgltf files")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                ondemand::array gltfFiles = sceneObject.value().get_array();
                int gltfFileCount = gltfFiles.count_elements();
                if (gltfFileCount == 1)
                {
                    LOG_CORE_INFO("loading 1 gltf file (fastgltf)");
                }
                else
                {
                    LOG_CORE_INFO("loading {0} gltf files (fastgltf)", gltfFileCount);
                }

                std::vector<GltfInfo> gltfInfoVector{};
                gltfInfoVector.resize(gltfFileCount);

                {
                    uint fileCount{0};
                    for (auto gltfFileJSON : gltfFiles)
                    {
                        ParseGltfFile(gltfFileJSON, true /*fast*/, gltfInfoVector[fileCount]);
                        ++fileCount;
                    }
                }

                {
                    std::vector<Gltf::GltfFile>& gltfFilesFromScene =
                        m_SceneDescriptionFile.m_FastgltfFiles.m_GltfFilesFromScene;

                    for (auto& gltfInfo : gltfInfoVector)
                    {
                        if (!gltfInfo.m_LoadFuture.has_value())
                        {
                            // file was not loaded (probably not found on disk)
                            continue;
                        }
                        auto& loadFuture = gltfInfo.m_LoadFuture.value();
                        if (!loadFuture.get())
                        {
                            LOG_CORE_CRITICAL("gltf file did not load properly: {0}", gltfInfo.m_GltfFile.m_Filename);
                            continue;
                        }
                        gltfFilesFromScene.push_back(gltfInfo.m_GltfFile);

                        std::vector<Gltf::Instance>& gltfFileInstances = gltfFilesFromScene.back().m_Instances;
                        gltfFileInstances.resize(gltfInfo.m_InstanceCount);
                        uint instanceIndex = 0;
                        for (auto& gltfFileInstance : gltfFileInstances)
                        {
                            gltfFileInstance = gltfInfo.m_GltfFile.m_Instances[instanceIndex];

                            { // transform
                                std::string fullEntityName = std::string("SL::") + gltfInfo.m_GltfFile.m_Filename +
                                                             std::string("::" + std::to_string(instanceIndex) + "::root");
                                entt::entity entity = m_Scene.m_Dictionary.Retrieve(fullEntityName);

                                gltfFileInstance.m_Entity = entity;
                                TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                                transform.SetScale(gltfInfo.m_InstanceTransforms[instanceIndex].GetScale());
                                transform.SetRotation(gltfInfo.m_InstanceTransforms[instanceIndex].GetRotation());
                                transform.SetTranslation(gltfInfo.m_InstanceTransforms[instanceIndex].GetTranslation());
                            }

                            for (auto& gltfNode : gltfFileInstance.m_Nodes)
                            {
                                // script component
                                if (!gltfNode.m_ScriptComponent.empty())
                                {
                                    std::string fullEntityName = std::string("SL::") + gltfInfo.m_GltfFile.m_Filename +
                                                                 "::" + std::to_string(instanceIndex) +
                                                                 "::" + gltfNode.m_Name;
                                    entt::entity gameObject = m_Scene.m_Dictionary.Retrieve(fullEntityName);
                                    if (gameObject != entt::null)
                                    {
                                        LOG_CORE_INFO("found script '{0}' for entity '{1}' in scene description",
                                                      gltfNode.m_ScriptComponent, fullEntityName);
                                        ScriptComponent scriptComponent(gltfNode.m_ScriptComponent);
                                        m_Scene.m_Registry.emplace<ScriptComponent>(gameObject, scriptComponent);
                                    }
                                    else
                                    {
                                        LOG_CORE_WARN("could not find script '{0}' for entity '{1}' in scene description",
                                                      gltfNode.m_ScriptComponent, fullEntityName);
                                    }
                                }
                            }

                            ++instanceIndex;
                        }
                    }
                }
            }
            else if (sceneObjectKey == "fbx files")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                ondemand::array fbxFiles = sceneObject.value().get_array();
                {
                    int fbxFileCount = fbxFiles.count_elements();
                    if (fbxFileCount == 1)
                    {
                        LOG_CORE_INFO("loading 1 fbx file (asset importer)");
                    }
                    else
                    {
                        LOG_CORE_INFO("loading {0} fbx files (asset importer)", fbxFileCount);
                    }
                }

                for (auto fbxFileJSON : fbxFiles)
                {
                    ParseFbxFile(fbxFileJSON, false /*asset importer*/);
                }
            }
            else if (sceneObjectKey == "ufbx files")
            {
                CORE_ASSERT((sceneObject.value().type() == ondemand::json_type::array), "type must be array");
                ondemand::array fbxFiles = sceneObject.value().get_array();
                {
                    int fbxFileCount = fbxFiles.count_elements();
                    if (fbxFileCount == 1)
                    {
                        LOG_CORE_INFO("loading 1 fbx file (ufbx)");
                    }
                    else
                    {
                        LOG_CORE_INFO("loading {0} fbx files (ufbx)", fbxFileCount);
                    }
                }

                for (auto fbxFileJSON : fbxFiles)
                {
                    ParseFbxFile(fbxFileJSON, true /*ufbx*/);
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized scene object '" + std::string(sceneObjectKey) + "'");
            }
        }
        FinalizeTerrainDescriptions();
    }

    void SceneLoaderJSON::ParseGltfFile(ondemand::object gltfFileJSON, bool fast, SceneLoaderJSON::GltfInfo& gltfInfo)
    {
        std::string gltfFilename;
        int sceneID = Gltf::GLTF_NOT_USED;
        bool instanceFieldFound = false;

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
                }
                else
                {
                    LOG_CORE_ERROR("gltf file not found: {0}", gltfFilename);
                    return;
                }
            }
            else if (gltfFileObjectKey == "sceneID")
            {
                if (instanceFieldFound)
                {
                    LOG_CORE_ERROR("place sceneID before instances in the scene description file");
                }
                sceneID = static_cast<int>(gltfFileObject.value().get_uint64());
            }
            else if (gltfFileObjectKey == "instances")
            {
                // get array of gltf file instances
                ondemand::array instances = gltfFileObject.value();
                int instanceCount = instances.count_elements();
                if (!instanceCount)
                {
                    LOG_CORE_ERROR("no instances found (json file broken): {0}", gltfFilename);
                    return;
                }
                gltfInfo.m_InstanceCount = instanceCount;
                instanceFieldFound = true;

                if (fast)
                {
                    auto loadGltf = [this, gltfFilename, instanceCount, sceneID]()
                    {
                        FastgltfBuilder builder(gltfFilename, m_Scene);
                        builder.SetDictionaryPrefix("SL"); // scene loader
                        return builder.Load(instanceCount, sceneID);
                    };
                    gltfInfo.m_LoadFuture = Engine::m_Engine->m_PoolPrimary.SubmitTask(loadGltf);
                }
                else
                {
                    auto loadGltf = [this, gltfFilename, instanceCount, sceneID]()
                    {
                        GltfBuilder builder(gltfFilename, m_Scene);
                        builder.SetDictionaryPrefix("SL"); // scene loader
                        return builder.Load(instanceCount, sceneID);
                    };
                    gltfInfo.m_LoadFuture = Engine::m_Engine->m_PoolPrimary.SubmitTask(loadGltf);
                }

                gltfInfo.m_GltfFile = Gltf::GltfFile{gltfFilename};
                {
                    uint instanceIndex = 0;
                    gltfInfo.m_InstanceTransforms.resize(instances.count_elements());
                    gltfInfo.m_GltfFile.m_Instances.resize(instances.count_elements());
                    for (auto instance : instances)
                    {
                        Gltf::Instance& gltfFileInstance = gltfInfo.m_GltfFile.m_Instances[instanceIndex];
                        ondemand::object instanceObjects = instance.value();
                        for (auto instanceObject : instanceObjects)
                        {
                            std::string_view instanceObjectKey = instanceObject.unescaped_key();

                            if (instanceObjectKey == "transform")
                            {
                                CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::object),
                                            "type must be object");
                                ParseTransform(instanceObject.value(), gltfInfo.m_InstanceTransforms[instanceIndex]);
                            }
                            else if (instanceObjectKey == "nodes")
                            {
                                CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::array),
                                            "type must be object");
                                ParseNodesGltf(instanceObject.value(), gltfFilename, gltfFileInstance);
                            }
                            else
                            {
                                LOG_CORE_CRITICAL("unrecognized gltf instance object");
                            }
                        }
                        ++instanceIndex;
                    }
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized gltf file object");
            }
        }
    }

    void SceneLoaderJSON::ParseFbxFile(ondemand::object fbxFileJSON, bool ufbx)
    {
        std::string fbxFilename;

        std::vector<Fbx::FbxFile>* fbxFilesFromScene = ufbx ? &m_SceneDescriptionFile.m_UFbxFiles.m_FbxFilesFromScene
                                                            : &m_SceneDescriptionFile.m_FbxFiles.m_FbxFilesFromScene;

        bool loadSuccessful = false;

        for (auto fbxFileObject : fbxFileJSON)
        {
            std::string_view fbxFileObjectKey = fbxFileObject.unescaped_key();

            if (fbxFileObjectKey == "filename")
            {
                std::string_view fbxFilenameStringView = fbxFileObject.value().get_string();
                fbxFilename = std::string(fbxFilenameStringView);
                if (EngineCore::FileExists(fbxFilename))
                {
                    LOG_CORE_INFO("Scene loader found {0}", fbxFilename);
                }
                else
                {
                    LOG_CORE_ERROR("fbx file not found: {0}", fbxFilename);
                    return;
                }
            }
            else if (fbxFileObjectKey == "instances")
            {
                // get array of fbx file instances
                ondemand::array instances = fbxFileObject.value();
                int instanceCount = instances.count_elements();
                if (ufbx)
                {
                    UFbxBuilder builder(fbxFilename, m_Scene);
                    builder.SetDictionaryPrefix("SL"); // scene loader
                    loadSuccessful = builder.Load(instanceCount);
                }
                else
                {
                    FbxBuilder builder(fbxFilename, m_Scene);
                    builder.SetDictionaryPrefix("SL"); // scene loader
                    loadSuccessful = builder.Load(instanceCount);
                }
                if (loadSuccessful)
                {
                    Fbx::FbxFile fbxFile(fbxFilename);
                    fbxFilesFromScene[0].push_back(fbxFile);
                }
                else
                {
                    LOG_CORE_ERROR("fbx file did not load properly: {0}", fbxFilename);
                    return;
                }

                if (!instanceCount)
                {
                    LOG_CORE_ERROR("no instances found (json file broken): {0}", fbxFilename);
                    return;
                }

                std::vector<Fbx::Instance>& fbxFileInstances = fbxFilesFromScene[0].back().m_Instances;
                fbxFileInstances.resize(instanceCount);

                {
                    uint instanceIndex = 0;
                    for (auto instance : instances)
                    {
                        std::string fullEntityName =
                            std::string("SL::") + fbxFilename + std::string("::" + std::to_string(instanceIndex) + "::root");
                        entt::entity entity = m_Scene.m_Dictionary.Retrieve(fullEntityName);
                        Fbx::Instance& fbxFileInstance = fbxFileInstances[instanceIndex];
                        fbxFileInstance.m_Entity = entity;
                        ondemand::object instanceObjects = instance.value();
                        for (auto instanceObject : instanceObjects)
                        {
                            std::string_view instanceObjectKey = instanceObject.unescaped_key();

                            if (instanceObjectKey == "transform")
                            {
                                CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::object),
                                            "type must be object");
                                TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                                ParseTransform(instanceObject.value(), transform);
                            }
                            else
                            {
                                LOG_CORE_CRITICAL("unrecognized fbx instance object");
                            }
                        }
                        ++instanceIndex;
                    }
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized fbx file object");
            }
        }
    }

    void SceneLoaderJSON::ParseTransform(ondemand::object transformJSON, TransformComponent& transform)
    {
        // transform
        glm::vec3 scale{1.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 translation{0.0f};

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

    void SceneLoaderJSON::ParseNodesGltf(ondemand::array nodesJSON, std::string const& gltfFilename,
                                         Gltf::Instance& gltfFileInstance)
    {
        uint nodeCount = nodesJSON.count_elements();
        if (!nodeCount)
            return;

        gltfFileInstance.m_Nodes.resize(nodeCount);

        uint nodeIndex = 0;
        for (auto nodeJSON : nodesJSON)
        {
            CORE_ASSERT((nodeJSON.value().type() == ondemand::json_type::object), "type must be object");
            ondemand::object nodeObjects = nodeJSON.value();

            Gltf::Node& gltfNode = gltfFileInstance.m_Nodes[nodeIndex];
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
                }
                else
                {
                    LOG_CORE_CRITICAL("unrecognized node component");
                }
            }
            ++nodeIndex;
        }
    }

    glm::vec3 SceneLoaderJSON::ConvertToVec3(ondemand::array arrayJSON)
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
                    LOG_CORE_ERROR("JSON::CConvertToVec3(...) argument must have 3 components");
                    break;
                }
            }
            ++componentIndex;
        }
        return returnVec3;
    }

    void SceneLoaderJSON::ParseTerrainDescription(ondemand::object terrainDescription,
                                                  std::vector<Terrain::TerrainDescription>& terrainDescriptions,
                                                  TerrainInfo& terrainInfo)
    {
        std::string filename;

        for (auto terrainDescriptionObject : terrainDescription)
        {
            std::string_view terrainDescriptionObjectKey = terrainDescriptionObject.unescaped_key();

            if (terrainDescriptionObjectKey == "filename")
            {
                std::string_view filenameStringView = terrainDescriptionObject.value().get_string();
                filename = std::string(filenameStringView);
                if (EngineCore::FileExists(filename))
                {
                    LOG_CORE_INFO("Scene loader found {0}", filename);
                }
                else
                {
                    LOG_CORE_CRITICAL("terrain description not found: {0}", filename);
                    return;
                }
            }
            else if (terrainDescriptionObjectKey == "instances")
            {
                // get array of terrain instances
                ondemand::array instances = terrainDescriptionObject.value();
                int instanceCount = instances.count_elements();

                if (!instanceCount)
                {
                    LOG_CORE_ERROR("no instances found (json file broken): {0}", filename);
                    return;
                }

                auto loadTerrain = [this, filename, instanceCount]()
                {
                    TerrainLoaderJSON terrainLoaderJSON(m_Scene);
                    return terrainLoaderJSON.Deserialize(filename, instanceCount);
                };

                terrainInfo.m_LoadFuture = Engine::m_Engine->m_PoolPrimary.SubmitTask(loadTerrain);
                terrainInfo.m_Filename = filename;
                terrainInfo.m_InstanceCount = instanceCount;
                terrainInfo.m_InstanceTransforms.resize(instanceCount);

                {
                    uint instanceIndex = 0;
                    for (auto instance : instances)
                    {
                        ondemand::object instanceObjects = instance.value();
                        for (auto instanceObject : instanceObjects)
                        {
                            std::string_view instanceObjectKey = instanceObject.unescaped_key();

                            if (instanceObjectKey == "transform")
                            {
                                CORE_ASSERT((instanceObject.value().type() == ondemand::json_type::object),
                                            "type must be object");
                                TransformComponent& transform = terrainInfo.m_InstanceTransforms[instanceIndex];
                                ParseTransform(instanceObject.value(), transform);
                            }
                            else
                            {
                                LOG_CORE_CRITICAL("unrecognized terrain instance object");
                            }
                        }
                        ++instanceIndex;
                    }
                }
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized terrain description object");
            }
        }
    }

    std::vector<Terrain::TerrainDescription>& SceneLoaderJSON::GetTerrainDescriptions()
    {
        return m_SceneDescriptionFile.m_TerrainDescriptions;
    }

    void SceneLoaderJSON::FinalizeTerrainDescriptions()
    {
        for (auto& terrainInfo : m_TerrainInfos)
        {
            if (!terrainInfo.m_LoadFuture.has_value())
            {
                // file was not loaded (probably not found on disk)
                continue;
            }
            auto& loadFuture = terrainInfo.m_LoadFuture.value();
            if (!loadFuture.get())
            {
                continue;
            }
            Terrain::TerrainDescription terrainDescriptionScene(terrainInfo.m_Filename);
            m_SceneDescriptionFile.m_TerrainDescriptions.push_back(terrainDescriptionScene);

            std::vector<Terrain::Instance>& terrainInstances =
                m_SceneDescriptionFile.m_TerrainDescriptions.back().m_Instances;
            terrainInstances.resize(terrainInfo.m_InstanceCount);

            {
                uint instanceIndex = 0;
                for (auto& terrainInstance : terrainInstances)
                {
                    std::string name = terrainInfo.m_Filename + std::string("::") + std::to_string(instanceIndex);
                    entt::entity entity = m_Scene.m_Dictionary.Retrieve(name);
                    CORE_ASSERT(entity != entt::null, "couldn't find entity");
                    terrainInstance.m_Entity = entity;
                    TransformComponent& transform = m_Scene.m_Registry.get<TransformComponent>(entity);
                    transform.SetScale(terrainInfo.m_InstanceTransforms[instanceIndex].GetScale());
                    transform.SetRotation(terrainInfo.m_InstanceTransforms[instanceIndex].GetRotation());
                    transform.SetTranslation(terrainInfo.m_InstanceTransforms[instanceIndex].GetTranslation());

                    ++instanceIndex;
                }
            }
        }
    }
} // namespace GfxRenderEngine
