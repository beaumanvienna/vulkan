#include "baseTerrain.hpp"

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

    BaseTerrain::BaseTerrain(const std::string &filepath)
    {
        readFile(filepath);
    };
    void BaseTerrain::readFile(const std::string &filepath)
    {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        size_t floatCount = fileSize / sizeof(float);

        std::vector<float> buffer(floatCount);
        file.seekg(0);
        file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
        file.close();

        terrainSize = static_cast<size_t>(sqrt(floatCount));
        terrainData.resize(terrainSize, std::vector<float>(terrainSize));

        std::cout << sqrt(buffer.size()) << std::endl;
        terrainSize = sqrt(buffer.size());
        for (size_t i = 0; i < terrainSize; ++i)
        {
            for (size_t j = 0; j < terrainSize; ++j)
            {
                terrainData[i][j] = buffer[i * terrainSize + j];
            }
        }
        std::cout << "terrain Size: " << terrainData.size() << std::endl;
    }

}
