/* Engine Copyright (c) 2023 Engine Development Team
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

#include <memory>
#include <iostream>
#include <unordered_map>
#include <limits>

#include "gtc/type_ptr.hpp"
#include "stb_image.h"

#include "core.h"
#include "renderer/model.h"
#include "auxiliary/hash.h"
#include "auxiliary/file.h"
#include "auxiliary/math.h"
#include "scene/scene.h"

namespace std
{
    template <> struct hash<GfxRenderEngine::Vertex>
    {
        size_t operator()(GfxRenderEngine::Vertex const& vertex) const
        {
            size_t seed = 0;
            GfxRenderEngine::HashCombine(seed, vertex.m_Position, vertex.m_Color, vertex.m_Normal, vertex.m_UV);
            return seed;
        }
    };
} // namespace std

namespace GfxRenderEngine
{

    float Model::m_NormalMapIntensity = 1.0f;

    SkeletalAnimations& Model::GetAnimations() { return *(m_Animations.get()); }
} // namespace GfxRenderEngine
