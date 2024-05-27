/* Engine Copyright (c) 2024 Engine Development Team
   https://github.com/beaumanvienna/vulkan

   Author: Mantar (https://github.com/AhmetErenLacinbala)

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

#include "baseTerrain.h"
#include "auxiliary/file.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <cstddef>
#include <cmath>

namespace GfxRenderEngine
{

    BaseTerrain::BaseTerrain(std::string const& filepath) { ReadFile(filepath); }

    void BaseTerrain::ReadFile(std::string const& filepath)
    {
        if (!EngineCore::FileExists(filepath) && !EngineCore::IsDirectory(filepath))
        {
            LOG_CORE_CRITICAL("BaseTerrain::ReadFile file not found");
            return;
        }

        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + m_Filepath);
            return;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        size_t floatCount = fileSize / sizeof(float);

        std::vector<float> buffer(floatCount);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        m_TerrainSize = static_cast<size_t>(std::sqrt(floatCount));
        m_TerrainData.resize(m_TerrainSize, std::vector<float>(m_TerrainSize));

        m_TerrainSize = std::sqrt(buffer.size());
        for (size_t i = 0; i < m_TerrainSize; ++i)
        {
            for (size_t j = 0; j < m_TerrainSize; ++j)
            {
                m_TerrainData[i][j] = buffer[i * m_TerrainSize + j];
            }
        }
        LOG_CORE_INFO("terrain Size: {0}", m_TerrainData.size());
    }

} // namespace GfxRenderEngine
