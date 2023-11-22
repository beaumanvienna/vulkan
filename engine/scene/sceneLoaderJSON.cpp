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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>
#include "simdjson.h"

#include "core.h"
#include "auxiliary/file.h"
#include "scene/sceneLoaderJSON.h"

namespace GfxRenderEngine
{
    
    using namespace simdjson;

    SceneLoaderJSON::SceneLoaderJSON(Scene& scene)
        : m_Scene(scene)
    {
    }

    void SceneLoaderJSON::Deserialize()
    {
        std::string filepath;
        if (EngineCore::FileExists(m_Scene.m_Filepath))
        {
            filepath = m_Scene.m_Filepath;
            LOG_CORE_INFO("Loading scene {0}", m_Scene.m_Filepath);
        }
        else if (EngineCore::FileExists(m_Scene.m_AlternativeFilepath))
        {
            filepath = m_Scene.m_AlternativeFilepath;
            LOG_CORE_INFO("Loading scene {0}", m_Scene.m_AlternativeFilepath);
        }
        else
        {
            LOG_CORE_CRITICAL("Scene loader could neither find file {0} nor file {1}", m_Scene.m_Filepath, m_Scene.m_AlternativeFilepath);
            return;
        }

        ondemand::parser parser;
        padded_string json = padded_string::load(filepath);
        ondemand::document doc = parser.iterate(json);
        ondemand::object object = doc.get_object();

        for(auto field : object)
        {
            // parses and writes out the key, after unescaping it,
            // to a string buffer. It causes a performance penalty.
            std::string_view keyv = field.unescaped_key();
            LOG_CORE_INFO("keyv = {0}", keyv);
            if (keyv == "gltf files")
            {
                
            }
        }
        LOG_CORE_CRITICAL("about to crash ...");
        exit(0);
    }

    glm::vec3 SceneLoaderJSON::ConvertToVec3()
    {
        return glm::vec3{0.0f};
    }

    void SceneLoaderJSON::LoadPrefab(const std::string& filepath)
    {
    }

    void SceneLoaderJSON::Serialize()
    {
        auto& filepath = m_Scene.m_Filepath;
    }
}
