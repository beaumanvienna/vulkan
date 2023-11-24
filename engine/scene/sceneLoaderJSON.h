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

#pragma once

#include <iostream>
#include "simdjson.h"

using namespace simdjson;

#include "engine.h"
#include "scene/scene.h"
#include "scene/gltf.h"

namespace GfxRenderEngine
{
    class SceneLoaderJSON
    {

    public:

        SceneLoaderJSON(Scene& scene);
        ~SceneLoaderJSON() {}

        void Deserialize(std::string& filepath, std::string& alternativeFilepath);
        void Serialize();
        Gltf::GltfFiles& GetGltfFiles() { return m_SceneDescriptionFile.m_GltfFiles; }

    private:

        struct SceneDescriptionFile
        {
            std::string m_FileFormatIdentifier;
            std::string m_Description;
            std::string m_Author;
            Gltf::GltfFiles m_GltfFiles;
            std::vector<SceneDescriptionFile> m_Prefabs;
        };

    private:

        void ParseGltfFileJSON(ondemand::object gltfFileJSON);
        void ParsePrefabJSON(ondemand::object prefabJSON);
        void ParseTransformJSON(ondemand::object transformJSON, entt::entity entity);
        void ParseNodesJSON(ondemand::array nodesJSON, std::string const& gltfFilename, Gltf::Instance& gltfFileInstance);

        void PrintType(ondemand::value elementJSON);
        glm::vec3 ConvertToVec3(ondemand::array arrayJSON);

    private:

        static constexpr double SUPPORTED_FILE_FORMAT_VERSION = 1.2;

        Scene& m_Scene;
        bool m_LoadPrefab;

        std::vector<std::string> m_PrefabFiles;
        SceneDescriptionFile m_SceneDescriptionFile;

    };
}
