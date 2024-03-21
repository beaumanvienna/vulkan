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
        Camera() = delete;
        Camera(ProjectionType projectionType);

        void SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
        void SetOrthographicProjection3D(float left, float right, float bottom, float top, float near, float far);
        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);
        void SetProjection(float left, float right, float bottom, float top, float near, float far);
        void SetViewDirection(const glm::vec3& position, const glm::vec3& direction,
                              const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f});
        void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);
        void SetView(const glm::mat4& modelMatrix);
        void SetName(const std::string& name) { m_Name = name; }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const std::string& GetName() const { return m_Name; }

    private:
        void RecalculateViewMatrix();

    private:
        std::string m_Name;
        ProjectionType m_ProjectionType;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position;
        glm::vec3 m_Rotation;
    };
} // namespace GfxRenderEngine
