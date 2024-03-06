/* Engine Copyright (c) 2021 Engine Development Team
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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "core.h"
#include "transform/matrix.h"
#include "transform/transformation.h"
#include "scene/components.h"

#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

namespace GfxRenderEngine
{

    Transformation::Transformation(float duration)
        : m_Duration(duration), m_StartTime(0s), m_Transform(glm::mat4(1.0f)), m_IsRunning(false)
    {
    }

    void Transformation::Start()
    {
        if (!m_IsRunning)
        {
            m_StartTime = Engine::m_Engine->GetTime();
            m_IsRunning = true;
        }
    }

    void Transformation::Stop() { m_IsRunning = false; }

    bool Transformation::IsRunning()
    {
        m_IsRunning = (Engine::m_Engine->GetTime() - m_StartTime) < m_Duration;
        return m_IsRunning;
    }

    Translation::Translation(float duration /* in seconds */, glm::vec2& pos1, glm::vec2& pos2)
        : Transformation(duration), m_Pos1(pos1), m_Pos2(pos2), m_Translation(glm::vec3(0.0f))
    {
    }

    glm::mat4& Translation::GetTransformation()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaX = m_Pos1.x * (1 - delta) + m_Pos2.x * delta;
            float deltaY = m_Pos1.y * (1 - delta) + m_Pos2.y * delta;
            glm::vec3 translation = glm::vec3(deltaX, deltaY, 0.0f);

            m_Transform = Translate(translation);
        }

        return m_Transform;
    }

    glm::vec3 Translation::GetTranslation()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaX = m_Pos1.x * (1 - delta) + m_Pos2.x * delta;
            float deltaY = m_Pos1.y * (1 - delta) + m_Pos2.y * delta;
            m_Translation = glm::vec3(deltaX, deltaY, 0.0f);
        }
        return m_Translation;
    }

    Rotation::Rotation(float duration, float rotation1, float rotation2)
        : Transformation(duration), m_Rotation(glm::vec3(0.0f)), m_Rotation1(rotation1), m_Rotation2(rotation2)
    {
    }

    glm::mat4& Rotation::GetTransformation()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaRotation = m_Rotation1 * (1 - delta) + m_Rotation2 * delta;
            m_Transform = Rotate(deltaRotation, glm::vec3(0, 0, 1));
        }

        return m_Transform;
    }

    glm::vec3 Rotation::GetRotation()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaRotation = m_Rotation1 * (1 - delta) + m_Rotation2 * delta;
            m_Rotation = glm::vec3(0, 0, deltaRotation);
        }
        return m_Rotation;
    }

    Scaling::Scaling(float duration, float scale1, float scale2)
        : Transformation(duration), m_ScaleX1(1.0f), m_ScaleY1(scale1), m_ScaleX2(1.0f), m_ScaleY2(scale2)
    {
    }

    Scaling::Scaling(float duration /* in seconds */, float scaleX1, float scaleY1, float scaleX2, float scaleY2)
        : Transformation(duration), m_ScaleX1(scaleX1), m_ScaleY1(scaleY1), m_ScaleX2(scaleX2), m_ScaleY2(scaleY2),
          m_Scale(glm::vec3(0.0f))
    {
    }

    glm::mat4& Scaling::GetTransformation()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaScaleX = m_ScaleX1 * (1 - delta) + m_ScaleX2 * delta;
            float deltaScaleY = m_ScaleY1 * (1 - delta) + m_ScaleY2 * delta;
            m_Transform = Scale(glm::vec3(deltaScaleX, deltaScaleY, 1.0f));
        }

        return m_Transform;
    }

    glm::vec3 Scaling::GetScale()
    {
        float delta;
        if (IsRunning())
        {
            delta = (Engine::m_Engine->GetTime() - m_StartTime) / m_Duration;
            float deltaScaleX = m_ScaleX1 * (1 - delta) + m_ScaleX2 * delta;
            float deltaScaleY = m_ScaleY1 * (1 - delta) + m_ScaleY2 * delta;
            m_Scale = glm::vec3(deltaScaleX, deltaScaleY, 1.0f);
        }
        return m_Scale;
    }

    Animation::Animation()
        : m_CurrentSequenceTranslation(0), m_CurrentSequenceRotation(0), m_CurrentSequenceScale(0), m_Running(false)
    {
    }

    void Animation::Start()
    {
        bool isRunningTranslation = false, isRunningRotation = false, isRunningScale = false;
        m_CurrentSequenceTranslation = 0;
        m_CurrentSequenceRotation = 0;
        m_CurrentSequenceScale = 0;

        if (m_Translations.size())
        {
            isRunningTranslation = true;
            m_Translations[0].Start();
        }
        if (m_Rotations.size())
        {
            isRunningRotation = true;
            m_Rotations[0].Start();
        }
        if (m_Scalings.size())
        {
            isRunningScale = true;
            m_Scalings[0].Start();
        }

        m_Running = isRunningTranslation | isRunningRotation | isRunningScale;
    }

    void Animation::Stop()
    {
        m_Running = false;

        if (m_Translations.size())
        {
            m_Translations[0].Stop();
        }
        if (m_Rotations.size())
        {
            m_Rotations[0].Stop();
        }
        if (m_Scalings.size())
        {
            m_Scalings[0].Stop();
        }
    }

    void Animation::Reset()
    {
        m_Translations.clear();
        m_Rotations.clear();
        m_Scalings.clear();
        m_Running = false;
        m_CurrentSequenceTranslation = 0;
        m_CurrentSequenceRotation = 0;
        m_CurrentSequenceScale = 0;
        m_FinalScaling = glm::vec3(1.0f);
        m_FinalRotation = glm::vec3(0.0f);
        m_FinalTranslation = glm::vec3(0.0f);
    }

    bool Animation::IsRunning()
    {
        if (m_Running)
        {
            // determine if m_Running needs to be reset and deal with sequence counter
            bool isRunningTranslation = false, isRunningRotation = false, isRunningScale = false;

            // translation sequences
            m_NumberOfTranslationSequences = m_Translations.size();
            if (m_NumberOfTranslationSequences)
            {
                isRunningTranslation = m_Translations[m_CurrentSequenceTranslation].IsRunning();
                if ((!isRunningTranslation) && (m_CurrentSequenceTranslation < m_NumberOfTranslationSequences - 1))
                {
                    m_CurrentSequenceTranslation++;
                    m_Translations[m_CurrentSequenceTranslation].Start();
                    isRunningTranslation = m_Translations[m_CurrentSequenceTranslation].IsRunning();
                }
            }

            // rotation sequences
            m_NumberOfRotationSequences = m_Rotations.size();
            if (m_NumberOfRotationSequences)
            {
                isRunningRotation = m_Rotations[m_CurrentSequenceRotation].IsRunning();
                if ((!isRunningRotation) && (m_CurrentSequenceRotation < m_NumberOfRotationSequences - 1))
                {
                    m_CurrentSequenceRotation++;
                    m_Rotations[m_CurrentSequenceRotation].Start();
                    isRunningRotation = m_Rotations[m_CurrentSequenceRotation].IsRunning();
                }
            }

            // translation sequences
            m_NumberOfScaleSequences = m_Scalings.size();
            if (m_NumberOfScaleSequences)
            {
                isRunningScale = m_Scalings[m_CurrentSequenceScale].IsRunning();
                if ((!isRunningScale) && (m_CurrentSequenceScale < m_NumberOfScaleSequences - 1))
                {
                    m_CurrentSequenceScale++;
                    m_Scalings[m_CurrentSequenceScale].Start();
                    isRunningScale = m_Scalings[m_CurrentSequenceScale].IsRunning();
                }
            }

            m_Running = isRunningTranslation | isRunningRotation | isRunningScale;
        }
        return m_Running;
    }

    void Animation::AddTranslation(const Translation translation) { m_Translations.push_back(translation); }

    void Animation::AddRotation(const Rotation rotation) { m_Rotations.push_back(rotation); }

    void Animation::AddScaling(const Scaling scale) { m_Scalings.push_back(scale); }

    const glm::mat4& Animation::GetMat4()
    {
        if (IsRunning())
        {
            m_Transform = glm::mat4(1.0f);
            if (m_NumberOfScaleSequences)
            {
                m_Transform = m_Scalings[m_CurrentSequenceScale].GetTransformation() * m_Transform;
            }
            if (m_NumberOfRotationSequences)
            {
                m_Transform = m_Rotations[m_CurrentSequenceRotation].GetTransformation() * m_Transform;
            }
            if (m_NumberOfTranslationSequences)
            {
                m_Transform = m_Translations[m_CurrentSequenceTranslation].GetTransformation() * m_Transform;
            }
        }
        else
        {
            m_Transform = Scale(m_FinalScaling) * glm::mat4(1.0f);
            ;
            m_Transform = glm::toMat4(glm::quat(m_FinalRotation)) * m_Transform;
            m_Transform = Translate(m_FinalTranslation) * m_Transform;
        }
        return m_Transform;
    }

    void Animation::SetFinal(const glm::vec3& scaling, const glm::vec3& rotation, const glm::vec3 translation)
    {
        m_FinalScaling = scaling;
        m_FinalRotation = rotation;
        m_FinalTranslation = translation;
    }
} // namespace GfxRenderEngine
