/* Engine Copyright (c) 2021 Engine Development Team 
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

#include "vendor/imgui/imgui.h"
#include "vendor/imGuizmo/ImGuizmo.h"
#include "gtc/type_ptr.hpp"

#include "core.h"
#include "scene/scene.h"

#include "application/lucre/lucre.h"
#include "application/lucre/UI/imgui.h"

namespace LucreApp
{
    int   ImGUI::m_SelectedGameObject;
    float ImGUI::m_Roughness = 0.1f;
    bool  ImGUI::m_UseRoughness = false;
    float ImGUI::m_Metallic = 0.5f;
    bool  ImGUI::m_UseMetallic = false;
    float ImGUI::m_NormalMapIntensity = 0.9f;
    bool  ImGUI::m_UseNormalMapIntensity = false;
    float ImGUI::m_PointLightIntensity = 1.0f;
    bool  ImGUI::m_UsePointLightIntensity = false;

    void ImGUI::DebugWindow()
    {
        // selected entity
        ImGui::SliderInt("Game Object", &m_SelectedGameObject, 0, 17);
        // roughness
        ImGui::Checkbox("use###001", &m_UseRoughness);
        ImGui::SameLine();
        ImGui::SliderFloat("roughness", &m_Roughness, 0.0f, 1.0f);

        // metallic
        ImGui::Checkbox("use###002", &m_UseMetallic);
        ImGui::SameLine();
        ImGui::SliderFloat("metallic", &m_Metallic, 0.0f, 1.0f);

        // normal map intensity
        ImGui::Checkbox("use###003", &m_UseNormalMapIntensity);
        ImGui::SameLine();
        ImGui::SliderFloat("normal map", &m_NormalMapIntensity, 0.0f, 1.0f);

        // point light intensity
        ImGui::Checkbox("use###004", &m_UsePointLightIntensity);
        ImGui::SameLine();
        ImGui::SliderFloat("pt lghts", &m_PointLightIntensity, 0.0f, 10.0f);

        if (m_SelectedGameObject)
        {
            ImGuizmo::BeginFrame();
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            uint contextWidth  = Engine::m_Engine->GetContextWidth();
            uint contextHeight = Engine::m_Engine->GetContextHeight();

            ImGuizmo::SetRect(0, 0, contextWidth, contextHeight);

            auto& currentScene = Lucre::m_Application->GetScene();
            auto& camera = currentScene.GetCamera();

            auto projectionMatrix = glm::scale(glm::mat4(1.0f), {1.0f, -1.0f, 1.0f}) * camera.GetProjectionMatrix();
            auto& viewMatrix = camera.GetViewMatrix();

            auto& transform = currentScene.Registry().get<TransformComponent>((entt::entity)m_SelectedGameObject);
            glm::mat4 mat4 = transform.GetMat4();

            ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
                ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(mat4));
        }
    }
}
