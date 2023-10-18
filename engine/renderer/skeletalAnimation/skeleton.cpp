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
#include "auxiliary/debug.h"

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
                //UpdateJoint(ROOT_JOINT);  // recursively updates skeleton
                if(1){
                    for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                    {
                        m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_Joints[jointIndex].GetDeformedBindMatrix();
                    }
                    for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                    {
                        int16_t parentJoint = m_Joints[jointIndex].m_ParentJoint;
                        if (parentJoint != Armature::NO_PARENT)
                        {
                            m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_ShaderData.m_FinalJointsMatrices[parentJoint] * m_ShaderData.m_FinalJointsMatrices[jointIndex];
                        }
                    }
                    for (int16_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                    {
                        m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_ShaderData.m_FinalJointsMatrices[jointIndex] * m_Joints[jointIndex].m_UndefomedInverseBindMatrix;
                    }
                }
            }
        }

        // Update the global bind matrices of all joints
        // traverses entire skeleton from top (a.k.a root)
        // This way, it is guaranteed that the global parent transform is already updated
        void Skeleton::UpdateJoint(int16_t jointIndex)
        {
            auto& currentJoint = m_Joints[jointIndex]; // just a reference for easier code
            // update this node
            UpdateFinalDeformedBindMatrix(jointIndex);

            // update children
            size_t numberOfChildern = currentJoint.m_Children.size();
            for (size_t childIndex = 0; childIndex < numberOfChildern; ++childIndex)
            {
                int childJoint = currentJoint.m_Children[childIndex];
                UpdateJoint(childJoint);
            }
        }

        // update the final deformed bind matrix for a joint 
        // from the parent (must be already final) and from the local deformation transform
        void Skeleton::UpdateFinalDeformedBindMatrix(int16_t jointIndex)
        {
            m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_Joints[jointIndex].GetDeformedBindMatrix();
            int16_t parentJoint = m_Joints[jointIndex].m_ParentJoint;
            if (parentJoint != Armature::NO_PARENT)
            {
                m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_ShaderData.m_FinalJointsMatrices[parentJoint] * m_ShaderData.m_FinalJointsMatrices[jointIndex];
            }
            m_ShaderData.m_FinalJointsMatrices[jointIndex] = m_ShaderData.m_FinalJointsMatrices[jointIndex] * m_Joints[jointIndex].m_UndefomedInverseBindMatrix;
        }
    }
}
