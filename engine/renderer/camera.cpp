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
#include "glm/gtx/euler_angles.hpp"

namespace GfxRenderEngine
{

    Camera::Camera(ProjectionType projectionType) : m_ProjectionType{projectionType}, m_Position{0.0f}, m_Rotation{0.0f} {}

    void Camera::SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
    {
        m_ProjectionType = ORTHOGRAPHIC_PROJECTION;
        // top and bottom flipped to invert Y
        m_ProjectionMatrix = glm::orthoRH(left, right, top, bottom, near, far);
    }

    void Camera::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
    {
        m_Fovy = fovy;
        CORE_ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "aspect too small");
        m_ProjectionType = PERSPECTIVE_PROJECTION;
        m_ProjectionMatrix = glm::perspective(fovy, aspect, near, far);
        m_ProjectionMatrix[1][1] *= -1; // flip Y
    }

    void Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up)
    {
        m_Position = position;
        m_ViewMatrix = glm::lookAtRH(position, direction, up);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        m_Direction = glm::normalize(direction);
    }

    void Camera::SetView(const glm::mat4& modelMatrix)
    {
        constexpr int column = 3;
        m_Position = glm::vec3(modelMatrix[column][0], modelMatrix[column][1], modelMatrix[column][2]);
        m_ViewMatrix = glm::inverse(modelMatrix);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

        glm::vec3 forward{0.0f, 0.0f, -1.0f}; // For right-handed
        m_Direction = glm::normalize(glm::mat3(modelMatrix) * forward);
    }
} // namespace GfxRenderEngine
