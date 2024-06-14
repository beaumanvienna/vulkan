/* Engine Copyright (c) 2023 Engine Development Team
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

#include <iostream>
#include <map>

#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"

#include "engine.h"
#include "renderer/skeletalAnimation/joints.h"

namespace GfxRenderEngine
{
    namespace Armature
    {
        static constexpr int NO_PARENT = -1;
        static constexpr int ROOT_JOINT = 0;

        struct ShaderData
        {
            std::vector<glm::mat4> m_FinalJointsMatrices;
        };

        struct Joint
        {
            std::string m_Name;
            glm::mat4 m_InverseBindMatrix; // a.k.a undeformed inverse node matrix

            // deformed / animated
            // to be applied to the node matrix a.k.a bind matrix in the world coordinate system,
            // controlled by an animation or a single pose (they come out of gltf animation samplers)
            glm::vec3 m_DeformedNodeTranslation{0.0f};                            // T
            glm::quat m_DeformedNodeRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // R
            glm::vec3 m_DeformedNodeScale{1.0f};                                  // S

            glm::mat4 GetDeformedBindMatrix()
            {
                // apply scale, rotation, and translation IN THAT ORDER (read from right to the left)
                // to the original undefomed bind matrix
                // dynamically called once per frame
                return glm::translate(glm::mat4(1.0f), m_DeformedNodeTranslation) * // T
                       glm::mat4(m_DeformedNodeRotation) *                          // R
                       glm::scale(glm::mat4(1.0f), m_DeformedNodeScale);            // S
            }

            // parents and children for the tree hierachy
            int m_ParentJoint;
            std::vector<int> m_Children;
        };

        struct Skeleton
        {
            void Traverse();
            void Traverse(Joint const& joint, uint indent = 0);
            void Update();
            void UpdateJoint(int16_t joint); // signed because -1 maybe used for invalid joint

            bool m_IsAnimated = true;
            std::string m_Name;
            std::vector<Joint> m_Joints;
            std::map<int, int> m_GlobalNodeToJointIndex;
            ShaderData m_ShaderData;
        };
    } // namespace Armature

} // namespace GfxRenderEngine
