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

#include "gtc/quaternion.hpp"
#include "gtx/quaternion.hpp"
#include "gtc/type_ptr.hpp"

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
                skeleton.m_ShaderData.m_FinalJointsMatrices.resize(numberOfJoints);

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

                    // set up node transform (either TRS or from directy from "matrix")
                    // the fields are set to identity matrices in the constructor
                    // in case it cannot be found in the gltf model
                    auto& gltfNode = m_GltfModel.nodes[globalGltfNodeIndex];

                    if (gltfNode.translation.size() == 3) // std::vector<double> gltfmodel.node.translation; // length must be 0 or 3
                    {
                        glm::vec3 t = glm::make_vec3(gltfNode.translation.data());
                        joints[jointIndex].m_NodeTranslation = glm::translate(glm::mat4(1.0f), t);
                    }

                    if (gltfNode.rotation.size() == 4) // std::vector<double> gltfmodel.node.rotation; // length must be 0 or 4
                    {
                        glm::quat q = glm::make_quat(gltfNode.rotation.data());
                        joints[jointIndex].m_NodeRotation = glm::toMat4(q);
                    }

                    if (gltfNode.scale.size() == 3) // std::vector<double> gltfmodel.node.scale; // length must be 0 or 3
                    {
                        glm::vec3 s = glm::make_vec3(gltfNode.scale.data());
                        joints[jointIndex].m_NodeScale = glm::scale(glm::mat4(1.0f), s);
                    }

                    if (gltfNode.matrix.size() == 16) // std::vector<double> gltfmodel.node.matrix; // length must be 0 or 16
                    {
                        joints[jointIndex].m_NodeMatrix = glm::make_mat4x4(gltfNode.matrix.data());
                    }

                    // set up map "global node" to "joint index"
                    skeleton.m_GlobalGltfNodeToJointIndex[globalGltfNodeIndex] = jointIndex;
                }

                int rootJoint = glTFSkin.joints[0]; // the here always works but the gltf field skins.skeleton can be ignored

                LoadJoint(skeleton, rootJoint, Armature::NO_PARENT);

                skeleton.Traverse();
            }

            // create a buffer to be used in the shader for the joint matrices
            // The gltf model has multiple animations, all applied to the same skeleton
            // --> all skeletons have the same size (and we can use m_Skeletons[0] to get the number of joints)
            int numberOfJoints = m_Skeletons[0].m_Joints.size();
            int bufferSize = numberOfJoints * sizeof(glm::mat4); // in bytes
            m_ShaderData = Buffer::Create(bufferSize);
        }

        size_t numberOfAnimations = m_GltfModel.animations.size();
        for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
        {
            auto& gltfAnimation = m_GltfModel.animations[animationIndex];
            std::string name = gltfAnimation.name;
            std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(name);

            // Samplers
            size_t numberOfSamplers = gltfAnimation.samplers.size();
            std::vector<SkeletalAnimation::Sampler> samplers(numberOfSamplers);
            for (size_t samplerIndex = 0; samplerIndex < numberOfSamplers; ++samplerIndex)
            {
                tinygltf::AnimationSampler glTFSampler = gltfAnimation.samplers[samplerIndex];
                auto& sampler  = samplers[samplerIndex];
                
                sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                if (glTFSampler.interpolation == "STEP")
                {
                    sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::STEP;
                }
                else if (glTFSampler.interpolation == "CUBICSPLINE")
                {
                    sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::CUBICSPLINE;
                }

                // get timestamp
                {
                    const tinygltf::Accessor&   accessor   = m_GltfModel.accessors[glTFSampler.input];
                    const tinygltf::BufferView& bufferView = m_GltfModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer&     buffer     = m_GltfModel.buffers[bufferView.buffer];
                    int timestampBufferDataType = accessor.componentType;

                    if (timestampBufferDataType == GL_FLOAT)
                    {
                        const float* timestampBuffer = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (int index = 0; index < accessor.count; ++index)
                        {
                            sampler.m_Timestamp = timestampBuffer[index];
                        }
                    }
                    else
                    {
                        LOG_CORE_INFO("Builder::LoadSkeletons: cannot handle timestamp format");
                    }
                }

                // Read sampler keyframe output translate/rotate/scale values
                {
                    const tinygltf::Accessor&   accessor   = m_GltfModel.accessors[glTFSampler.output];
                    const tinygltf::BufferView& bufferView = m_GltfModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer&     buffer     = m_GltfModel.buffers[bufferView.buffer];

                    switch (accessor.type)
                    {
                        case TINYGLTF_TYPE_VEC3:
                        {
                            const glm::vec3* outputBuffer = reinterpret_cast<const glm::vec3*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                sampler.m_TRSoutputValuesToBeInterpolated = glm::vec4(outputBuffer[index], 0.0f);
                            }
                            break;
                        }
                        case TINYGLTF_TYPE_VEC4:
                        {
                            const glm::vec4* outputBuffer = reinterpret_cast<const glm::vec4*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++)
                            {
                                sampler.m_TRSoutputValuesToBeInterpolated = glm::vec4(outputBuffer[index]);
                            }
                            break;
                        }
                        default: {
                            LOG_CORE_CRITICAL("void Builder::LoadSkeletons(...): accessor type not found");
                            break;
                        }
                    }
                }
            }

            // Channels
            //animations[i].channels.resize(gltfAnimation.channels.size());
            //for (size_t channelIndex = 0; channelIndex < gltfAnimation.channels.size(); ++channelIndex)
            //{
            //    tinygltf::AnimationChannel glTFChannel = gltfAnimation.channels[channelIndex];
            //    AnimationChannel &         dstChannel  = animations[i].channels[channelIndex];
            //    dstChannel.path                        = glTFChannel.target_path;
            //    dstChannel.samplerIndex                = glTFChannel.sampler;
            //    dstChannel.node                        = nodeFromIndex(glTFChannel.target_node);
            //}

            m_Animations.Push(animation);
        }
        
        if (m_Animations.Size()) material.m_Features |= Material::HAS_SKELETAL_ANIMATION;
    }

    // recursive function via global gltf nodes (which have children)
    // tree structure links (local) skeleton joints
    void Builder::LoadJoint(Armature::Skeleton& skeleton, int globalGltfNodeIndex, int parentJoint)
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
