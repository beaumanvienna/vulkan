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

//#pragma once
//
//#include <memory>
//
//#include "engine.h"
//#include "renderer/rendererAPI.h"
//
//class RenderCommand
//{
//public:
//
//    static void SetClearColor(const glm::vec4& color)
//    {
//        s_RendererAPI->SetClearColor(color);
//    }
//    
//    static void Clear()
//    {
//        s_RendererAPI->Clear();
//    }
//    
//    static void EnableBlending()
//    {
//        s_RendererAPI->EnableBlending();
//    }
//    
//    static void DisableBlending()
//    {
//        s_RendererAPI->DisableBlending();
//    }
//    
//    static void EnableDethTesting()
//    {
//        s_RendererAPI->EnableDethTesting();
//    }
//    
//    static void DisableDethTesting()
//    {
//        s_RendererAPI->DisableDethTesting();
//    }
//    
//    static void EnableScissor()
//    {
//        s_RendererAPI->EnableScissor();
//    }
//    
//    static void DisableScissorg()
//    {
//        s_RendererAPI->DisableScissor();
//    }
//
//    static void SetScissor(int left, int bottom, int width, int height)
//    {
//        s_RendererAPI->SetScissor(left, bottom, width, height);
//    }
//
//    static void Flush()
//    {
//        return; // Flush() deactivated
//        s_RendererAPI->Flush();
//    }
//
//    static std::unique_ptr<RendererAPI> s_RendererAPI;
//    
//private:
//
//    
//};
//