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

#include "VKtextureSlotManager.h"

namespace GfxRenderEngine
{
    static constexpr int INITIAL_SLOT = 1;

    VKTextureSlotManager::VKTextureSlotManager()
    {
        uint maxTextureSlots = 1024;
        m_TextureSlots.resize(maxTextureSlots);
        for(auto slot : m_TextureSlots)
        {
            slot = false;
        }
    }

    uint VKTextureSlotManager::GetTextureSlot()
    {
        uint textureSlot = INITIAL_SLOT;
        for(auto slot : m_TextureSlots)
        {
            if(!slot)
            {
                slot = true;
                return textureSlot;
            }
            textureSlot++;
        }
        LOG_APP_CRITICAL("no free texture slot found");
        return INITIAL_SLOT - 1;
    }

    void VKTextureSlotManager::RemoveTextureSlot(uint slot)
    {
        if ((slot >= INITIAL_SLOT) && (slot < m_TextureSlots.size()))
        {
            m_TextureSlots[slot-1] = false;
        }
    }
}
