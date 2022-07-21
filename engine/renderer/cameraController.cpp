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

#include "cameraController.h"
#include "core.h"
#include "platform/input.h"

namespace GfxRenderEngine
{

    CameraController::CameraController(Camera::ProjectionType type)
        : m_ZoomFactor{1.0f}
    {
        m_Camera = std::make_shared<Camera>();
        SetProjection(type);
    }

    void CameraController::SetZoomFactor(float factor)
    {
        m_ZoomFactor = factor;
        SetProjection(m_Camera->GetProjectionType());
    }

    void CameraController::SetRotation(const glm::vec3& rotation)
    {
        m_Camera->SetRotation( rotation );
    }

    void CameraController::SetTranslation(const glm::vec2& translation)
    {
        m_Camera->SetPosition( {translation.x, translation.y, 0.0f} );
    }

    void CameraController::SetTranslation(const glm::vec3& translation)
    {
        m_Camera->SetPosition(translation);
    }

    void CameraController::SetProjection()
    {
        SetProjection(m_Camera->GetProjectionType());
    }

    void CameraController::SetProjection(Camera::ProjectionType type)
    {
        // aspect ratio of main window
        float aspectRatio = Engine::m_Engine->GetWindowAspectRatio();

        switch(type)
        {
            case Camera::ORTHOGRAPHIC_PROJECTION:
            {
                float normalize = Engine::m_Engine->GetContextWidth();

                float ortho_left   =   normalize;
                float ortho_right  =  -normalize;
                float ortho_bottom =  -normalize / aspectRatio;
                float ortho_top    =   normalize / aspectRatio;
                float ortho_near   =  1.0f;
                float ortho_far    = -1.0f;
                m_Camera->SetOrthographicProjection
                (
                    ortho_left * m_ZoomFactor,
                    ortho_right * m_ZoomFactor,
                    ortho_bottom * m_ZoomFactor,
                    ortho_top * m_ZoomFactor,
                    ortho_near,
                    ortho_far
                );
                break;
            }
            case Camera::PERSPECTIVE_PROJECTION:
            {
                m_Camera->SetPerspectiveProjection
                (
                    glm::radians(50.0f) * m_ZoomFactor,
                    aspectRatio,
                    0.1f, // near
                    50.0f // far
                );
                break;
            }
        }
    }

    void CameraController::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation)
    {
        m_Camera->SetViewYXZ(position, rotation);
    }
}
