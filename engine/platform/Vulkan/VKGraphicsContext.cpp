/* Engine Copyright (c) 2022 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include <chrono>
#include <thread>

#include "VKGraphicsContext.h"
#include "core.h"

float gCPUtimePerFrame;

VKContext::VKContext(GLFWwindow* window, uint refreshRate)
    : m_Window(window), m_RefreshRate(refreshRate), m_Initialized(false)
{
    m_MicroSecondsPerFrame = 1e6 / m_RefreshRate;
}

bool VKContext::Init()
{
    m_Initialized = false;
    //char description[1024];
    //
    ////clear error
    //GLFWClearError();
    //
    //// create context
    //glfwMakeContextCurrent(m_Window);
    //
    //// get error
    //int errorCode = glfwGetError((const char**)(&description));
    //
    //// test error code
    //if ( errorCode != GLFW_NO_ERROR)
    //{
    //    LOG_CORE_CRITICAL("could not create window context, error code {0}", errorCode);
    //}
    //else
    //{
    //    m_Initialized = true;
    //}
    //m_StartTime = glfwGetTime();
    
    return m_Initialized;
}

void VKContext::SetVSync(int interval)
{ 
    //m_VSync = interval;
    //// set the number of screen updates to wait from the time glfwSwapBuffers 
    //// was called before swapping the buffers
    //GLFWCall(glfwSwapInterval(m_VSync)); // wait for next screen update
}

void SwapBuffers()
{
    //Engine::m_Engine->OnUpdate();
    //Engine::m_Engine->SwapBuffers();
}

void VKContext::SwapBuffers()
{

    //uint  diffTime = static_cast<int>( (glfwGetTime() - m_StartTime) * 1e6 );
    //gCPUtimePerFrame = static_cast<float>(diffTime) / 1000.0f;
    //int  sleepTime = m_MicroSecondsPerFrame - diffTime - 333;
    //if (sleepTime < 0) sleepTime = 0;
    //
    //// here ends the frame
    //GLFWCall(glfwSwapBuffers(m_Window));
    //if (m_VSyncIsWorking)
    //{
    //    diffTime = static_cast<int>( (glfwGetTime() - m_StartTime) * 1e6 );
    //    if (diffTime < (m_MicroSecondsPerFrame/2)) 
    //    {
    //        // time difference too short
    //        // glfwSwapBuffers is not blocking
    //        m_VSyncIsWorking--;
    //    }
    //
    //    if (!m_VSyncIsWorking)
    //    {
    //        //LOG_CORE_CRITICAL("GLFW VSync is buggy, switching to usleep()");
    //    }
    //}
    //else
    //{
    //    std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
    //}
    //// here starts the new frame
    //m_StartTime = glfwGetTime(); 
}
