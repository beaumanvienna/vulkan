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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <memory>

#include "engine.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imGuizmo/ImGuizmo.h"
#include "scene/sceneLoader.h"
#include "scene/gltf.h"

namespace LucreApp
{
    struct SliderEntry
    {
        std::string m_Label;
        entt::entity m_Entity;
    };

    using EnttV = std::vector<SliderEntry>;

    class ImGUI
    {

    public:
        static void DebugWindow();
        static void SetupSlider(Scene* scene);

    public:
        static int m_SelectedModel;
        static int m_SelectedModelPrevious;
        static int m_MaxModels;
        static EnttV m_VisibleModels;
        static int m_SelectedGameObject;
        static const char* m_CurrentItem;

        static float m_Roughness;
        static bool m_UseRoughness;
        static float m_Metallic;
        static bool m_UseMetallic;
        static float m_NormalMapIntensity;
        static bool m_UseNormalMapIntensity;
        static float m_PointLightIntensity;
        static bool m_UsePointLightIntensity;
        static bool m_UseAmbientLightIntensity;
        static float m_AmbientLightIntensity;
        static bool m_UseScale;
        static bool m_UseRotate;
        static bool m_UseTranslate;
        static bool m_ShowDebugShadowMap;
        static bool m_UseEmissiveStrength;
        static float m_EmissiveStrength;
        static bool m_UseAnimation;
        static bool m_RepeatAnimation;

    private:
        static void TraverseObjectTree(Scene& scene, uint const nodeIndex, uint const maxDepth);
        static void TraverseObjectTree(Scene& scene, uint const nodeIndex, uint const depth, uint const maxDepth);

    private:
        struct SliderEntry
        {
            std::string m_Name;
            entt::entity m_Entity;
        };

        static ImGuizmo::OPERATION GetGuizmoMode();
    };
} // namespace LucreApp
