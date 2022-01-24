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

#include <memory>

#include "engine.h"
#include "camera.h"

namespace GfxRenderEngine
{

    class CameraController
    {
    public:

        CameraController(Camera::ProjectionType type = Camera::PERSPECTIVE_PROJECTION);
        void OnUpdate();

        void SetTranslationSpeed(float translationSpeed) { m_TranslationSpeed = translationSpeed; }
        void SetRotationSpeed(float rotationSpeed) { m_RotationSpeed = rotationSpeed; }

        void SetProjection();
        void SetProjection(Camera::ProjectionType type);

        void SetZoomFactor(float factor);
        float GetZoomFactor() const { return m_ZoomFactor; }

        void SetRotation(const glm::vec3& rotation);
        void SetTranslation(const glm::vec2& translation);
        void SetTranslation(const glm::vec3& translation);

        std::shared_ptr<Camera>& GetCamera() { return m_Camera; }

    private:

        std::shared_ptr<Camera> m_Camera;

        glm::vec2 m_Translation;
        float m_TranslationSpeed;

        float m_ZoomFactor;
        float m_RotationSpeed;

    };
}
