
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

#include "core.h"
#include "renderer/image.h"
#include "renderer/model.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/terrainBuilderMultiMaterial.h"
#include "auxiliary/file.h"
#include "scene/scene.h"

namespace GfxRenderEngine
{
    bool TerrainBuilderMultiMaterial::LoadMesh(Scene& scene, int instanceCount, Terrain::TerrainSpec const& terrainSpec)
    {
        bool meshFound = !terrainSpec.m_FilepathMesh.empty() && // 3D model for terrain provided?
                         EngineCore::FileExists(terrainSpec.m_FilepathMesh) &&
                         !EngineCore::IsDirectory(terrainSpec.m_FilepathMesh);
        if (!meshFound)
        {
            return false;
        }

        auto material = std::make_shared<PbrMultiMaterial>();

        FastgltfBuilder fastgltfBuilder(terrainSpec.m_FilepathMesh, scene, material);
        fastgltfBuilder.SetDictionaryPrefix("TLMM"); // terrain loader multi material
        fastgltfBuilder.Load(instanceCount);

        // create material descriptor
        material->m_MaterialDescriptor =                                   //
            MaterialDescriptor::Create(Material::MaterialType::MtPbrMulti, //
                                       material->m_PbrMultiMaterialTextures);

        return true;
    }

    bool TerrainBuilderMultiMaterial::LoadTerrain(Scene& scene, int instanceCount, Terrain::TerrainSpec const& terrainSpec)
    {
        ZoneScopedNC("TerrainBuilderMultiMaterial::LoadTerrain", 0xFF0000);

        if (LoadMesh(scene, instanceCount, terrainSpec))
        {
            return true;
        }

        return false;
    }

} // namespace GfxRenderEngine
