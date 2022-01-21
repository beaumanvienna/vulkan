/* Engine Copyright (c) 2021 Engine Development Team 
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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
    
   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#pragma once

#include <iostream>
#include <map>

#include "engine.h"
#include "yaml-cpp/yaml.h"

namespace GfxRenderEngine
{

    class SettingsManager
    {
    public:
    
        SettingsManager();
    
        void SetFilepath(const std::string& filepath) { m_Filepath = filepath; }
    
        void SaveToFile();
        void SaveToFile(const std::string& filepath);
    
        bool LoadFromFile();
        bool LoadFromFile(const std::string& filepath);
        bool SettingsLoadedFromFile() const { return m_SettingsLoadedFromFile; }
        
        void ApplySettings();
        void PrintSettings() const;
        
        template <typename T>
        void PushSetting(std::string key, T* value); 
    
    private:
    
        enum class ElementType
        {
            TYPE_INT,
            TYPE_BOOL,
            TYPE_STRING,
            TYPE_RENDERERAPI_API
        };
    
        struct ListElement
        {
            ElementType m_Type;
            void* m_Pointer;
        };
    
    private:
    
        std::string m_Filepath;
        bool m_SettingsLoadedFromFile;
        YAML::Node m_YAMLData;
        std::map<std::string, ListElement> m_Settings;
    
    };
}
