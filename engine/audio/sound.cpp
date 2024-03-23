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

#include <iostream>
#include <stdio.h>

#include "audio/sound.h"

#ifdef PULSEAUDIO

using namespace LibPAmanager;

namespace GfxRenderEngine
{

    SoundDeviceManager* Sound::m_SoundDeviceManager;

    void Sound::Start()
    {
        m_SoundDeviceManager = SoundDeviceManager::GetInstance();
        m_SoundDeviceManager->Start();
    }

    uint Sound::GetDesktopVolume()
    {
        uint desktopVolume = 0;
        desktopVolume = m_SoundDeviceManager->GetVolume();
        return desktopVolume;
    }

    void Sound::SetDesktopVolume(uint desktopVolume) { m_SoundDeviceManager->SetVolume(desktopVolume); }

    std::vector<std::string>& Sound::GetOutputDeviceList() { return m_SoundDeviceManager->GetOutputDeviceList(); }

    void Sound::SetOutputDevice(const std::string& outputDevice) { m_SoundDeviceManager->SetOutputDevice(outputDevice); }

    void Sound::SetCallback(std::function<void(const LibPAmanager::Event&)> callback)
    {
        m_SoundDeviceManager->SetCallback(callback);
    }

    std::string& Sound::GetDefaultOutputDevice() { return m_SoundDeviceManager->GetDefaultOutputDevice(); }
} // namespace GfxRenderEngine

#else

namespace GfxRenderEngine
{
    void Sound::Start() {}

    uint Sound::GetDesktopVolume()
    {
        uint desktopVolume = 0;
        return desktopVolume;
    }

    void Sound::SetDesktopVolume(uint desktopVolume) {}

    std::vector<std::string>& Sound::GetOutputDeviceList()
    {
        static std::vector<std::string> dummy;
        return dummy;
    }

    void Sound::SetOutputDevice(const std::string& outputDevice) {}

    std::string& Sound::GetDefaultOutputDevice()
    {
        static std::string dummy;
        return dummy;
    }
} // namespace GfxRenderEngine

#endif
