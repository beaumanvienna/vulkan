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

#include "gtx/matrix_decompose.hpp"

#include "cameraController.h"
#include "core.h"
#include "platform/input.h"

namespace GfxRenderEngine
{

    CameraController::CameraController(OrthographicCameraComponent& orthographicCameraComponent) : m_ZoomFactor{1.0f}
    {
        m_Camera = std::make_shared<Camera>(Camera::ProjectionType::ORTHOGRAPHIC_PROJECTION);
        SetProjection(orthographicCameraComponent);
    }

    CameraController::CameraController(PerspectiveCameraComponent& perspectiveCameraComponent) : m_ZoomFactor{1.0f}
    {
        m_Camera = std::make_shared<Camera>(Camera::ProjectionType::PERSPECTIVE_PROJECTION);
        SetProjection(perspectiveCameraComponent);
    }

    void CameraController::SetZoomFactor(float factor)
    {
        m_ZoomFactor = factor;
        SetProjection();
    }

    void CameraController::SetProjection(PerspectiveCameraComponent& perspectiveCameraComponent)
    {

        m_ZNear = perspectiveCameraComponent.m_ZNear;
        m_ZFar = perspectiveCameraComponent.m_ZFar;
        m_Fovy = perspectiveCameraComponent.m_YFov;
        m_Aspect = Engine::m_Engine->GetWindowAspectRatio(); // aspect ratio of main window

        m_Camera->SetPerspectiveProjection(m_Fovy * m_ZoomFactor, m_Aspect, m_ZNear, m_ZFar);
    }

    void CameraController::SetProjection(OrthographicCameraComponent& orthographicCameraComponent)
    {
        m_XMag = orthographicCameraComponent.m_XMag;
        m_YMag = orthographicCameraComponent.m_YMag;
        m_ZNear = orthographicCameraComponent.m_ZNear;
        m_ZFar = orthographicCameraComponent.m_ZFar;
        m_Aspect = Engine::m_Engine->GetWindowAspectRatio(); // aspect ratio of main window

        float normalize = Engine::m_Engine->GetWindowWidth();
        float orthoLeft = 0.0f;
        float orthoRight = normalize;
        float orthoBottom = normalize / m_Aspect;
        float orthoTop = 0.0f;
        float orthoNear = m_ZNear;
        float orthoFar = m_ZFar;
        m_Camera->SetOrthographicProjection(orthoLeft * m_ZoomFactor, orthoRight * m_ZoomFactor, orthoBottom * m_ZoomFactor,
                                            orthoTop * m_ZoomFactor, orthoNear, orthoFar);
    }

    void CameraController::SetProjection()
    {
        m_Aspect = Engine::m_Engine->GetWindowAspectRatio(); // aspect ratio of main window
        Camera::ProjectionType type = m_Camera->GetProjectionType();
        switch (type)
        {
            case Camera::ORTHOGRAPHIC_PROJECTION:
            {
                float normalize = Engine::m_Engine->GetWindowWidth();

                float orthoLeft = 0.0f;
                float orthoRight = normalize;
                float orthoBottom = normalize / m_Aspect;
                float orthoTop = 0.0f;
                float orthoNear = m_ZNear;
                float orthoFar = m_ZFar;
                m_Camera->SetOrthographicProjection(orthoLeft * m_ZoomFactor, orthoRight * m_ZoomFactor,
                                                    orthoBottom * m_ZoomFactor, orthoTop * m_ZoomFactor, orthoNear,
                                                    orthoFar);
                break;
            }
            case Camera::PERSPECTIVE_PROJECTION:
            {
                m_Camera->SetPerspectiveProjection(m_Fovy * m_ZoomFactor, m_Aspect, m_ZNear, m_ZFar);
                break;
            }
            case Camera::PROJECTION_UNDEFINED:
            {
                CORE_ASSERT(false, "CameraController PROJECTION UNDEFINED");
                break;
            }
        }
    }

    void CameraController::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation)
    {
        m_Camera->SetViewYXZ(position, rotation);
    }

    void CameraController::SetView(const glm::mat4& modelMatrix) { m_Camera->SetView(modelMatrix); }
} // namespace GfxRenderEngine
