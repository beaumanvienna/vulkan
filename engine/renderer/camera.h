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

#include "engine.h"

namespace GfxRenderEngine
{

    class Camera
    {
    public:

        enum ProjectionType
        {
            PROJECTION_UNDEFINED,
            ORTHOGRAPHIC_PROJECTION,
            PERSPECTIVE_PROJECTION
        };

    public:

        Camera();

        void SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);
        void SetProjection(float left, float right, float bottom, float top, float near, float far);
        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }

        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation);

        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

        void SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f});
        void SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f});
        void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);

    private:

        void RecalculateViewMatrix();

    private: 

        ProjectionType m_ProjectionType;
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position;
        glm::vec3 m_Rotation;

    };
}
