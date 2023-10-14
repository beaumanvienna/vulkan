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

#include <memory>

#include "renderer/model.h"

namespace GfxRenderEngine
{

    void Builder::LoadSkeletons(Material& material)
    {
        size_t numberOfSkeletons = m_GltfModel.skins.size();
        if (!numberOfSkeletons)
        {
            return;
        }

        // set material flag
        material.m_Features |= Material::HAS_SKELETAL_ANIMATION;

        // adjust the size of the skeleton std::vector to the number of skeletons
        m_Skeletons.resize(numberOfSkeletons);

        // loop over all skeletons from the glTF model and fill our skeleton std::vector
        for (size_t skeletonIndex = 0; skeletonIndex < numberOfSkeletons; ++skeletonIndex)
        {
            const tinygltf::Skin& glTFSkin = m_GltfModel.skins[skeletonIndex];

            // does it have information about joints?
            if (glTFSkin.inverseBindMatrices != GLTF_NOT_USED) // glTFSkin.inverseBindMatrices refers to an gltf accessor
            {
                auto& skeleton = m_Skeletons[skeletonIndex]; // just a reference to the skeleton to be set up (to make code easier)
                auto& joints = skeleton.m_Joints; // just a reference to the joints std::vector of that skeleton (to make code easier)

                // set up number of joints
                size_t numberOfJoints = glTFSkin.joints.size();
                // resize the joints vector of the skeleton object (to be filled)
                joints.resize(numberOfJoints);

                // set up name of skeleton
                skeleton.m_Name = glTFSkin.name;

                // retrieve array of inverse bind matrices of all joints
                // --> first, retrieve raw data and copy into a std::vector
                const tinygltf::Accessor&   accessor   = m_GltfModel.accessors[glTFSkin.inverseBindMatrices];
                const tinygltf::BufferView& bufferView = m_GltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer&     buffer     = m_GltfModel.buffers[bufferView.buffer];
                // assert # of matrices matches # of joints
                if (accessor.count != numberOfJoints) LOG_CORE_CRITICAL("accessor.count != numberOfJoints"); 

                auto& inverseBindMatrices = skeleton.m_InverseBindMatrices;
                inverseBindMatrices.resize(numberOfJoints);
                int bufferSize = accessor.count * sizeof(glm::mat4); // in bytes
                memcpy(inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], bufferSize);

                // loop over all joints from gltf model and fill the skeleton with joints
                for (size_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                {
                    int globalGltfNodeIndex = glTFSkin.joints[jointIndex];
                    joints[jointIndex].m_GlobalGltfNodeIndex   = globalGltfNodeIndex;
                    joints[jointIndex].m_InverseBindMatrix = inverseBindMatrices[jointIndex];

                    // set up map global node 2 joint index
                    skeleton.m_GlobalGltfNodeToJointIndex[globalGltfNodeIndex] = jointIndex;
                }

                int rootJoint = glTFSkin.joints[0]; // the here always works but the gltf field skins.skeleton can be ignored

                LoadJoint(skeleton, rootJoint, SkeletalAnimation::NO_PARENT);

                skeleton.Traverse();
            }

            // create a buffer to be used in the shader for the joint matrices
            // The gltf model has multiple animations, all applied to the same skeleton
            // --> all skeletons have the same size (and we can use m_Skeletons[0] to get the number of joints)
            int numberOfJoints = m_Skeletons[0].m_Joints.size();
            int bufferSize = numberOfJoints * sizeof(glm::mat4); // in bytes
            m_ShaderData = Buffer::Create(bufferSize);
        }
    }

    // recursive function via global gltf nodes (which have children)
    // tree structure links (local) skeleton joints
    void Builder::LoadJoint(SkeletalAnimation::Skeleton& skeleton, int globalGltfNodeIndex, int parentJoint)
    {
        int currentJoint = skeleton.m_GlobalGltfNodeToJointIndex[globalGltfNodeIndex];
        auto& joint = skeleton.m_Joints[currentJoint]; // a reference to the current joint

        joint.m_ParentJoint = parentJoint;

        //process children (if any)
        size_t numberOfChildren = m_GltfModel.nodes[globalGltfNodeIndex].children.size();
        if (numberOfChildren > 0)
        {
            joint.m_Children.resize(numberOfChildren);
            for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
            {
                uint globalGltfNodeIndexForChild = m_GltfModel.nodes[globalGltfNodeIndex].children[childIndex];
                joint.m_Children[childIndex] = skeleton.m_GlobalGltfNodeToJointIndex[globalGltfNodeIndexForChild];
                LoadJoint(skeleton, globalGltfNodeIndexForChild, currentJoint);
            }
        }
    }
}
