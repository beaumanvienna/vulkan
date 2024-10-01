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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#include <vector>

#include "log/log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace GfxRenderEngine
{

    Log::Log()
    {
        std::vector<spdlog::sink_ptr> logSink;
        logSink.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        spdlog::set_pattern("%^[%T] %n: %v%$");
        m_Logger = std::make_shared<spdlog::logger>("Engine", begin(logSink), end(logSink));
        if (m_Logger)
        {
            spdlog::register_logger(m_Logger);
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->flush_on(spdlog::level::trace);
        }

        m_AppLogger = std::make_shared<spdlog::logger>("Application", begin(logSink), end(logSink));
        if (m_AppLogger)
        {
            spdlog::register_logger(m_AppLogger);
            m_Logger->set_level(spdlog::level::trace);
            m_Logger->flush_on(spdlog::level::trace);
        }
    }
} // namespace GfxRenderEngine
