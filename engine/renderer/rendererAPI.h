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

#pragma once

//#include <memory>
//
//#include "engine.h"
//
//#include "vertexArray.h"
//#include "glm.hpp"

class RendererAPI
{
public:

    enum API
    {
        OPENGL,
        VULKAN
    };
    
public:
    
    //virtual ~RendererAPI() = default;
    //
    //virtual void SetClearColor(const glm::vec4& color) = 0;
    //virtual void Clear() const = 0;
    //virtual void EnableBlending() const = 0;
    //virtual void DisableBlending() const = 0;
    //virtual void EnableDethTesting() const = 0;
    //virtual void DisableDethTesting() const = 0;
    //virtual void EnableScissor() const = 0;
    //virtual void DisableScissor() const = 0;
    //virtual void SetScissor(int left, int bottom, int width, int height) const = 0;
    //
    //virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) const = 0;
    //
    //static void Create();
    //
    //static void SetAPI(API api) { s_API = api;}
    static API GetAPI() { return s_API;}
    
private:
    
    static API s_API;
    
};
