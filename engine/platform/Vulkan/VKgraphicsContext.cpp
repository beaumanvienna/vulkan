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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#include "VKcore.h"
#include "VKgraphicsContext.h"

namespace GfxRenderEngine
{
    VK_Context::VK_Context(VK_Window* window)
        : m_Window{window}
    {
        Init();
    }

    bool VK_Context::Init()
    {
        m_Renderer = std::make_shared<VK_Renderer>(m_Window, VK_Core::m_Device);
        m_Initialized = true;
        return m_Initialized;
    }

    void VK_Context::SetVSync(int interval)
    {
    }

    void VK_Context::SwapBuffers()
    {
    }

    std::shared_ptr<Model> VK_Context::LoadModel(const Builder& builder)
    {
        ASSERT(VK_Core::m_Device != nullptr);
        auto model = std::make_shared<VK_Model>(VK_Core::m_Device, builder);
        return model;
    }

}