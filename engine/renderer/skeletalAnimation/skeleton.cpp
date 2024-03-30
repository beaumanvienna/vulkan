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

#include "renderer/skeletalAnimation/skeleton.h"

namespace GfxRenderEngine
{
    namespace Armature
    {
        void Skeleton::Traverse()
        {
            LOG_CORE_WARN("Skeleton: {0}", m_Name);
            uint indent = 0;
            std::string indentStr(indent, ' ');
            auto& joint = m_Joints[0]; // root joint
            Traverse(joint, indent + 1);
        }

        void Skeleton::Traverse(Joint const& joint, uint indent)
        {
            std::string indentStr(indent, ' ');
            size_t numberOfChildren = joint.m_Children.size();
            LOG_CORE_INFO("{0}name: {1}, m_Parent: {2}, m_Children.size(): {3}", indentStr, joint.m_Name,
                          joint.m_ParentJoint, numberOfChildren);
            for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
            {
                int jointIndex = joint.m_Children[childIndex];
                LOG_CORE_INFO("{0}child {1}: index: {2}", indentStr, childIndex, jointIndex);
            }

            for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
            {
                int jointIndex = joint.m_Children[childIndex];
                Traverse(m_Joints[jointIndex], indent + 1);
            }
        }

        void Skeleton::Update()
        {
            // update the final global transform of all joints
            int16_t numberOfJoints = static_cast<int16_t>(m_Joints.size());

            if (!m_IsAnimated) // used for debugging to check if the model renders w/o deformation
            {
                for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                {
                    m_ShaderData.m_FinalJointsMatrices[jointIndex] = glm::mat4(1.0f);
                }
            }
            else
            {
                // STEP 1: apply animation results
                for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                {
                    m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_Joints[jointIndex].GetDeformedBindMatrix();
                }

                // STEP 2: recursively update final joint matrices
                UpdateJoint(ROOT_JOINT);

                // STEP 3: bring back into model space
                for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                {
                    m_ShaderData.m_FinalJointsMatrices[jointIndex] =
                        m_ShaderData.m_FinalJointsMatrices[jointIndex] * m_Joints[jointIndex].m_InverseBindMatrix;
                }
            }
        }

        // Update the final joint matrices of all joints
        // traverses entire skeleton from top (a.k.a root a.k.a hip bone)
        // This way, it is guaranteed that the global parent transform is already updated
        void Skeleton::UpdateJoint(int16_t jointIndex)
        {
            auto& currentJoint = m_Joints[jointIndex]; // just a reference for easier code

            int16_t parentJoint = currentJoint.m_ParentJoint;
            if (parentJoint != Armature::NO_PARENT)
            {
                m_ShaderData.m_FinalJointsMatrices[jointIndex] =
                    m_ShaderData.m_FinalJointsMatrices[parentJoint] * m_ShaderData.m_FinalJointsMatrices[jointIndex];
            }

            // update children
            size_t numberOfChildren = currentJoint.m_Children.size();
            for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
            {
                int childJoint = currentJoint.m_Children[childIndex];
                UpdateJoint(childJoint);
            }
        }
    } // namespace Armature
} // namespace GfxRenderEngine
