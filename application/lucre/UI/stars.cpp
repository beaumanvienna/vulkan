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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "lucre.h"
#include "UI/stars.h"
#include "transform/matrix.h"

namespace LucreApp
{
    UIStarIcon::UIStarIcon(std::string const& name) : Layer(name) {}

    UIStarIcon::~UIStarIcon() {}

    void UIStarIcon::OnAttach()
    {
        m_Renderer = Engine::m_Engine->GetRenderer();
        m_Spritesheet = Lucre::m_Spritesheet;

        Init();
    }

    void UIStarIcon::Init()
    {
        m_StarMoveIn1.Reset();
        m_StarMoveIn2.Reset();
        m_StarMoveIn3.Reset();
        m_StarRotate1.Reset();
        m_StarRotate2.Reset();
        m_StarRotate3.Reset();
        m_StarMoveOut1.Reset();
        m_StarMoveOut2.Reset();
        m_StarMoveOut3.Reset();

        float duration = 1.0f;
        // star icon
        float desktopWidth = Engine::m_Engine->GetDesktopWidth();
        float windowHeight = Engine::m_Engine->GetWindowHeight();

        float scale = 4.0f * windowHeight / desktopWidth;
        m_StarSprite = Sprite2D(m_Spritesheet->GetSprite(I_STAR));
        m_StarSprite.SetScale(scale);

        // 1st star icon: move right to top right corner
        glm::vec2 finalOutOfScreenPosition;
        glm::vec2 finalScreenPosition1;
        float xPosWide1 = UI::g_Common->m_AvailableWidth - UI::g_Common->m_TabMarginLeftRight;
        float yPosWide12 = 160.0f * UI::g_Common->m_ScaleAll;
        finalOutOfScreenPosition = glm::vec2(2000.0f, 300.0f);
        finalScreenPosition1 = glm::vec2(xPosWide1, yPosWide12);

        m_StarMoveIn1.AddTranslation(Translation(1.0f * duration, finalOutOfScreenPosition, finalScreenPosition1));
        m_StarMoveIn1.AddRotation(Rotation(1.0f * duration, 0.0f, 3.141f));

        m_StarRotate1.AddRotation(Rotation(100.0f * duration, 0.0f, 250.0f));
        m_StarRotate1.AddTranslation(Translation(100.0f * duration, finalScreenPosition1, finalScreenPosition1));

        m_StarMoveOut1.AddTranslation(Translation(1.0f * duration, finalScreenPosition1, finalOutOfScreenPosition));
        m_StarMoveOut1.AddRotation(Rotation(1.0f * duration, 0.0f, 3.141f));

        // 2nd star icon: move left to top left corner
        float xPosWide23 = UI::g_Common->m_TabMarginLeftRight + UI::g_Common->m_IconWidth * 3.0f;
        glm::vec2 finalScreenPosition2;
        finalScreenPosition2 = glm::vec2(xPosWide23, yPosWide12);

        m_StarMoveIn2.AddTranslation(Translation(1.0f * duration, finalOutOfScreenPosition, finalScreenPosition2));
        m_StarMoveIn2.AddRotation(Rotation(1.0f * duration, 0.0f, -3.141f));

        m_StarRotate2.AddRotation(Rotation(100.0f * duration, 0.0f, -250.0f));
        m_StarRotate2.AddTranslation(Translation(100.0f * duration, finalScreenPosition2, finalScreenPosition2));

        m_StarMoveOut2.AddTranslation(Translation(1.0f * duration, finalScreenPosition2, finalOutOfScreenPosition));
        m_StarMoveOut2.AddRotation(Rotation(1.0f * duration, 0.0f, 3.141f));

        // 3rd star icon: move left to bottom left corner
        float yPosWide3 = yPosWide12 + 460.0f * UI::g_Common->m_ScaleAll;
        glm::vec2 finalScreenPosition3;

        finalScreenPosition3 = glm::vec2(xPosWide23, yPosWide3);

        m_StarMoveIn3.AddTranslation(Translation(1.0f * duration, finalOutOfScreenPosition, finalScreenPosition3));
        m_StarMoveIn3.AddRotation(Rotation(1.0f * duration, 0.0f, 3.141f));

        m_StarRotate3.AddRotation(Rotation(100.0f * duration, 0.0f, 250.0f));
        m_StarRotate3.AddTranslation(Translation(100.0f * duration, finalScreenPosition3, finalScreenPosition3));

        m_StarMoveOut3.AddTranslation(Translation(1.0f * duration, finalScreenPosition3, finalOutOfScreenPosition));
        m_StarMoveOut3.AddRotation(Rotation(1.0f * duration, 0.0f, 3.141f));

        m_Start = false;
        m_Stop = false;
        ChangeState(State::IDLE);
    }

    void UIStarIcon::OnDetach() {}

    void UIStarIcon::OnUpdate(Timestep const& timestep)
    {
        switch (m_State)
        {
            case State::IDLE:
                if (m_Start)
                {
                    ChangeState(State::MOVE_IN);
                    StartSequence();
                }
                break;
            case State::MOVE_IN:
                if (m_Stop)
                {
                    ChangeState(State::MOVE_OUT);
                    StopSequence();
                }
                else if (!m_StarMoveIn1.IsRunning())
                {
                    ChangeState(State::ROTATE);
                    Rotate();
                }
                break;
            case State::ROTATE:
                if (m_Stop)
                {
                    ChangeState(State::MOVE_OUT);
                    StopSequence();
                }
                break;
            case State::MOVE_OUT:
                if (m_Start)
                {
                    ChangeState(State::MOVE_IN);
                    StartSequence();
                }
                else if (!m_StarMoveOut1.IsRunning())
                {
                    ChangeState(State::IDLE);
                }
                break;
        }

        if (m_StarMoveIn1.IsRunning())
        {
            {
                auto animationMatrix = m_StarMoveIn1.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarMoveIn2.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarMoveIn3.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
        }
        else if (m_StarRotate1.IsRunning())
        {
            {
                auto animationMatrix = m_StarRotate1.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarRotate2.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarRotate3.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
        }
        else if (m_StarMoveOut1.IsRunning())
        {
            {
                auto animationMatrix = m_StarMoveOut1.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarMoveOut2.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
            {
                auto animationMatrix = m_StarMoveOut3.GetMat4();

                // transformed position
                glm::mat4 position = animationMatrix * m_StarSprite.GetMat4();
                m_Renderer->DrawWithTransform(m_StarSprite, position);
            }
        }
    }

    void UIStarIcon::OnEvent(Event& event) {}

    void UIStarIcon::StartSequence()
    {
        m_Start = false;
        m_Stop = false;
        m_StarMoveIn1.Start();
        m_StarMoveIn2.Start();
        m_StarMoveIn3.Start();

        m_StarRotate1.Stop();
        m_StarRotate2.Stop();
        m_StarRotate3.Stop();

        m_StarMoveOut1.Stop();
        m_StarMoveOut2.Stop();
        m_StarMoveOut3.Stop();
    }

    void UIStarIcon::StopSequence()
    {
        m_Start = false;
        m_Stop = false;
        m_StarMoveIn1.Stop();
        m_StarMoveIn2.Stop();
        m_StarMoveIn3.Stop();

        m_StarRotate1.Stop();
        m_StarRotate2.Stop();
        m_StarRotate3.Stop();

        m_StarMoveOut1.Start();
        m_StarMoveOut2.Start();
        m_StarMoveOut3.Start();
    }

    void UIStarIcon::Rotate()
    {
        m_StarMoveIn1.Stop();
        m_StarMoveIn2.Stop();
        m_StarMoveIn3.Stop();

        m_StarRotate1.Start();
        m_StarRotate2.Start();
        m_StarRotate3.Start();

        m_StarMoveOut1.Stop();
        m_StarMoveOut2.Stop();
        m_StarMoveOut3.Stop();
    }

    void UIStarIcon::ChangeState(State state) { m_State = state; }

    void UIStarIcon::OnResize()
    {
        bool start = m_StarMoveIn1.IsRunning();
        bool rotate = m_StarRotate1.IsRunning();

        Init();

        if (start || rotate)
            StartSequence();
    }
} // namespace LucreApp
