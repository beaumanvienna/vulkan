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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#include <fstream>

#include "auxiliary/file.h"
#include "settings/settings.h"
#include "renderer/rendererAPI.h"

namespace GfxRenderEngine
{

    SettingsManager::SettingsManager() : m_Filepath("engine.cfg") {}

    void SettingsManager::SaveToFile(const std::string& filepath)
    {
        YAML::Emitter out;

        out << YAML::BeginMap;

        for (const auto& [key, value] : m_Settings)
        {
            switch (value.m_Type)
            {
                case ElementType::TYPE_INT:
                    out << YAML::Key << key << YAML::Value << *((int*)value.m_Pointer);
                    break;
                case ElementType::TYPE_BOOL:
                    out << YAML::Key << key << YAML::Value << *((bool*)value.m_Pointer);
                    break;
                case ElementType::TYPE_STRING:
                    out << YAML::Key << key << YAML::Value << *((std::string*)value.m_Pointer);
                    break;
                case ElementType::TYPE_RENDERERAPI_API:
                    out << YAML::Key << key << YAML::Value << *((RendererAPI::API*)value.m_Pointer);
                    break;
            }
        }

        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SettingsManager::SaveToFile() { SaveToFile(m_Filepath); }

    bool SettingsManager::LoadFromFile(const std::string& filepath)
    {
        m_SettingsLoadedFromFile = false;

        if (EngineCore::FileExists(filepath))
        {
            m_SettingsLoadedFromFile = true;
            m_YAMLNode = YAML::LoadFile(filepath);
            ApplySettings();
        }

        return m_SettingsLoadedFromFile;
    }

    bool SettingsManager::LoadFromFile() { return LoadFromFile(m_Filepath); }

    void SettingsManager::ApplySettings()
    {
        if (m_SettingsLoadedFromFile)
        {
            for (const auto& [key, value] : m_Settings)
            {
                auto entry = m_YAMLNode[key];
                if (entry)
                {
                    switch (value.m_Type)
                    {
                        case ElementType::TYPE_INT:
                            *((int*)value.m_Pointer) = m_YAMLNode[key].as<int>();
                            break;
                        case ElementType::TYPE_BOOL:
                            *((bool*)value.m_Pointer) = m_YAMLNode[key].as<bool>();
                            break;
                        case ElementType::TYPE_STRING:
                            *((std::string*)value.m_Pointer) = m_YAMLNode[key].as<std::string>();
                            break;
                        case ElementType::TYPE_RENDERERAPI_API:
                            *((RendererAPI::API*)value.m_Pointer) = (RendererAPI::API)m_YAMLNode[key].as<int>();
                            break;
                    }
                }
            }
        }
    }

    void SettingsManager::PrintSettings() const
    {
        for (const auto& [key, value] : m_Settings)
        {
            switch (value.m_Type)
            {
                case ElementType::TYPE_INT:
                    LOG_CORE_INFO("SettingsManager: key '{0}', value is {1}", key, *((int*)value.m_Pointer));
                    break;
                case ElementType::TYPE_BOOL:
                    LOG_CORE_INFO("SettingsManager: key '{0}', value is {1}", key, *((bool*)value.m_Pointer));
                    break;
                case ElementType::TYPE_STRING:
                    LOG_CORE_INFO("SettingsManager: key '{0}', value is {1}", key, *((std::string*)value.m_Pointer));
                    break;
                case ElementType::TYPE_RENDERERAPI_API:
                    LOG_CORE_INFO("SettingsManager: key '{0}', value is {1}", key, *((RendererAPI::API*)value.m_Pointer));
                    break;
            }
        }
    }

    template <> void SettingsManager::PushSetting<int>(std::string key, int* value)
    {
        ListElement listElement{ElementType::TYPE_INT, value};
        m_Settings.insert(std::make_pair(key, listElement));
    }

    template <> void SettingsManager::PushSetting<bool>(std::string key, bool* value)
    {
        ListElement listElement{ElementType::TYPE_BOOL, value};
        m_Settings.insert(std::make_pair(key, listElement));
    }

    template <> void SettingsManager::PushSetting<std::string>(std::string key, std::string* value)
    {
        ListElement listElement{ElementType::TYPE_STRING, value};
        m_Settings.insert(std::make_pair(key, listElement));
    }

    template <> void SettingsManager::PushSetting<RendererAPI::API>(std::string key, RendererAPI::API* value)
    {
        ListElement listElement{ElementType::TYPE_RENDERERAPI_API, value};
        m_Settings.insert(std::make_pair(key, listElement));
    }
} // namespace GfxRenderEngine
namespace YAML
{
    template <> struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
            {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <> struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

} // namespace YAML

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}
