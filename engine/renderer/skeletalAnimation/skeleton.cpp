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
            Traverse(joint, indent+1);
        }

        void Skeleton::Traverse(Joint const& joint, uint indent)
        {
            std::string indentStr(indent, ' ');
            size_t numberOfChildern = joint.m_Children.size();
            LOG_CORE_INFO
            (
                            "{0}m_GlobalGltfNodeIndex: {1}, localIndex: {2}, m_Parent: {3}, m_Children.size(): {4}", 
                            indentStr,
                            joint.m_GlobalGltfNodeIndex,
                            m_GlobalGltfNodeToJointIndex[joint.m_GlobalGltfNodeIndex],
                            joint.m_ParentJoint,
                            numberOfChildern
            );
            for (size_t childIndex = 0; childIndex < numberOfChildern; ++childIndex)
            {
                int jointIndex = joint.m_Children[childIndex];
                Traverse(m_Joints[jointIndex], indent+1);
            }
        }

        void Skeleton::Update()
        {
            UpdateJoints(m_Joints[0]);
        }

        void Skeleton::UpdateJoints(Joint& joint)
        {
            glm::mat4 inverseTransform = glm::inverse(GetNodeMatrix(joint));

            size_t numberOfJoints = m_Joints.size();
            for (size_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
            {
                m_ShaderData.m_FinalJointsMatrices[jointIndex] = GetNodeMatrix(m_Joints[jointIndex]) * m_InverseBindMatrices[jointIndex];
                m_ShaderData.m_FinalJointsMatrices[jointIndex] = inverseTransform * m_ShaderData.m_FinalJointsMatrices[jointIndex];
            }

            size_t numberOfChildern = joint.m_Children.size();
            for (size_t childIndex = 0; childIndex < numberOfChildern; ++childIndex)
            {
                int jointIndex = joint.m_Children[childIndex];
                UpdateJoints(m_Joints[jointIndex]);
            }
        }

        // Traverse skeleton to the root to get the local matrix in world space
        glm::mat4 Skeleton::GetNodeMatrix(Joint& joint)
        {
            glm::mat4 nodeMatrix = joint.m_NodeMatrix;
            int parentJoint = joint.m_ParentJoint;
            while (parentJoint != Armature::NO_PARENT)
            {
                nodeMatrix    = m_Joints[parentJoint].m_NodeMatrix * nodeMatrix;
                parentJoint   = m_Joints[parentJoint].m_ParentJoint;
            }
            return nodeMatrix;
        }
    }
}
