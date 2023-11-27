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


#include "gtc/type_ptr.hpp"
#include "gtx/matrix_decompose.hpp"

#include "core.h"
#include "scene/scene.h"
#include "scene/components.h"

#include "application/lucre/lucre.h"
#include "application/lucre/UI/imgui.h"

#include "auxiliary/file.h"
#include "renderer/model.h"

namespace LucreApp
{
    int   ImGUI::m_SelectedModel = 0;
    int   ImGUI::m_MaxModels = 0;
    EnttV ImGUI::m_VisibleModels;
    int   ImGUI::m_SelectedGameObject = 0;

    float ImGUI::m_Roughness = 0.1f;
    bool  ImGUI::m_UseRoughness = false;
    float ImGUI::m_Metallic = 0.5f;
    bool  ImGUI::m_UseMetallic = false;
    float ImGUI::m_NormalMapIntensity = 0.9f;
    bool  ImGUI::m_UseNormalMapIntensity = false;
    float ImGUI::m_PointLightIntensity = 1.0f;
    float ImGUI::m_AmbientLightIntensity = 0.1f;
    bool  ImGUI::m_UseAmbientLightIntensity = false;
    bool  ImGUI::m_UsePointLightIntensity = false;
    bool  ImGUI::m_UseScale = false;
    bool  ImGUI::m_UseRotate = false;
    bool  ImGUI::m_UseTranslate = false;
    bool  ImGUI::m_ShowDebugShadowMap = false;
    bool  ImGUI::m_UseEmissiveStrength = false;
    float ImGUI::m_EmissiveStrength = 0.35;
    bool  ImGUI::m_UseAnimation = false;
    bool  ImGUI::m_RepeatAnimation = false;

    void ImGUI::DebugWindow()
    {
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
        uint contextWidth  = Engine::m_Engine->GetWindowWidth();
        uint contextHeight = Engine::m_Engine->GetWindowHeight();

        auto  currentScene = Lucre::m_Application->GetScene();
        auto& camera       = currentScene->GetCamera();
        auto& registry     = currentScene->GetRegistry();

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
        std::string gameObjectLabel = "Model ";
        entt::entity entity = static_cast<entt::entity>(0);
        if (m_VisibleModels.size() > 0)
        {
            auto& [label, selectedModel] = m_VisibleModels[m_SelectedModel];
            entity = selectedModel;
            gameObjectLabel += label + std::string(", entity: ") + std::to_string(static_cast<int>(entity));
        }

        ImGui::SliderInt(gameObjectLabel.c_str(), &m_SelectedModel, 0, m_MaxModels);

        static int selectedModelPrevious = m_SelectedModel;
        static const char* currentItem = nullptr;
        if (m_SelectedModel != selectedModelPrevious)
        {
            // reset animations drop down
            selectedModelPrevious = m_SelectedModel;
            m_SelectedGameObject = 0;
            m_UseAnimation = false;
            m_RepeatAnimation = false;
            currentItem = nullptr;
        }

        {
            uint node = currentScene->GetTreeNodeIndex(entity);
            const uint maxDepth = 5;
            TraverseObjectTree(*currentScene, node, maxDepth);
        }

        if (registry.all_of<PbrMaterial>(entity))
        {
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

            // emission strength
            ImGui::Checkbox("use###006", &m_UseEmissiveStrength);
            ImGui::SameLine();
            ImGui::SliderFloat("emissive strength", &m_EmissiveStrength, 0.0f, 1.0f);
        }

        if (registry.all_of<SkeletalAnimationTag>(static_cast<entt::entity>(m_SelectedGameObject)))
        {
            auto& mesh = registry.get<MeshComponent>(static_cast<entt::entity>(m_SelectedGameObject));
            auto& animations = mesh.m_Model.get()->GetAnimations();
            size_t numberOfAnimations = animations.Size();
            std::vector<const char*> items(numberOfAnimations);
            uint itemIndex = 0;
            for (auto& animation : animations)
            {
                items[itemIndex++] = animation.GetName().c_str();
            }
            
            if (!currentItem)
            {
                currentItem = items[0];
            }
            ImGui::Checkbox("use###007", &m_UseAnimation);
            ImGui::SameLine();
            ImGui::Checkbox("repeat###001", &m_RepeatAnimation);
            ImGui::SameLine();
            if (ImGui::BeginCombo("##combo", currentItem)) // The 2nd parameter is the label previewed before opening the combo
            {
                for (size_t index = 0; index < numberOfAnimations; ++index)
                {
                    bool isSelected = (currentItem == items[index]);
                    if (ImGui::Selectable(items[index], isSelected))
                    {
                        currentItem = items[index];
                        if (m_UseAnimation)
                        {
                            animations.Start(currentItem);
                            animations.SetRepeat(m_RepeatAnimation);
                        }
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus(); // set initial focus when opening the combo (scrolling + for keyboard navigation support)
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            ImGui::Text("select animation");
        }

        auto guizmoMode = GetGuizmoMode();
        if (m_VisibleModels.size() > 0)
        {
            ImGuizmo::BeginFrame();
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(0, 0, contextWidth, contextHeight);

            auto projectionMatrix = glm::scale(glm::mat4(1.0f), {1.0f, -1.0f, 1.0f}) * camera.GetProjectionMatrix();
            auto& viewMatrix = camera.GetViewMatrix();

            if (m_UseEmissiveStrength)
            {
                bool found = false;
                {
                    if (registry.all_of<PbrEmissiveTag>(entity))
                    {
                        found = true;
                        auto& pbrEmissiveTag = registry.get<PbrEmissiveTag>(entity);
                        pbrEmissiveTag.m_EmissiveStrength = m_EmissiveStrength;
                    }
                }
                if (!found)
                {
                    if (registry.all_of<PbrEmissiveTextureTag>(entity))
                    {
                        auto& pbrEmissiveTextureTag = registry.get<PbrEmissiveTextureTag>(entity);
                        pbrEmissiveTextureTag.m_EmissiveStrength = m_EmissiveStrength;
                    }
                }
            }

            auto& transform = registry.get<TransformComponent>(entity);
            glm::mat4 mat4 = transform.GetMat4();

            ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
                guizmoMode, ImGuizmo::LOCAL, glm::value_ptr(mat4));

            glm::vec3 translation;
            glm::quat rotation;
            glm::vec3 scale;
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

            float minimumChange = 0.000001f;

            if (glm::length(actualTranslation - transform.GetTranslation()) > minimumChange)
            {
                transform.SetTranslation(actualTranslation);
            }

            if (glm::length(actualRotationEuler - (transform.GetRotation() * 180.0f / glm::pi<float>())) > minimumChange)
            {
                transform.SetRotation(actualRotationEuler * glm::pi<float>() / 180.0f);
            }

            if (glm::length(actualScale - transform.GetScale()) > minimumChange)
            {
                transform.SetScale(actualScale);
            }
        }
        // point light intensity
        ImGui::Checkbox("use###004", &m_UsePointLightIntensity);
        ImGui::SameLine();
        ImGui::SliderFloat("point lights", &m_PointLightIntensity, 0.0f, 10.0f);

        // ambient light intensity
        ImGui::Checkbox("use###005", &m_UseAmbientLightIntensity);
        ImGui::SameLine();
        ImGui::SliderFloat("ambient light", &m_AmbientLightIntensity, 0.0f, 1.0f);

        // shadow map debug window
        ImGui::Checkbox("show shadow map", &m_ShowDebugShadowMap);
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

    // set up maxGameObjects, and the std::vector for visibleGameObjects
    void ImGUI::SetupSlider(Scene* scene)
    {
        m_SelectedModel = 0;
        TreeNode& rootNode = scene->GetTreeNode(SceneGraph::ROOT_NODE);

        { // insert root
            auto& label = rootNode.GetName();
            auto entity = rootNode.GetGameObject();

            m_VisibleModels.push_back({label, entity});
        }

        // insert gltf files
        for (uint nodeIndex : rootNode.GetChildren())
        {
            TreeNode& node = scene->GetTreeNode(nodeIndex);
            auto& label = node.GetName();
            auto entity = node.GetGameObject();

            m_VisibleModels.push_back({label, entity});
        }

        m_MaxModels = m_VisibleModels.size()-1;
    }

    void ImGUI::TraverseObjectTree(Scene& scene, uint const nodeIndex, uint const maxDepth)
    {
        TraverseObjectTree(scene, nodeIndex, 0/*uint depth*/, maxDepth); // start with depth 0
    }

    void ImGUI::TraverseObjectTree(Scene& scene, uint const nodeIndex, uint const depth, uint const maxDepth)
    {
        if (depth < maxDepth)
        {
            TreeNode& node = scene.GetTreeNode(nodeIndex);
            int gameObject = static_cast<int>(node.GetGameObject());
            ImGui::PushID(gameObject);
            std::string label = "entity " + std::to_string(gameObject) + " " + node.GetName();

            uint numberOfChildren = node.Children();
            if ((numberOfChildren) && (depth != (maxDepth-1)))
            {
                if (ImGui::TreeNodeEx(label.c_str()))
                {
                    for (uint index = 0; index < node.Children(); ++index)
                    {
                        TraverseObjectTree(scene, node.GetChild(index), depth + 1, maxDepth);
                    }
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::TreeNodeEx(label.c_str(), /*ImGuiTreeNodeFlags_NoTreePushOnOpen | */ImGuiTreeNodeFlags_Leaf);
                ImGui::SameLine();
                if (ImGui::SmallButton("edit")) { m_SelectedGameObject = gameObject; }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }
}
