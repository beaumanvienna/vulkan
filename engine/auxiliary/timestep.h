/* Engine Copyright (c) 2022 Engine Development Team
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

#include <chrono>

#include "engine.h"

namespace GfxRenderEngine
{
    class Timestep
    {

    public:
        Timestep(std::chrono::duration<float, std::chrono::seconds::period> time);

        std::chrono::duration<float, std::chrono::seconds::period> GetSeconds() const;
        std::chrono::duration<float, std::chrono::milliseconds::period> GetMilliseconds() const;

        void Print() const;
        float Count() const;

        Timestep& operator=(const std::chrono::duration<float, std::chrono::seconds::period>& timestep);
        Timestep& operator-=(const Timestep& other);
        Timestep operator-(const Timestep& other) const;
        bool operator<=(const std::chrono::duration<float, std::chrono::seconds::period>& other) const;

        operator float() const { return m_Timestep.count(); }
        glm::vec3 operator*(const glm::vec3& other) const
        {
            auto ts = m_Timestep.count();
            return glm::vec3(ts * other.x, ts * other.y, ts * other.z);
        }

    private:
        std::chrono::duration<float, std::chrono::seconds::period> m_Timestep;
    };
} // namespace GfxRenderEngine
