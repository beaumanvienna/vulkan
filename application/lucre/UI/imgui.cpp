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


#include "gtc/type_ptr.hpp"
#include "gtx/matrix_decompose.hpp"

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
    bool  ImGUI::m_UseScale = false;
    bool  ImGUI::m_UseRotate = false;
    bool  ImGUI::m_UseTranslate = false;

    void ImGUI::DebugWindow()
    {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
        uint contextWidth  = Engine::m_Engine->GetContextWidth();
        uint contextHeight = Engine::m_Engine->GetContextHeight();

        auto& currentScene = Lucre::m_Application->GetScene();
        auto& camera       = currentScene.GetCamera();
        auto& registry     = currentScene.GetRegistry();
        auto& dictionary   = currentScene.GetDictionary();

        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(contextWidth, contextHeight));
        // scale/rotate/translate mode
        ImGui::Checkbox("translate", &m_UseTranslate);
        ImGui::SameLine();
        ImGui::Checkbox("rotate", &m_UseRotate);
        ImGui::SameLine();
        ImGui::Checkbox("scale", &m_UseScale);
        ImGui::SameLine();

        // selected entity
        std::string gameObjectLabel = "Game Object";
        if (m_SelectedGameObject > 1)
        {
            auto& label = dictionary.GetShortName((entt::entity)m_SelectedGameObject);
            gameObjectLabel += std::string(" ") + label;
        }

        ImGui::SliderInt(gameObjectLabel.c_str(), &m_SelectedGameObject, 0, 17);
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
        ImGui::SliderFloat("point lights", &m_PointLightIntensity, 0.0f, 10.0f);

        auto guizmoMode = GetGuizmoMode();
        if (m_SelectedGameObject > 1) // id one is the camera
        {
            ImGuizmo::BeginFrame();
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(0, 0, contextWidth, contextHeight);

            auto projectionMatrix = glm::scale(glm::mat4(1.0f), {1.0f, -1.0f, 1.0f}) * camera.GetProjectionMatrix();
            auto& viewMatrix = camera.GetViewMatrix();

            auto& transform = registry.get<TransformComponent>((entt::entity)m_SelectedGameObject);
            glm::mat4 mat4 = transform.GetMat4();

            ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
                guizmoMode, ImGuizmo::LOCAL, glm::value_ptr(mat4));

            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(mat4, scale, rotation, translation, skew, perspective);
            glm::vec3 rotationEuler = glm::eulerAngles(rotation);

            if (ImGuizmo::IsUsing())
            {
                transform.SetTranslation(translation);
                transform.SetRotation(rotationEuler);
                transform.SetScale(scale);
            }

            glm::vec3 actualTranslation   = transform.GetTranslation();
            glm::vec3 actualRotationEuler = transform.GetRotation() * 180.0f / glm::pi<float>();
            glm::vec3 actualScale         = transform.GetScale();

            ImGui::InputFloat3("Translation", glm::value_ptr(actualTranslation));
            ImGui::InputFloat3("Rotation",    glm::value_ptr(actualRotationEuler));
            ImGui::InputFloat3("Scale",       glm::value_ptr(actualScale));

            if (glm::length(actualTranslation - transform.GetTranslation()) > 0.001f)
            {
                transform.SetTranslation(actualTranslation);
            }

            if (glm::length(actualRotationEuler - (transform.GetRotation() * 180.0f / glm::pi<float>())) > 0.001f)
            {
                transform.SetRotation(actualRotationEuler * glm::pi<float>() / 180.0f);
            }

            if (glm::length(actualScale - transform.GetScale()) > 0.001f)
            {
                transform.SetScale(actualScale);
            }
        }
    }

    ImGuizmo::OPERATION ImGUI::GetGuizmoMode()
    {
        if (!m_UseScale && !m_UseRotate && !m_UseTranslate)
        {
            return ImGuizmo::TRANSLATE;
        }
        else if (m_UseTranslate)
        {
            return ImGuizmo::TRANSLATE;
        }
        else if (m_UseRotate)
        {
            return ImGuizmo::ROTATE;
        }
        else if (m_UseScale)
        {
            return ImGuizmo::SCALE;
        }
        return ImGuizmo::TRANSLATE;
    }
}
