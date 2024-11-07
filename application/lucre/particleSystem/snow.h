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

#include "engine.h"
#include "scene/scene.h"
#include "scene/components.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    class Snow
    {

    public:
        Snow(Scene& scene, std::string const& jsonFile);
        void OnUpdate(Timestep timestep, TransformComponent& cameraTransform);

    private:
        struct SysDescription
        {
            std::optional<uint> m_PoolSize{0};
            std::optional<std::string> m_Model;
            std::optional<std::string> m_DictionaryPrefix;
            std::optional<glm::vec3> m_Vertex1; // 0,0,0 coordinate for cubic snow volume
            std::optional<glm::vec3> m_Vertex2; // 1,1,1 coordinate for cubic snow volume
        };

        struct Particle
        {
            glm::vec3 m_Velocity;
            glm::vec3 m_RotationSpeed;
            TransformComponent* m_Transform;
        };

    private:
        static constexpr double SUPPORTED_FILE_FORMAT_VERSION = 1.2;
        void ParseSysDescription(std::string const& jsonFile);

    private:
        static constexpr bool NO_SCENE_GRAPH = false;
        Scene& m_Scene;
        Registry& m_Registry;
        Dictionary& m_Dictionary;
        SysDescription m_SysDescription;
        bool m_Initialized{false};
        uint m_PoolIndex{0};
        std::vector<Particle> m_ParticlePool;
    };
} // namespace GfxRenderEngine
