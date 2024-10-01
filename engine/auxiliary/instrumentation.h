/* Controller Copyright (c) 2021 Controller Development Team
   https://github.com/beaumanvienna/gfxRenderController

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

#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>

namespace GfxRenderEngine
{
// enable PROFILING in premake5.lua
#if defined(PROFILING)

#if (defined(__FUNCSIG__) || defined(_MSC_VER))
#define FUNC_SIGNATURE __FUNCSIG__
#elif defined(__GNUC__)
#define FUNC_SIGNATURE __PRETTY_FUNCTION__
#endif

#define PROFILE_SCOPE_LINE2(name, line) Instrumentation::Timer timer##line(*g_Profiler, name)
#define PROFILE_SCOPE_LINE(name, line) PROFILE_SCOPE_LINE2(name, line)
#define PROFILE_SCOPE(name) PROFILE_SCOPE_LINE(name, __LINE__)
#define PROFILE_FUNCTION() PROFILE_SCOPE(FUNC_SIGNATURE)

    namespace Instrumentation
    {

        struct Result
        {
            std::string m_Name;
            std::chrono::duration<double, std::micro> m_Start;
            std::chrono::microseconds m_ElapsedTime;
            std::thread::id m_ThreadID;
        };

        struct Session
        {
            std::string m_Name;
        };

        class Profiler
        {
        public:
            Profiler(const std::string& name, const std::string& filename = "results.json");
            ~Profiler();
            Profiler(const Profiler&) = delete;
            Profiler(Profiler&&) = delete;

            void CreateEntry(const Result& result);

        private:
            void StartJsonFile();
            void EndJsonFile();

        private:
            std::mutex m_Mutex;
            Session* m_CurrentSession;
            std::ofstream m_OutputStream;
            std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
        };

        class Timer
        {

        public:
            Timer(Profiler& Profiler, const char* name);
            ~Timer();

        private:
            Profiler& m_Profiler;
            const char* m_Name;
            std::chrono::time_point<std::chrono::high_resolution_clock> m_Start, m_End;
        };
    } // namespace Instrumentation

#else
#define PROFILE_BEGIN_SESSION(name, filepath)
#define PROFILE_END_SESSION()
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif
} // namespace GfxRenderEngine
