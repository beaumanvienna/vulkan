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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "engine.h"
#include "scene/components.h"

#include "application/lucre/scripts/duck/duckScript.h"

namespace LucreApp
{

    void DuckScript::Start()
    {
    }

    void DuckScript::Stop()
    {
    }

    void DuckScript::OnUpdate(const Timestep& timestep)
    {
        auto& transform = m_Registry.get<TransformComponent>(m_GameObject);

        const glm::vec3& translation = transform.GetTranslation();

        static constexpr float MOVE_UP   =  1.0f;
        static constexpr float MOVE_DOWN = -1.0f;
        static float move = MOVE_UP;

        if ((translation.y > 1.0f) && (move==MOVE_UP))
        {
            move = MOVE_DOWN;
        }
        else if ((translation.y < 0.5f) && (move==MOVE_DOWN))
        {
            move = MOVE_UP;
        }
        transform.SetTranslationY(translation.y + timestep*move);
    }

    void DuckScript::OnEvent()
    {
    }
}
