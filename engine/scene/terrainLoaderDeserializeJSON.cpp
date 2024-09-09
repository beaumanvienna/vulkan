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

    bool TerrainLoaderJSON::Deserialize(std::string filepath, int instanceCount)
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

        Terrain::TerrainSpec& terrainSpec = m_TerrainDescriptionFile.m_TerrainSpec;
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
                terrainSpec.m_FilepathHeightMap = std::string(heightMapPath);
                LOG_CORE_INFO("Heightmap Path: {0}", terrainSpec.m_FilepathHeightMap);
            }
            else if (terrainAttributesKey == "colorMapPath")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::string),
                            "colormap path must be string");
                std::string_view colorMapPath = terrainAttributes.value().get_string();
                terrainSpec.m_FilepathColorMap = std::string(colorMapPath);
                LOG_CORE_INFO("Colormap Path: {0}", terrainSpec.m_FilepathColorMap);
            }
            else if (terrainAttributesKey == "material")
            {
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::object), "type must be object");
                Material::PbrMaterial& pbrMaterial = terrainSpec.m_PbrMaterial;
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
                CORE_ASSERT((terrainAttributes.value().type() == ondemand::json_type::object),
                            "grass 3D model path must be object");
                ondemand::object grassSpec = terrainAttributes.value().get_object();
                ParseGrassSpecification(grassSpec);
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized terrain object '" + std::string(terrainAttributesKey) + "'");
            }
        }

        TerrainBuilder builder{};
        return builder.LoadTerrain(m_Scene, instanceCount, terrainSpec);
    }

    void TerrainLoaderJSON::ParseGrassSpecification(ondemand::object grassSpecification)
    {
        Terrain::TerrainSpec& terrainSpec = m_TerrainDescriptionFile.m_TerrainSpec;
        Terrain::GrassSpec& grassSpec = terrainSpec.m_GrassSpec;

        for (auto grassAttribute : grassSpecification)
        {
            std::string_view grassAttributeKey = grassAttribute.unescaped_key();

            if (grassAttributeKey == "modelPath")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::string),
                            "grass model filepath must be string");
                std::string_view grassModelFilenameStringView = grassAttribute.value().get_string();
                std::string& filepath = grassSpec.m_FilepathGrassModel;
                filepath = std::string(grassModelFilenameStringView);
                if (EngineCore::FileExists(filepath))
                {
                    LOG_CORE_INFO("grass model found {0}", filepath);
                }
                else
                {
                    LOG_CORE_ERROR("grass model not found: {0}", filepath);
                    return;
                }
            }
            else if (grassAttributeKey == "heightMapPath")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::string),
                            "grass model filepath must be string");
                std::string_view heightmapFilenameStringView = grassAttribute.value().get_string();
                std::string& filepath = grassSpec.m_FilepathGrassHeightMap;
                filepath = std::string(heightmapFilenameStringView);
                if (EngineCore::FileExists(filepath))
                {
                    LOG_CORE_INFO("heightmap for grass found {0}", filepath);
                }
                else
                {
                    LOG_CORE_ERROR("heightmap for grass not found: {0}", filepath);
                    return;
                }
            }
            else if (grassAttributeKey == "denistyMapPath")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::string),
                            "grass model filepath must be string");
                std::string_view densityMapFilenameStringView = grassAttribute.value().get_string();
                std::string& filepath = grassSpec.m_FilepathDensityMap;
                filepath = std::string(densityMapFilenameStringView);
                if (EngineCore::FileExists(filepath))
                {
                    LOG_CORE_INFO("density map for grass found {0}", filepath);
                }
                else
                {
                    LOG_CORE_ERROR("density map for grass not found: {0}", filepath);
                    return;
                }
            }
            else if (grassAttributeKey == "transform")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::object), "transform must be object");
                ondemand::object transformJSON = grassAttribute.value().get_object();
                ParseTransform(transformJSON);
            }
            else if (grassAttributeKey == "scaleXZ")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::number), "type must be number");
                grassSpec.m_ScaleXZ = grassAttribute.value().get_double();
            }
            else if (grassAttributeKey == "scaleY")
            {
                CORE_ASSERT((grassAttribute.value().type() == ondemand::json_type::number), "type must be number");
                grassSpec.m_ScaleY = grassAttribute.value().get_double();
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized grass attribute");
            }
        }
    }

    void TerrainLoaderJSON::ParseTransform(ondemand::object transformJSON)
    {
        Terrain::TerrainSpec& terrainSpec = m_TerrainDescriptionFile.m_TerrainSpec;
        Terrain::GrassSpec& grassSpec = terrainSpec.m_GrassSpec;

        glm::vec3 scale{1.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 translation{0.0f};

        for (auto transformComponent : transformJSON)
        {
            std::string_view transformComponentKey = transformComponent.unescaped_key();
            if (transformComponentKey == "scale")
            {
                ondemand::array scaleJSON = transformComponent.value();
                scale = ConvertToVec3(scaleJSON);
            }
            else if (transformComponentKey == "rotation")
            {
                ondemand::array rotationJSON = transformComponent.value();
                rotation = ConvertToVec3(rotationJSON);
            }
            else if (transformComponentKey == "translation")
            {
                ondemand::array translationJSON = transformComponent.value();
                translation = ConvertToVec3(translationJSON);
            }
            else
            {
                LOG_CORE_CRITICAL("unrecognized transform component");
            }
        }

        grassSpec.m_Scale = scale;
        grassSpec.m_Rotation = rotation;
        grassSpec.m_Translation = translation;
    }

    glm::vec3 TerrainLoaderJSON::ConvertToVec3(ondemand::array arrayJSON)
    {
        glm::vec3 returnVec3{0.0f};
        uint componentIndex = 0;
        for (auto component : arrayJSON)
        {
            switch (componentIndex)
            {
                case 0:
                {
                    returnVec3.x = component.get_double();
                    break;
                }
                case 1:
                {
                    returnVec3.y = component.get_double();
                    break;
                }
                case 2:
                {
                    returnVec3.z = component.get_double();
                    break;
                }
                default:
                {
                    LOG_CORE_ERROR("JSON::CConvertToVec3(...) argument must have 3 components");
                    break;
                }
            }
            ++componentIndex;
        }
        return returnVec3;
    }

} // namespace GfxRenderEngine
