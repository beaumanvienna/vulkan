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

#include "timestep.h"

namespace GfxRenderEngine
{
    Timestep::Timestep(std::chrono::duration<float, std::chrono::seconds::period> time) : m_Timestep(time) {}

    Timestep& Timestep::operator=(const std::chrono::duration<float, std::chrono::seconds::period>& timestep)
    {
        m_Timestep = timestep;
        return *this;
    }

    Timestep& Timestep::operator-=(const Timestep& other)
    {
        m_Timestep = m_Timestep - other.m_Timestep;
        return *this;
    }
    Timestep Timestep::operator-(const Timestep& other) const { return m_Timestep - other.m_Timestep; }

    bool Timestep::operator<=(const std::chrono::duration<float, std::chrono::seconds::period>& other) const
    {
        return (m_Timestep - other) <= 0ms;
    }

    std::chrono::duration<float, std::chrono::seconds::period> Timestep::GetSeconds() const { return m_Timestep; }

    std::chrono::duration<float, std::chrono::milliseconds::period> Timestep::GetMilliseconds() const
    {
        return (std::chrono::duration<float, std::chrono::milliseconds::period>)m_Timestep;
    }

    void Timestep::Print() const
    {
        auto inMilliSeconds = GetMilliseconds();
        LOG_CORE_INFO("timestep in milli seconds: {0} ms", inMilliSeconds.count());
        auto inSeconds = GetSeconds();
        LOG_CORE_INFO("timestep in seconds: {0} s", inSeconds.count());
    }

    float Timestep::Count() const { return m_Timestep.count(); }
} // namespace GfxRenderEngine
