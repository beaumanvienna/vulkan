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

#include "engine.h"

namespace GfxRenderEngine
{

    class Camera
    {
    public:

        enum ProjectionTypes
        {
            ORTHOGRAPHIC_PROJECTION,
            PERSPECTIVE_PROJECTION
        };

    public:
        Camera();

        void SetOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
        void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

        //Camera(float left, float right, float bottom, float top, float near, float far);
        //void SetProjection(float left, float right, float bottom, float top, float near, float far);
        //
        //const glm::vec3& GetPosition() const { return m_Position; }
        //const float& GetRotation() const { return m_Rotation; }
        //
        void SetPosition(const glm::vec3& position);
        void SetRotation(const float& rotation);
        //
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        //const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        //const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    private:

        void RecalculateViewMatrix();

    private: 

        glm::mat4 m_ProjectionMatrix;
        //glm::mat4 m_ViewMatrix;
        //glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position;
        float m_Rotation = 0.0f;

    };
}
