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

#include "engine.h"
#include "renderer/buffer.h"
#include "renderer/skeletalAnimation/joints.h"

namespace GfxRenderEngine
{
    namespace Armature
    {
        static constexpr int NO_PARENT = -1;

        struct ShaderData
        {
            std::vector<glm::mat4> m_FinalJointsMatrices;
        };

        struct Joint
        {
            // the joint
            int m_GlobalGltfNodeIndex;
            glm::mat4 m_InverseBindMatrix;

            // world space transform
            glm::mat4 m_NodeTranslation{1.0f};
            glm::mat4 m_NodeRotation{1.0f};
            glm::mat4 m_NodeScale{1.0f};
            glm::mat4 m_NodeMatrix{1.0f};

            // parents and children for the tree hierachy
            int m_ParentJoint;
            std::vector<int> m_Children;
        };

        struct Skeleton
        {
            void Traverse();
            void Traverse(Joint const& joint, uint indent = 0);
            void Update();
            void UpdateJoints(Joint& joint);
            glm::mat4 GetNodeMatrix(Joint& joint);

            std::string                 m_Name;
            std::vector<Joint>          m_Joints;
            std::vector<glm::mat4>      m_InverseBindMatrices;
            std::map<int, int>          m_GlobalGltfNodeToJointIndex;
            ShaderData                  m_ShaderData;
        };
    }

}
