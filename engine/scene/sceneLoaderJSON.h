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
#include <fstream>
#include "simdjson.h"

using namespace simdjson;

#include "engine.h"
#include "scene/scene.h"
#include "renderer/gltf.h"

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
            double m_FileFormatIdentifier;
            std::string m_Description;
            std::string m_Author;
            Gltf::GltfFiles m_GltfFiles;
            Fbx::FbxFiles m_FbxFiles;
        };

    private:

        void Deserialize(std::string& filepath);

        void ParseGltfFile(ondemand::object gltfFileJSON);
        void ParseFbxFile(ondemand::object fbxFileJSON);
        void ParseObjFile(ondemand::object objFileJSON);
        void ParseTransform(ondemand::object transformJSON, entt::entity entity);
        void ParseNodesGltf(ondemand::array nodesJSON, std::string const& gltfFilename, Gltf::Instance& gltfFileInstance);


        glm::vec3 ConvertToVec3(ondemand::array arrayJSON);

    private:

        static constexpr bool NO_COMMA = true;
        static constexpr int NO_INDENT = 0;

        void SerializeScene(int indent);
        void SerializeString(int indent, std::string const& key, std::string const& value, bool noComma = false);
        void SerializeBool(int indent, std::string const& key, bool value, bool noComma = false);
        void SerializeNumber(int indent, std::string const& key, double const value, bool noComma = false);
        void SerializeGltfFiles(int indent);
        void SerializeGltfFile(int indent, Gltf::GltfFile const& gltfFile, bool noComma);
        void SerializeInstances(int indent, std::vector<Gltf::Instance> const& instances);
        void SerializeInstance(int indent, Gltf::Instance const& instance, bool noComma);
        void SerializeTransform(int indent, entt::entity const& entity, bool noComma = false);
        void SerializeNodes(int indent, std::vector<Gltf::Node> const& nodes);
        void SerializeNode(int indent, Gltf::Node const& node, bool noComma);
        void SerializeVec3(int indent, std::string name, glm::vec3 const& vec3, bool noComma = false);

    private:

        std::ofstream m_OutputFile;
        static constexpr double SUPPORTED_FILE_FORMAT_VERSION = 1.2;

        Scene& m_Scene;

        SceneDescriptionFile m_SceneDescriptionFile;

    };
}
