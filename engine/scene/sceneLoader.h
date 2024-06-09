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

#pragma once

#include <unordered_map>

#include "engine.h"
#include "scene/gltf.h"
#include "scene/scene.h"
#include "yaml-cpp/yaml.h"

namespace GfxRenderEngine
{
    class SceneLoader
    {

    public:

        SceneLoader(Scene& scene);
        ~SceneLoader() {}

        void Deserialize();
        void Serialize();
        Gltf::GltfFiles& GetGltfFiles() { return m_GltfFiles; }

    private:

        void LoadPrefab(const std::string& filepath);
        glm::vec3 ConvertToVec3(const YAML::Node& node);

    private:

        Scene& m_Scene;
        
        std::vector<std::string> m_PrefabFiles;
        Gltf::GltfFiles m_GltfFiles;

    };
}
