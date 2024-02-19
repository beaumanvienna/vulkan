#pragma once

#include <string>
#include <vector>

namespace GfxRenderEngine
{
    class BaseTerrain
    {
    public:
        BaseTerrain(const std::string &filepath);
        std::vector<std::vector<float>> terrainData;

    private:
        std::string filepath;
        void readFile(const std::string &filepath);
        uint32_t terrainSize = 0;
    };

}