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
        std::unordered_map<std::string, int> nameToBoneIndex;

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
            for (uint boneIndex = 0; boneIndex < numberOfJoints; ++boneIndex)
            {
                aiBone* bone = mesh->mBones[boneIndex];
                std::string boneName = bone->mName.C_Str();
                nameToBoneIndex[boneName] = boneIndex;

                // compatibility code with glTF loader; needed in skeletalAnimation.cpp
                // m_Channels.m_Node must be set up accordingly
                m_Skeleton->m_GlobalNodeToJointIndex[boneIndex] = boneIndex;
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
                    joints[jointIndex].m_InverseBindMatrix = mat4AssetImporterToGlm(bone->mOffsetMatrix);
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

        size_t numberOfAnimations = m_FbxScene->mNumAnimations;
        for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
        {
            aiAnimation& fbxAnimation = *m_FbxScene->mAnimations[animationIndex];

            std::string animationName(fbxAnimation.mName.C_Str());
            // the asset importer includes animations twice,
            // as "armature|name" and "name"
            if (animationName.find("|") != std::string::npos)
            {
                continue;
            }
            std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(animationName);

            // animation speed
            double ticksPerSecond = 0.0;
            if (fbxAnimation.mTicksPerSecond > std::numeric_limits<float>::epsilon())
            {
                ticksPerSecond = fbxAnimation.mTicksPerSecond;
            }
            else
            {
                LOG_CORE_ERROR("no speed information found in fbx file");
                ticksPerSecond = 30.0;
            }

            {
                uint channelAndSamplerIndex = 0;
                uint numberOfFbxChannels = fbxAnimation.mNumChannels;
                for (uint fbxChannelIndex = 0; fbxChannelIndex < numberOfFbxChannels; ++fbxChannelIndex)
                {
                    aiNodeAnim& fbxChannel = *fbxAnimation.mChannels[fbxChannelIndex];
                    std::string fbxChannelName(fbxChannel.mNodeName.C_Str());

                    // use fbx channels that actually belong to bones
                    bool isBone = nameToBoneIndex.contains(fbxChannelName);
                    if (isBone)
                    {
                        // helper lambdas to convert asset importer formats to glm
                        auto vec3AssetImporterToGlm = [](aiVector3D const& vec3AssetImporter)
                        { return glm::vec3(vec3AssetImporter.x, vec3AssetImporter.y, vec3AssetImporter.z); };

                        auto quaternionAssetImporterToGlmVec4 = [](aiQuaternion const& quaternionAssetImporter)
                        {
                            glm::vec4 vec4GLM;
                            vec4GLM.x = quaternionAssetImporter.x;
                            vec4GLM.y = quaternionAssetImporter.y;
                            vec4GLM.z = quaternionAssetImporter.z;
                            vec4GLM.w = quaternionAssetImporter.w;

                            return vec4GLM;
                        };

                        // Each node of the skeleton has channels that point to samplers
                        { // set up channels
                            {
                                SkeletalAnimation::Channel channel;
                                channel.m_Path = SkeletalAnimation::Path::TRANSLATION;
                                channel.m_SamplerIndex = channelAndSamplerIndex + 0;
                                channel.m_Node = nameToBoneIndex[fbxChannelName];

                                animation->m_Channels.push_back(channel);
                            }
                            {
                                SkeletalAnimation::Channel channel;
                                channel.m_Path = SkeletalAnimation::Path::ROTATION;
                                channel.m_SamplerIndex = channelAndSamplerIndex + 1;
                                channel.m_Node = nameToBoneIndex[fbxChannelName];

                                animation->m_Channels.push_back(channel);
                            }
                            {
                                SkeletalAnimation::Channel channel;
                                channel.m_Path = SkeletalAnimation::Path::SCALE;
                                channel.m_SamplerIndex = channelAndSamplerIndex + 2;
                                channel.m_Node = nameToBoneIndex[fbxChannelName];

                                animation->m_Channels.push_back(channel);
                            }
                        }

                        { // set up samplers
                            {
                                uint numberOfKeys = fbxChannel.mNumPositionKeys;

                                SkeletalAnimation::Sampler sampler;
                                sampler.m_Timestamps.resize(numberOfKeys);
                                sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                                sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                                for (uint key = 0; key < numberOfKeys; ++key)
                                {
                                    aiVector3D& value = fbxChannel.mPositionKeys[key].mValue;
                                    sampler.m_TRSoutputValuesToBeInterpolated[key] =
                                        glm::vec4(vec3AssetImporterToGlm(value), 0.0f);
                                    sampler.m_Timestamps[key] = fbxChannel.mPositionKeys[key].mTime / ticksPerSecond;
                                }

                                animation->m_Samplers.push_back(sampler);
                            }
                            {
                                uint numberOfKeys = fbxChannel.mNumRotationKeys;

                                SkeletalAnimation::Sampler sampler;
                                sampler.m_Timestamps.resize(numberOfKeys);
                                sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                                sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                                for (uint key = 0; key < numberOfKeys; ++key)
                                {
                                    aiQuaternion& value = fbxChannel.mRotationKeys[key].mValue;
                                    sampler.m_TRSoutputValuesToBeInterpolated[key] = quaternionAssetImporterToGlmVec4(value);
                                    sampler.m_Timestamps[key] = fbxChannel.mPositionKeys[key].mTime / ticksPerSecond;
                                }

                                animation->m_Samplers.push_back(sampler);
                            }
                            {
                                uint numberOfKeys = fbxChannel.mNumScalingKeys;

                                SkeletalAnimation::Sampler sampler;
                                sampler.m_Timestamps.resize(numberOfKeys);
                                sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                                sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                                for (uint key = 0; key < numberOfKeys; ++key)
                                {
                                    aiVector3D& value = fbxChannel.mScalingKeys[key].mValue;
                                    sampler.m_TRSoutputValuesToBeInterpolated[key] =
                                        glm::vec4(vec3AssetImporterToGlm(value), 0.0f);
                                    sampler.m_Timestamps[key] = fbxChannel.mPositionKeys[key].mTime / ticksPerSecond;
                                }

                                animation->m_Samplers.push_back(sampler);
                            }
                        }
                        channelAndSamplerIndex += 3;
                    }
                }
            }

            if (animation->m_Samplers.size()) // at least one sampler found
            {
                auto& sampler = animation->m_Samplers[0];
                if (sampler.m_Timestamps.size() >= 2) // samplers have at least 2 keyframes to interpolate in between
                {
                    animation->SetFirstKeyFrameTime(sampler.m_Timestamps[0]);
                    animation->SetLastKeyFrameTime(sampler.m_Timestamps.back());
                }
            }

            m_Animations->Push(animation);
        }

        m_SkeletalAnimation = (m_Animations->Size()) ? true : false;
    }
} // namespace GfxRenderEngine
