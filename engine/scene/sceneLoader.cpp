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

#include "auxiliary/file.h"
#include "scene/sceneLoader.h"

namespace GfxRenderEngine
{
    SceneLoader::SceneLoader(Scene& scene)
        : m_Scene(scene)
    {
    }

    void SceneLoader::Deserialize()
    {
        YAML::Node yamlNode;
        auto& filepath = m_Scene.m_Filepath;

        if (EngineCore::FileExists(filepath))
        {
            LOG_CORE_WARN("Scene loader found {0}", filepath);
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
                if (EngineCore::FileExists(gltfFile.as<std::string>()))
                {
                    LOG_CORE_WARN("Scene loader found {0}", gltfFile.as<std::string>());
                    Builder builder{gltfFile.as<std::string>()};
                    builder.LoadGLTF(m_Scene.m_Registry, m_Scene.m_SceneHierarchy, m_Scene.m_Dictionary);
                }
                else
                {
                    LOG_CORE_CRITICAL("Scene loader could not find file {0}", gltfFile.as<std::string>());
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

    }

    void SceneLoader::LoadPrefab(const std::string& filepath)
    {
        YAML::Node yamlNode;

        if (EngineCore::FileExists(filepath))
        {
            LOG_CORE_WARN("Scene loader found {0}", filepath);
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
                if (EngineCore::FileExists(gltfFile.as<std::string>()))
                {
                    LOG_CORE_WARN("Scene loader found {0}", gltfFile.as<std::string>());
                    Builder builder{gltfFile.as<std::string>()};
                    builder.LoadGLTF(m_Scene.m_Registry, m_Scene.m_SceneHierarchy, m_Scene.m_Dictionary);
                }
                else
                {
                    LOG_CORE_CRITICAL("Scene loader could not find file {0}", gltfFile.as<std::string>());
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

    }

    void SceneLoader::Serialize()
    {
    }
}
