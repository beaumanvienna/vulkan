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

#include "entt.hpp"

#include "engine.h"

namespace GfxRenderEngine
{
    namespace Terrain
    {

        struct Instance
        {
            entt::entity m_Entity;

            Instance() = default;
            Instance(entt::entity entity) : m_Entity{entity} {}
        };

        struct GrassParameters
        {
            int m_Width;
            int m_Height; // not used,
            float m_ScaleXZ;
            float m_ScaleY;
        };

        struct GrassShaderData
        {
            int m_Height;
            int m_Index;
        };
        struct TerrainDescription
        {
            std::string m_Filename;
            std::vector<Instance> m_Instances;

            TerrainDescription(std::string const& filename) : m_Filename{filename} {}
        };

        struct GrassSpec
        {
            std::string m_FilepathGrassModel;
            std::string m_FilepathGrassHeightMap;
            std::string m_FilepathDensityMap;
            glm::vec3 m_Translation{};
            glm::vec3 m_Rotation{};
            glm::vec3 m_Scale{};
            float m_ScaleXZ{1.0f};
            float m_ScaleY{1.0f};
        };

        struct TerrainSpec
        {
            std::string m_FilepathTerrainDescription;
            std::string m_FilepathHeightMap;
            std::string m_FilepathColorMap;
            Material::PbrMaterial m_PbrMaterial{};
            GrassSpec m_GrassSpec;
        };

    } // namespace Terrain
} // namespace GfxRenderEngine