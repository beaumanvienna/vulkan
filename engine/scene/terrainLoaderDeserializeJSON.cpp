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

#include "auxiliary/file.h"
#include "core.h"
#include "scene/terrainLoaderJSON.h"

namespace GfxRenderEngine
{

    TerrainLoaderJSON::TerrainLoaderJSON(Scene& scene) : m_Scene(scene) {}

    bool TerrainLoaderJSON::Deserialize(std::string& filepath, int instanceCount)
    {
        if (!EngineCore::FileExists(filepath))
        {
            LOG_CORE_CRITICAL("TerrainLoaderJSON: could not find file {0}", filepath);
            return false;
        }

        LOG_CORE_INFO("TerrainLoaderJSON: loading {0}", filepath);

        ondemand::parser parser;
        padded_string json = padded_string::load(filepath);

        ondemand::document terrainDocument = parser.iterate(json);
        ondemand::object terrainAttributess = terrainDocument.get_object();

        Terrain::TerrainSpec terrainSpec{};
        terrainSpec.m_FilepathTerrainDescription = filepath;

        for (auto terrainAttributes : terrainAttributess)
        {
            std::string_view terrainAttributesKey = terrainAttributes.unescaped_key();

            if (terrainAttributesKey == "file format identifier")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::number), "type must be number");

                // only check major version of "file format identifier"
                m_TerrainDescriptionFile.m_FileFormatIdentifier = terrainAttributes.value().get_double();
                CORE_ASSERT((std::trunc(m_TerrainDescriptionFile.m_FileFormatIdentifier) ==
                             std::trunc(SUPPORTED_FILE_FORMAT_VERSION)),
                            "The terrain description major version does not match");
            }
            else if (terrainAttributesKey == "description")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view terrainDescription = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_Description = std::string(terrainDescription);
                LOG_CORE_INFO("description: {0}", m_TerrainDescriptionFile.m_Description);
            }
            else if (terrainAttributesKey == "author")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string), "type must be string");
                std::string_view terrainAuthor = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_Author = std::string(terrainAuthor);
                LOG_CORE_INFO("author: {0}", m_TerrainDescriptionFile.m_Author);
            }
            else if (terrainAttributesKey == "heightMapPath")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string),
                            "heightmap path must be string");
                std::string_view heightMapPath = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_FilepathHeightMap = std::string(heightMapPath);
                terrainSpec.m_FilepathHeightMap = m_TerrainDescriptionFile.m_FilepathHeightMap;
                LOG_CORE_INFO("Heightmap Path: {0}", m_TerrainDescriptionFile.m_FilepathHeightMap);
            }
            else if (terrainAttributesKey == "grassHeightMapPath")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string),
                            "grass heightmap path must be string");
                std::string_view grassHeightMapPath = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_FilepathGrassHeightMap = std::string(grassHeightMapPath);
                terrainSpec.m_FilepathGrassHeightMap = m_TerrainDescriptionFile.m_FilepathGrassHeightMap;
                LOG_CORE_INFO("Heightmap Path: {0}", m_TerrainDescriptionFile.m_FilepathGrassHeightMap);
            }
            else if (terrainAttributesKey == "colorMapPath")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string),
                            "colormap path must be string");
                std::string_view colorMapPath = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_FilepathColorMap = std::string(colorMapPath);
                terrainSpec.m_FilepathColorMap = m_TerrainDescriptionFile.m_FilepathColorMap;
                LOG_CORE_INFO("Colormap Path: {0}", m_TerrainDescriptionFile.m_FilepathColorMap);
            }
            else if (terrainAttributesKey == "material")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::object), "type must be object");
                Material::PbrMaterial& pbrMaterial = m_TerrainDescriptionFile.m_PbrMaterial;
                ondemand::object materialJSON = terrainAttributes.value();
                for (auto materialComponent : materialJSON)
                {
                    std::string_view materialComponentKey = materialComponent.unescaped_key();
                    if (materialComponentKey == "roughness")
                    {
                        pbrMaterial.m_Roughness = materialComponent.value().get_double();
                        terrainSpec.m_PbrMaterial.m_Roughness = pbrMaterial.m_Roughness;
                    }
                    else if (materialComponentKey == "metallic")
                    {
                        pbrMaterial.m_Metallic = materialComponent.value().get_double();
                        terrainSpec.m_PbrMaterial.m_Metallic = pbrMaterial.m_Metallic;
                    }
                    else
                    {
                        LOG_CORE_CRITICAL("unrecognized material property '" + std::string(materialComponentKey) + "'");
                    }
                }
            }
            else if (terrainAttributesKey == "grass")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string),
                            "grass 3D model path must be string");
                std::string_view grassModelPath = terrainAttributes.value().get_string();
                m_TerrainDescriptionFile.m_FilepathGrassModel = std::string(grassModelPath);
                terrainSpec.m_FilepathGrassModel = m_TerrainDescriptionFile.m_FilepathGrassModel;
                LOG_CORE_INFO("Grass Model Path: {0}", m_TerrainDescriptionFile.m_FilepathGrassModel);
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized terrain object '" + std::string(terrainAttributesKey) + "'");
            }
        }

        TerrainBuilder builder{};
        return builder.LoadTerrain(m_Scene, instanceCount, terrainSpec);
    }

} // namespace GfxRenderEngine
