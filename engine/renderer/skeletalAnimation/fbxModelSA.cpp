/* Engine Copyright (c) 2024 Engine Development Team
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
#include "gtc/type_ptr.hpp"
#include "gtx/quaternion.hpp"

#include "renderer/builder/fbxBuilder.h"

namespace GfxRenderEngine
{
    void FbxBuilder::LoadSkeletonsFbx()
    {
        uint numberOfSkeletons = 0;
        uint meshIndex = 0;
        // iterate over all meshes and check if they have a skeleton
        for (uint index = 0; index < m_FbxScene->mNumMeshes; ++index)
        {
            aiMesh* mesh = m_FbxScene->mMeshes[index];
            if (mesh->mNumBones)
            {
                ++numberOfSkeletons;
                meshIndex = index;
            }
        }
        if (!numberOfSkeletons)
        {
            return;
        }

        if (numberOfSkeletons > 1)
        {
            LOG_CORE_WARN("A model should only have a single skin/armature/skeleton. Using skin {0}.",
                          numberOfSkeletons - 1);
        }

        m_Animations = std::make_shared<SkeletalAnimations>();
        m_Skeleton = std::make_shared<Armature::Skeleton>();

        // load skeleton
        {

            aiMesh* mesh = m_FbxScene->mMeshes[meshIndex];
            size_t numberOfJoints = mesh->mNumBones;
            auto& joints =
                m_Skeleton->m_Joints; // just a reference to the joints std::vector of that skeleton (to make code easier)

            joints.resize(numberOfJoints);
            m_Skeleton->m_ShaderData.m_FinalJointsMatrices.resize(numberOfJoints);

            // set up map to find the names of bones when traversing the node hierarchy
            // by iterating the mBones array of the mesh
            std::unordered_map<std::string, int> nameToBoneIndex;
            for (uint boneIndex = 0; boneIndex < numberOfJoints; ++boneIndex)
            {
                aiBone* bone = mesh->mBones[boneIndex];
                std::string boneName = bone->mName.C_Str();
                nameToBoneIndex[boneName] = boneIndex;
            }

            // lambda to convert aiMatrix4x4 to glm::mat4
            auto mat4AssetImporterToGlm = [](aiMatrix4x4 const& mat4AssetImporter)
            {
                glm::mat4 mat4Glm;
                for (uint glmRow = 0; glmRow < 4; ++glmRow)
                {
                    for (uint glmColumn = 0; glmColumn < 4; ++glmColumn)
                    {
                        mat4Glm[glmColumn][glmRow] = mat4AssetImporter[glmRow][glmColumn];
                    }
                }
                return mat4Glm;
            };

            // recursive lambda to traverse fbx node hierarchy
            std::function<void(aiNode*, uint&, int)> traverseNodeHierarchy = [&](aiNode* node, uint& jointIndex, int parent)
            {
                size_t numberOfChildren = node->mNumChildren;

                // does the node name correspond to a bone name?
                std::string nodeName = node->mName.C_Str();
                bool isBone = nameToBoneIndex.contains(nodeName);

                int parentForChildren = parent;
                if (isBone)
                {
                    parentForChildren = jointIndex;
                    joints[jointIndex].m_Name = nodeName;
                    uint boneIndex = nameToBoneIndex[nodeName];
                    aiBone* bone = mesh->mBones[boneIndex];
                    m_Skeleton->m_ShaderData.m_FinalJointsMatrices[jointIndex] = mat4AssetImporterToGlm(bone->mOffsetMatrix);

                    joints[jointIndex].m_ParentJoint = parent;
                    ++jointIndex;
                }
                for (uint childIndex = 0; childIndex < numberOfChildren; ++childIndex)
                {
                    if (isBone)
                    {
                        std::string childNodeName = node->mChildren[childIndex]->mName.C_Str();
                        bool childIsBone = nameToBoneIndex.contains(childNodeName);
                        if (childIsBone)
                        {
                            joints[parentForChildren].m_Children.push_back(jointIndex);
                        }
                    }
                    traverseNodeHierarchy(node->mChildren[childIndex], jointIndex, parentForChildren);
                }
            };
            uint jointIndex = 0;
            traverseNodeHierarchy(m_FbxScene->mRootNode, jointIndex, Armature::NO_PARENT);
            // m_Skeleton->Traverse();

            int bufferSize = numberOfJoints * sizeof(glm::mat4); // in bytes
            m_ShaderData = Buffer::Create(bufferSize);
            m_ShaderData->MapBuffer();
        }

        // size_t numberOfAnimations = m_GltfModel.animations.size();
        // for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
        //{
        //     auto& gltfAnimation = m_GltfModel.animations[animationIndex];
        //     std::string name(gltfAnimation.name);
        //     LOG_CORE_INFO("name of animation: {0}", name);
        //     std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(name);
        //
        //     // Samplers
        //     size_t numberOfSamplers = gltfAnimation.samplers.size();
        //     animation->m_Samplers.resize(numberOfSamplers);
        //     for (size_t samplerIndex = 0; samplerIndex < numberOfSamplers; ++samplerIndex)
        //     {
        //         fastgltf::AnimationSampler glTFSampler = gltfAnimation.samplers[samplerIndex];
        //         auto& sampler = animation->m_Samplers[samplerIndex];
        //
        //         sampler.m_Interpolation =
        //         static_cast<SkeletalAnimation::InterpolationMethod>(glTFSampler.interpolation);
        //
        //         // get timestamp
        //         {
        //             size_t count = 0;
        //             const float* timestampBuffer;
        //             fastgltf::ComponentType componentType =
        //                 LoadAccessor<float>(m_GltfModel.accessors[glTFSampler.inputAccessor], timestampBuffer,
        //                 &count);
        //
        //             if (fastgltf::getGLComponentType(componentType) == GL_FLOAT)
        //             {
        //                 sampler.m_Timestamps.resize(count);
        //                 for (size_t index = 0; index < count; ++index)
        //                 {
        //                     sampler.m_Timestamps[index] = timestampBuffer[index];
        //                 }
        //             }
        //             else
        //             {
        //                 CORE_ASSERT(false, "FbxBuilder::LoadSkeletonsGltf: cannot handle timestamp format");
        //             }
        //         }
        //
        //         // Read sampler keyframe output translate/rotate/scale values
        //         {
        //             size_t count = 0;
        //             fastgltf::AccessorType type;
        //             const uint* buffer;
        //             LoadAccessor<uint>(m_GltfModel.accessors[glTFSampler.outputAccessor], buffer, &count, &type);
        //
        //             switch (type)
        //             {
        //                 case fastgltf::AccessorType::Vec3:
        //                 {
        //                     const glm::vec3* outputBuffer = reinterpret_cast<const glm::vec3*>(buffer);
        //                     sampler.m_TRSoutputValuesToBeInterpolated.resize(count);
        //                     for (size_t index = 0; index < count; index++)
        //                     {
        //                         sampler.m_TRSoutputValuesToBeInterpolated[index] = glm::vec4(outputBuffer[index],
        //                         0.0f);
        //                     }
        //                     break;
        //                 }
        //                 case fastgltf::AccessorType::Vec4:
        //                 {
        //                     const glm::vec4* outputBuffer = reinterpret_cast<const glm::vec4*>(buffer);
        //                     sampler.m_TRSoutputValuesToBeInterpolated.resize(count);
        //                     for (size_t index = 0; index < count; index++)
        //                     {
        //                         sampler.m_TRSoutputValuesToBeInterpolated[index] = glm::vec4(outputBuffer[index]);
        //                     }
        //                     break;
        //                 }
        //                 default:
        //                 {
        //                             CORE_ASSERT(false, "void FbxBuilder::LoadSkeletonsGltf(...): accessor type not
        //    found"); break;
        //                 }
        //             }
        //         }
        //     }
        //     if (animation->m_Samplers.size()) // at least one sampler found
        //     {
        //         auto& sampler = animation->m_Samplers[0];
        //         if (sampler.m_Timestamps.size() >= 2) // samplers have at least 2 keyframes to interpolate in
        //             between
        //             {
        //                 animation->SetFirstKeyFrameTime(sampler.m_Timestamps[0]);
        //                 animation->SetLastKeyFrameTime(sampler.m_Timestamps.back());
        //             }
        //     }
        //     // Each node of the skeleton has channels that point to samplers
        //     size_t numberOfChannels = gltfAnimation.channels.size();
        //     animation->m_Channels.resize(numberOfChannels);
        //     for (size_t channelIndex = 0; channelIndex < numberOfChannels; ++channelIndex)
        //     {
        //         fastgltf::AnimationChannel glTFChannel = gltfAnimation.channels[channelIndex];
        //         SkeletalAnimation::Channel& channel = animation->m_Channels[channelIndex];
        //         channel.m_SamplerIndex = glTFChannel.samplerIndex;
        //         CORE_ASSERT(glTFChannel.nodeIndex.has_value(), "FbxBuilder::LoadSkeletonsGltf() no target node");
        //         channel.m_Node = glTFChannel.nodeIndex.value();
        //         switch (glTFChannel.path)
        //         {
        //             case fastgltf::AnimationPath::Scale:
        //             {
        //                 channel.m_Path = SkeletalAnimation::Path::SCALE;
        //                 break;
        //             }
        //             case fastgltf::AnimationPath::Rotation:
        //             {
        //                 channel.m_Path = SkeletalAnimation::Path::ROTATION;
        //                 break;
        //             }
        //             case fastgltf::AnimationPath::Translation:
        //             {
        //                 channel.m_Path = SkeletalAnimation::Path::TRANSLATION;
        //                 break;
        //             }
        //             default:
        //             {
        //                 LOG_CORE_CRITICAL("path not supported");
        //                 break;
        //             }
        //         }
        //     }
        //     m_Animations->Push(animation);
        // }

        if (m_Animations->Size())
            m_SkeletalAnimation = Material::HAS_SKELETAL_ANIMATION;
    }

    // recursive function via global gltf nodes (which have children)
    // tree structure links (local) skeleton joints
    void FbxBuilder::LoadJoint(int globalFbxNodeIndex, int parentJoint)
    {
        // int currentJoint = m_Skeleton->m_GlobalGltfNodeToJointIndex[globalGltfNodeIndex];
        // auto& joint = m_Skeleton->m_Joints[currentJoint]; // a reference to the current joint
        //
        // joint.m_ParentJoint = parentJoint;
        //
        //// process children (if any)
        // size_t numberOfChildren = m_GltfModel.nodes[globalGltfNodeIndex].children.size();
        // if (numberOfChildren > 0)
        //{
        //     joint.m_Children.resize(numberOfChildren);
        //     for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
        //     {
        //         uint globalGltfNodeIndexForChild = m_GltfModel.nodes[globalGltfNodeIndex].children[childIndex];
        //         joint.m_Children[childIndex] = m_Skeleton->m_GlobalGltfNodeToJointIndex[globalGltfNodeIndexForChild];
        //         LoadJoint(globalGltfNodeIndexForChild, currentJoint);
        //     }
        // }
    }
} // namespace GfxRenderEngine
