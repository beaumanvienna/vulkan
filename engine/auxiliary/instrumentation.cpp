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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#if defined(PROFILING)

    #include <sstream>

    #include "core.h"
    #include "engine.h"
    #include "auxiliary/instrumentation.h"
    #include "auxiliary/file.h"

    namespace GfxRenderEngine
    {
        namespace Instrumentation
        {
            Timer::Timer(const char* name)
                : m_Name(name)
            {
                m_Start = std::chrono::high_resolution_clock::now();
            }

            Timer::~Timer()
            {
                m_End = std::chrono::high_resolution_clock::now();

                auto start = std::chrono::duration<double, std::micro>{ m_Start.time_since_epoch() };
                auto elapsedMicroSeconds = std::chrono::time_point_cast<std::chrono::microseconds>(m_End).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_Start).time_since_epoch();
                SessionManager::Get().CreateEntry({ m_Name, start, elapsedMicroSeconds, std::this_thread::get_id() });
            }

            SessionManager::SessionManager()
                : m_CurrentSession(nullptr)
            {}

            SessionManager::~SessionManager()
            {
                End();
            }

            void SessionManager::Begin(const std::string& name, const std::string& filename)
            {
                std::lock_guard lock(m_Mutex);
                if (m_CurrentSession)
                {
                    EndInternal();
                }
                m_StartTime =  std::chrono::steady_clock::now();

                //this function must be called
                //after the constructor of engine 
                //and before engine.Start()
                std::string filepath, homeDir;
                #ifdef _MSC_VER
                    homeDir = "";
                #else
                    homeDir = getenv("HOME");
                    EngineCore::AddSlash(homeDir);
                #endif
                if (Engine::m_Engine)
                {
                    filepath = homeDir + Engine::m_Engine->GetConfigFilePath() + filename;
                }
                else
                {
                    filepath = filename;
                }
                m_OutputStream.open(filepath);

                if (m_OutputStream.is_open())
                {
                    m_CurrentSession = new Session({name});
                    StartJsonFile();
                }
                else
                {
                    // plain cout because logger might not yet be available
                    std::cout << "SessionManager could not open output file '" << filepath << "'" << std::endl;
                }
            }

            void SessionManager::End()
            {
                std::lock_guard lock(m_Mutex);
                EndInternal();
            }

            void SessionManager::CreateEntry(const Result& result)
            {
                if ((std::chrono::steady_clock::now() - m_StartTime) > 5min)
                {
                    return;
                }
                std::stringstream outputFile;

                outputFile << std::setprecision(3) << std::fixed;
                outputFile << ",\n    {";
                outputFile << "\"cat\":\"function\",";
                outputFile << "\"dur\":" << result.m_ElapsedTime.count() << ',';
                outputFile << "\"name\":\"" << result.m_Name << "\",";
                outputFile << "\"ph\":\"X\",";
                outputFile << "\"pid\":0,";
                outputFile << "\"tid\":" << result.m_ThreadID << ",";
                outputFile << "\"ts\":" << result.m_Start.count();
                outputFile << "}";

                std::lock_guard lock(m_Mutex);
                if (m_CurrentSession)
                {
                    m_OutputStream << outputFile.str();
                    m_OutputStream.flush();
                }
            }

            void SessionManager::StartJsonFile()
            {
                m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
                m_OutputStream.flush();
            }

            void SessionManager::EndJsonFile()
            {
                m_OutputStream << "]}";
                m_OutputStream.flush();
            }

            void SessionManager::EndInternal()
            {
                if (m_CurrentSession)
                {
                    EndJsonFile();
                    m_OutputStream.close();
                    delete m_CurrentSession;
                    m_CurrentSession = nullptr;
                }
            }
        }
    }
#endif
