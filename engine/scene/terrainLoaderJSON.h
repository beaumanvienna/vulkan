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

#pragma once

#include "simdjson.h"
#include <fstream>
#include <iostream>

using namespace simdjson;

#include "engine.h"
#include "scene/scene.h"
#include "scene/terrain.h"

namespace GfxRenderEngine
{
    class TerrainLoaderJSON
    {

    public:
        TerrainLoaderJSON(Scene& scene);
        ~TerrainLoaderJSON() {}

        bool Deserialize(std::string filepath, int instanceCount);

    private:
        struct TerrainDescriptionFile
        {
            // JSON file attributes here
            double m_FileFormatIdentifier;
            std::string m_Description;
            std::string m_Author;
            Terrain::TerrainSpec m_TerrainSpec{};
        };

        void ParseGrassSpecification(ondemand::object grassSpecification);
        void ParseTransform(ondemand::object transformJSON);
        glm::vec3 ConvertToVec3(ondemand::array arrayJSON);

    private:
        static constexpr double SUPPORTED_FILE_FORMAT_VERSION = 1.2;

        Scene& m_Scene;

        TerrainDescriptionFile m_TerrainDescriptionFile;
    };
} // namespace GfxRenderEngine
