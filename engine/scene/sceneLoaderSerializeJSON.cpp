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

namespace GfxRenderEngine
{

    void SceneLoaderJSON::Serialize()
    {
        m_OutputFile.open(m_Scene.m_Filepath);
        SerializeScene(NO_INDENT);
        m_OutputFile.close();
    }

    void SceneLoaderJSON::SerializeScene(int indent)
    {
        size_t gltfFileCount = m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene.size();
        size_t fastgltfFileCount = m_SceneDescriptionFile.m_FastgltfFiles.m_GltfFilesFromScene.size();
        size_t fbxFileCount = m_SceneDescriptionFile.m_FbxFiles.m_FbxFilesFromScene.size();
        size_t ufbxFileCount = m_SceneDescriptionFile.m_UFbxFiles.m_FbxFilesFromScene.size();
        size_t objFileCount = m_SceneDescriptionFile.m_ObjFiles.m_ObjFilesFromScene.size();

        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeNumber(indent, "file format identifier", SUPPORTED_FILE_FORMAT_VERSION);
        SerializeString(indent, "description", m_SceneDescriptionFile.m_Description);
        SerializeString(indent, "author", m_SceneDescriptionFile.m_Author);

        if (fastgltfFileCount) // terrain
        {
            bool noComma = (fastgltfFileCount == 0) && (gltfFileCount == 0) && (fbxFileCount == 0) && (ufbxFileCount == 0) &&
                           (objFileCount == 0);
            SerializeTerrrainDescriptions(indent, noComma);
        }
        if (fastgltfFileCount) // fastgltf
        {
            bool noComma = (gltfFileCount == 0) && (fbxFileCount == 0) && (ufbxFileCount == 0) && (objFileCount == 0);
            SerializeFastgltfFiles(indent, noComma);
        }
        if (gltfFileCount) // gltf
        {
            bool noComma = (fbxFileCount == 0) && (ufbxFileCount == 0) && (objFileCount == 0);
            SerializeGltfFiles(indent, noComma);
        }
        if (fbxFileCount) // fbx
        {
            bool noComma = (ufbxFileCount == 0) && (objFileCount == 0);
            SerializeFbxFiles(indent, noComma);
        }
        if (ufbxFileCount) // ufbx
        {
            bool noComma = (objFileCount == 0);
            SerializeUFbxFiles(indent, noComma);
        }
        if (objFileCount) // obj
        {
            SerializeObjFiles(indent);
        }

        m_OutputFile << indentStr << "}\n";
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

    void SceneLoaderJSON::SerializeNumber(int indent, std::string const& key, double const value, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"" << key << "\": " << value << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeGltfFiles(int indent, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"gltf files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t gltfFileCount = m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene.size();
        size_t gltfFileIndex = 0;
        for (auto& gltfFile : m_SceneDescriptionFile.m_GltfFiles.m_GltfFilesFromScene)
        {
            bool noCommaLocal = ((gltfFileIndex + 1) == gltfFileCount);
            SerializeGltfFile(indent, gltfFile, noCommaLocal);
            ++gltfFileIndex;
        }
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeFastgltfFiles(int indent, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"fastgltf files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t gltfFileCount = m_SceneDescriptionFile.m_FastgltfFiles.m_GltfFilesFromScene.size();
        size_t gltfFileIndex = 0;
        for (auto& gltfFile : m_SceneDescriptionFile.m_FastgltfFiles.m_GltfFilesFromScene)
        {
            bool noCommaLocal = ((gltfFileIndex + 1) == gltfFileCount);
            SerializeGltfFile(indent, gltfFile, noCommaLocal);
            ++gltfFileIndex;
        }
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
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
        size_t instanceIndex = 0;
        for (auto& instance : instances)
        {
            bool noComma = ((instanceIndex + 1) == instanceCount);
            SerializeInstance(indent, instance, noComma);
            ++instanceIndex;
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
        size_t nodeIndex = 0;
        for (auto& node : nodes)
        {
            bool noComma = ((nodeIndex + 1) == nodeCount);
            SerializeNode(indent, node, noComma);
            ++nodeIndex;
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

    void SceneLoaderJSON::SerializeFbxFiles(int indent, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"fbx files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t fbxFileCount = m_SceneDescriptionFile.m_FbxFiles.m_FbxFilesFromScene.size();
        size_t fbxFileIndex = 0;
        for (auto& fbxFile : m_SceneDescriptionFile.m_FbxFiles.m_FbxFilesFromScene)
        {
            bool noCommaLocal = ((fbxFileIndex + 1) == fbxFileCount);
            SerializeFbxFile(indent, fbxFile, noCommaLocal);
            ++fbxFileIndex;
        }
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeUFbxFiles(int indent, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"ufbx files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t fbxFileCount = m_SceneDescriptionFile.m_UFbxFiles.m_FbxFilesFromScene.size();
        size_t fbxFileIndex = 0;
        for (auto& fbxFile : m_SceneDescriptionFile.m_UFbxFiles.m_FbxFilesFromScene)
        {
            bool noCommaLocal = ((fbxFileIndex + 1) == fbxFileCount);
            SerializeFbxFile(indent, fbxFile, noCommaLocal);
            ++fbxFileIndex;
        }
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeFbxFile(int indent, Fbx::FbxFile const& fbxFile, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeString(indent, "filename", fbxFile.m_Filename);
        SerializeInstances(indent, fbxFile.m_Instances);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeInstances(int indent, std::vector<Fbx::Instance> const& instances)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"instances\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t instanceCount = instances.size();
        size_t instanceIndex = 0;
        for (auto& instance : instances)
        {
            bool noComma = ((instanceIndex + 1) == instanceCount);
            SerializeInstance(indent, instance, noComma);
            ++instanceIndex;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeInstance(int indent, Fbx::Instance const& instance, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeTransform(indent, instance.m_Entity, NO_COMMA);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeObjFiles(int indent)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"obj files\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t objFileCount = m_SceneDescriptionFile.m_ObjFiles.m_ObjFilesFromScene.size();
        size_t objFileIndex = 0;
        for (auto& objFile : m_SceneDescriptionFile.m_ObjFiles.m_ObjFilesFromScene)
        {
            bool noComma = ((objFileIndex + 1) == objFileCount);
            SerializeObjFile(indent, objFile, noComma);
            ++objFileIndex;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeObjFile(int indent, Obj::ObjFile const& objFile, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeString(indent, "filename", objFile.m_Filename);
        SerializeInstances(indent, objFile.m_Instances);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeInstances(int indent, std::vector<Obj::Instance> const& instances)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"instances\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t instanceCount = instances.size();
        size_t instanceIndex = 0;
        for (auto& instance : instances)
        {
            bool noCommaLocal = ((instanceIndex + 1) == instanceCount);
            SerializeInstance(indent, instance, noCommaLocal);
            ++instanceIndex;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeInstance(int indent, Obj::Instance const& instance, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeTransform(indent, instance.m_Entity, NO_COMMA);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeTerrrainDescriptions(int indent, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"terrain\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t terrainDescriptionCount = m_SceneDescriptionFile.m_TerrainDescriptions.size();
        size_t terrainDescriptionIndex = 0;
        for (auto& terrainDescription : m_SceneDescriptionFile.m_TerrainDescriptions)
        {
            bool noCommaLocal = ((terrainDescriptionIndex + 1) == terrainDescriptionCount);
            SerializeTerrrainDescription(indent, terrainDescription, noCommaLocal);
            ++terrainDescriptionIndex;
        }
        m_OutputFile << indentStr << "]" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeTerrrainDescription(int indent, Terrain::TerrainDescription const& terrainDescription,
                                                       bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeString(indent, "filename", terrainDescription.m_Filename);
        SerializeInstances(indent, terrainDescription.m_Instances);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

    void SceneLoaderJSON::SerializeInstances(int indent, std::vector<Terrain::Instance> const& instances)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "\"instances\":\n";
        m_OutputFile << indentStr << "[\n";
        indent += 4;
        size_t instanceCount = instances.size();
        size_t instanceIndex = 0;
        for (auto& instance : instances)
        {
            bool noComma = ((instanceIndex + 1) == instanceCount);
            SerializeInstance(indent, instance, noComma);
            ++instanceIndex;
        }
        m_OutputFile << indentStr << "]\n";
    }

    void SceneLoaderJSON::SerializeInstance(int indent, Terrain::Instance const& instance, bool noComma)
    {
        std::string indentStr(indent, ' ');
        m_OutputFile << indentStr << "{\n";
        indent += 4;
        SerializeTransform(indent, instance.m_Entity, NO_COMMA);
        m_OutputFile << indentStr << "}" << (noComma ? "" : ",") << "\n";
    }

} // namespace GfxRenderEngine
