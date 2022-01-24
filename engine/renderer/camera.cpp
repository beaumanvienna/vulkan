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

#include "renderer/camera.h"

namespace GfxRenderEngine
{

    Camera::Camera()
        : m_ProjectionMatrix{1.0f}/*, m_Position{0.0f}, m_Rotation(0.0f)*/
    {
        //RecalculateViewMatrix();
    }

    void Camera::SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far)
    {
        m_ProjectionMatrix = glm::mat4{1.0f};
        m_ProjectionMatrix[0][0] = 2.f / (right - left);
        m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
        m_ProjectionMatrix[2][2] = 1.f / (far - near);
        m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
        m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        m_ProjectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
    {
        ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        m_ProjectionMatrix = glm::mat4{0.0f};
        m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        m_ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
        m_ProjectionMatrix[2][2] = far / (far - near);
        m_ProjectionMatrix[2][3] = 1.f;
        m_ProjectionMatrix[3][2] = -(far * near) / (far - near);
    }

    //Camera::Camera(float left, float right, float bottom, float top, float near, float far)
    //    : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, near, far)), m_Position({0.0f, 0.0f, 0.0f}), m_Rotation(0.0f)
    //{
    //    RecalculateViewMatrix();
    //}
    //
    //void Camera::SetProjection(float left, float right, float bottom, float top, float near, float far)
    //{
    //    m_ProjectionMatrix = glm::ortho(left, right, bottom, top, near, far);
    //    RecalculateViewMatrix();
    //}

    void Camera::SetPosition(const glm::vec3& position) 
    { 
        m_Position = position; 
        //RecalculateViewMatrix();
    }

    void Camera::SetRotation(const float& rotation) 
    { 
        m_Rotation = rotation; 
        //RecalculateViewMatrix();
    }

    //void Camera::RecalculateViewMatrix()
    //{
    //    glm::mat4 translate = Translate( m_Position);
    //    glm::mat4 rotate =  Rotate( m_Rotation, glm::vec3(0, 0, 1) );
    //    glm::mat4 transform =  translate * rotate;
    //
    //    m_ViewMatrix = glm::inverse(transform);
    //    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    //}
}
