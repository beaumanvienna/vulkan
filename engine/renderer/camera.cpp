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

#include "renderer/camera.h"
#include "transform/matrix.h"

namespace GfxRenderEngine
{

    Camera::Camera(ProjectionType projectionType) : m_ProjectionType{projectionType}, m_Position{0.0f}, m_Rotation{0.0f} {}

    void Camera::SetProjection(float left, float right, float bottom, float top, float near, float far)
    {
        SetOrthographicProjection(left, right, bottom, top, near, far);
    }

    void Camera::SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
    {
        m_ProjectionType = ORTHOGRAPHIC_PROJECTION;
        m_ProjectionMatrix = glm::mat4{1.0f};
        m_ProjectionMatrix[0][0] = -2.f / (right + left);
        m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
        m_ProjectionMatrix[2][2] = 1.f / (far - near);
        m_ProjectionMatrix[3][0] = (left - right) / (right + left);
        m_ProjectionMatrix[3][1] = (bottom + top) / (top - bottom);
        m_ProjectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::SetOrthographicProjection3D(float left, float right, float bottom, float top, float near, float far)
    {
        m_ProjectionType = ORTHOGRAPHIC_PROJECTION;
        m_ProjectionMatrix = glm::ortho(-left, -right, bottom, top, near, far);
    }

    void Camera::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
    {
        CORE_ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "aspect too small");
        m_ProjectionType = PERSPECTIVE_PROJECTION;
        const float tanHalfFovy = tan(fovy / 2.f);
        m_ProjectionMatrix = glm::mat4{0.0f};
        m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
        m_ProjectionMatrix[2][2] = far / (far - near);
        m_ProjectionMatrix[2][3] = 1.f;
        m_ProjectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up)
    {
        m_Position = position;

        // create orthonormal basis
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};

        m_ViewMatrix = glm::mat4{1.f};
        m_ViewMatrix[0][0] = u.x;
        m_ViewMatrix[1][0] = u.y;
        m_ViewMatrix[2][0] = u.z;
        m_ViewMatrix[0][1] = v.x;
        m_ViewMatrix[1][1] = v.y;
        m_ViewMatrix[2][1] = v.z;
        m_ViewMatrix[0][2] = w.x;
        m_ViewMatrix[1][2] = w.y;
        m_ViewMatrix[2][2] = w.z;
        m_ViewMatrix[3][0] = -glm::dot(u, position);
        m_ViewMatrix[3][1] = -glm::dot(v, position);
        m_ViewMatrix[3][2] = -glm::dot(w, position);

        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void Camera::SetView(const glm::mat4& modelMatrix)
    {
        constexpr int column = 3;
        m_Position = glm::vec3(modelMatrix[column][0], modelMatrix[column][1], modelMatrix[column][2]);

        static glm::vec3 cameraPosition{0.0f, 0.0f, 0.0f};
        static glm::vec3 target{0.0f, 0.0f, 1.0f};
        static glm::vec3 up{0.0f, -1.0f, 0.0f};
        static glm::mat4 lookAt = glm::lookAt(cameraPosition, target, up);
        m_ViewMatrix = lookAt * glm::inverse(modelMatrix);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void Camera::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation)
    {
        m_Position = position;
        m_Rotation = glm::vec3{rotation.x, rotation.y + glm::pi<float>(), rotation.z + glm::pi<float>()};

        const float c3 = glm::cos(m_Rotation.z);
        const float s3 = glm::sin(m_Rotation.z);
        const float c2 = glm::cos(m_Rotation.x);
        const float s2 = glm::sin(m_Rotation.x);
        const float c1 = glm::cos(m_Rotation.y);
        const float s1 = glm::sin(m_Rotation.y);
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
        m_ViewMatrix = glm::mat4{1.f};
        m_ViewMatrix[0][0] = u.x;
        m_ViewMatrix[1][0] = u.y;
        m_ViewMatrix[2][0] = u.z;
        m_ViewMatrix[0][1] = v.x;
        m_ViewMatrix[1][1] = v.y;
        m_ViewMatrix[2][1] = v.z;
        m_ViewMatrix[0][2] = w.x;
        m_ViewMatrix[1][2] = w.y;
        m_ViewMatrix[2][2] = w.z;
        m_ViewMatrix[3][0] = -glm::dot(u, position);
        m_ViewMatrix[3][1] = -glm::dot(v, position);
        m_ViewMatrix[3][2] = -glm::dot(w, position);

        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
} // namespace GfxRenderEngine
