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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <string>
#include <memory>

#include "engine.h"
#include "scene/entity.h"
#include "scene/treeNode.h"
#include "scene/particleSystem.h"
#include "renderer/camera.h"

namespace GfxRenderEngine
{
    class Renderer
    {
    public:

        Renderer();
        virtual ~Renderer() {}

        // a draw call requires a vertex array (with a vertex buffer bound to it), index buffer, and bound shaders
        //void Submit(const std::shared_ptr<VertexArray>& vertexArray);
        virtual void Submit(entt::registry& registry, TreeNode& sceneHierarchy) = 0;
        virtual void Submit(std::shared_ptr<ParticleSystem>& particleSystem) = 0;
        virtual void NextSubpass() = 0;
        virtual void LightingPass() = 0;
        virtual void SubmitGUI(entt::registry& registry) = 0;
        virtual uint GetFrameCounter() = 0;

        //void BeginFrame(std::shared_ptr<OrthographicCamera>& camera, 
        //                        std::shared_ptr<ShaderProgram>& shader, 
        //                        std::shared_ptr<VertexBuffer>& vertexBuffer, 
        //                        std::shared_ptr<IndexBuffer>& indexBuffer);
        virtual void BeginFrame(Camera* camera, entt::registry& registry) = 0;
        virtual void EndScene() = 0;

        void Draw(Sprite* sprite, const glm::mat4& position, const float depth = 0.0f, const glm::vec4& color = glm::vec4(1.0f));
        //void Draw(std::shared_ptr<Texture> texture, const glm::mat4& position, const float depth, const glm::vec4& color = glm::vec4(1.0f));
        void Draw(std::shared_ptr<Texture> texture, const glm::mat4& position, const glm::vec4 textureCoordinates, const float depth, const glm::vec4& color = glm::vec4(1.0f));

    private:

        //void FillVertexBuffer(const int textureSlot, const glm::mat4& position, const float depth, const glm::vec4& color, const glm::vec4& textureCoordinates);

    private:

        //std::shared_ptr<IndexBuffer> m_IndexBuffer;
        //std::shared_ptr<VertexBuffer> m_VertexBuffer;
        //std::shared_ptr<ShaderProgram> m_Shader;
    };
}
