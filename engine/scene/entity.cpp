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

#include "scene/entity.h"

namespace GfxRenderEngine
{

    Entity::~Entity()
    {
    }

    Entity Entity::CreateEnity()
    {
        static id_t currentID = 0;

        return Entity{currentID++};
    }

    glm::mat4 TransformComponent::Mat4()
    {
        auto transform = glm::translate(glm::mat4{1.0f}, m_Translation);
        transform = glm::rotate(transform, m_Rotation.y, glm::vec3{0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, m_Rotation.x, glm::vec3{1.0f, 0.0f, 0.0f});
        transform = glm::rotate(transform, m_Rotation.z, glm::vec3{0.0f, 0.0f, 1.0f});
        transform = glm::scale(transform, m_Scale);
        return transform;
    }
}
