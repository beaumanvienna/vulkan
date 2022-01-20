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

#pragma once

#include <memory>
#include <string>

#include "event.h"
#include "engine.h"
#include "window.h"
#include "audio.h"

class Engine
{
public:


public:

    Engine(const std::string& configFilePath);
    ~Engine();

    bool Start();
    void OnUpdate();
    void OnEvent(Event& event);
    void OnRender();
    void Shutdown();
    void Quit();

    bool IsRunning() const { return m_Running; }
    std::string GetConfigFilePath() const { return m_ConfigFilePath; }
    std::shared_ptr<Window> GetWindow() const { return m_Window; }
    double GetTime() const { return m_Window->GetTime(); }

    static Engine* m_Engine;

private:
    static void SignalHandler(int signal);
    void ToggleFullscreen();

private:
    
    std::string m_ConfigFilePath;
    std::shared_ptr<Window> m_Window;
    std::shared_ptr<Audio> m_Audio;

    bool m_Running;

};
