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

#include <iostream>

#include "SDL.h"
#include "platform/SDL/SDLaudio.h"
#include "resources/resources.h"

namespace GfxRenderEngine
{

    void SDLAudio::Start()
    {
        SDL_InitSubSystem(SDL_INIT_AUDIO);

        // Set up the audio stream
        int result = Mix_OpenAudio(44100, AUDIO_S16SYS, SOUND_CHANNELS, 512);
        if (result < 0)
        {
            std::string errorMessage = SDL_GetError();
            LOG_CORE_WARN("Unable to open audio: {0}", errorMessage);
            return;
        }

        result = Mix_AllocateChannels(4);
        if (result < 0)
        {
            std::string errorMessage = SDL_GetError();
            LOG_CORE_WARN("Unable to allocate mixing channels: {0}", errorMessage);
            return;
        }
    }

    void SDLAudio::Stop()
    {
        for (uint i = 0; i < SOUND_CHANNELS; i++)
        {
            Mix_FreeChunk(m_DataBuffer[i]);
        }

        Mix_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    void SDLAudio::PlaySound(const std::string& filename)
    {
        memset(m_DataBuffer, 0, sizeof(Mix_Chunk*) * SOUND_CHANNELS);

        // load sound file from disk
        for (uint i = 0; i < SOUND_CHANNELS; i++)
        {
            m_DataBuffer[i] = Mix_LoadWAV(filename.c_str());
            if (m_DataBuffer[i] == nullptr)
            {
                LOG_CORE_WARN("SDLAudio::PlaySound: Unable to load sound file: {0}, Mix_GetError(): {1}", filename,
                              Mix_GetError());
                return;
            }
        }
        Mix_PlayChannel(-1, m_DataBuffer[0], 0);
    }

    void SDLAudio::PlaySound(const char* path, int resourceID, const std::string& resourceClass)
    {
        memset(m_DataBuffer, 0, sizeof(Mix_Chunk*) * SOUND_CHANNELS);

        // load file from memory
        size_t fileSize;
        void* data = (void*)ResourceSystem::GetDataPointer(fileSize, path, resourceID, resourceClass);

        for (uint i = 0; i < SOUND_CHANNELS; i++)
        {
            SDL_RWops* sdlRWOps = SDL_RWFromMem(data, fileSize);
            if (!sdlRWOps)
            {
                LOG_CORE_WARN("SDLAudio::PlaySound: Resource '{0}' not found", path);
                return;
            }

            m_DataBuffer[i] = Mix_LoadWAV_RW(sdlRWOps, 0);
            if (m_DataBuffer[i] == nullptr)
            {
                LOG_CORE_WARN("SDLAudio::PlaySound: Unable to load sound file: {0}, Mix_GetError(): {1}", path,
                              Mix_GetError());
                return;
            }
        }
        Mix_PlayChannel(-1, m_DataBuffer[0], 0);
    }
} // namespace GfxRenderEngine
