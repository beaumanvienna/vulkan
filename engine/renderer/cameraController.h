/* Engine Copyright (c) 2024 Engine Development Team
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

#include <memory>

#include "engine.h"
#include "camera.h"
#include "scene/components.h"

namespace GfxRenderEngine
{

    class CameraController
    {
    public:
        CameraController(OrthographicCameraComponent& orthographicCameraComponent);
        CameraController(PerspectiveCameraComponent& perspectiveCameraComponent);

        void SetProjection();
        void SetProjection(OrthographicCameraComponent& orthographicCameraComponent);
        void SetProjection(PerspectiveCameraComponent& perspectiveCameraComponent);

        void SetZoomFactor(float factor);
        float GetZoomFactor() const { return m_ZoomFactor; }

        void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);
        void SetView(const glm::mat4& modelMatrix);

        Camera& GetCamera() { return *m_Camera; }

    private:
        std::shared_ptr<Camera> m_Camera;

        float m_ZoomFactor;
        float m_ZNear;
        float m_ZFar;
        float m_Fovy;
        float m_Aspect;
        float m_XMag;
        float m_YMag;
    };
} // namespace GfxRenderEngine
