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

#include "renderer/builder/ufbxBuilder.h"

namespace GfxRenderEngine
{
    void UFbxBuilder::LoadSkeletonsFbx()
    {
        uint numberOfSkeletons = 0;
        uint meshIndex = 0;
        // iterate over all meshes and check if they have a skeleton
        for (uint index = 0; index < m_FbxScene->meshes.count; ++index)
        {
            ufbx_mesh& mesh = *m_FbxScene->meshes.data[index];
            if (mesh.skin_deformers.count)
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
            LOG_CORE_WARN("A model should only have a single skin/armature/skeleton. Using mesh {0}.",
                          numberOfSkeletons - 1);
        }

        m_Animations = std::make_shared<SkeletalAnimations>();
        m_Skeleton = std::make_shared<Armature::Skeleton>();
        std::unordered_map<std::string, int> nameToBoneIndex;

        // load skeleton
        {
            ufbx_mesh& mesh = *m_FbxScene->meshes.data[meshIndex];
            ufbx_skin_deformer& fbxSkin = *mesh.skin_deformers.data[0];
            size_t numberOfBones = fbxSkin.clusters.count;
            auto& bones =
                m_Skeleton->m_Joints; // just a reference to the bones std::vector of that skeleton (to make code easier)

            bones.resize(numberOfBones);
            m_Skeleton->m_ShaderData.m_FinalJointsMatrices.resize(numberOfBones);

            // set up map to find the names of Bones when traversing the node hierarchy
            // by iterating the clsuetrs array of the mesh
            for (uint boneIndex = 0; boneIndex < numberOfBones; ++boneIndex)
            {
                ufbx_skin_cluster& bone = *fbxSkin.clusters.data[boneIndex];
                std::string boneName = bone.name.data;
                nameToBoneIndex[boneName] = boneIndex;

                // compatibility code with glTF loader; needed in skeletalAnimation.cpp
                // m_Channels.m_Node must be set up accordingly
                m_Skeleton->m_GlobalNodeToJointIndex[boneIndex] = boneIndex;
            }

            // lambda to convert ufbx_matrix to glm::mat4
            auto mat4UfbxToGlm = [](ufbx_matrix const& mat4Ufbx)
            {
                glm::mat4 mat4Glm;
                for (uint column = 0; column < 4; ++column)
                {
                    mat4Glm[column].x = mat4Ufbx.cols[column].x;
                    mat4Glm[column].y = mat4Ufbx.cols[column].y;
                    mat4Glm[column].z = mat4Ufbx.cols[column].z;
                    mat4Glm[column].w = column < 3 ? 0.0f : 1.0f;
                }
                return mat4Glm;
            };

            // recursive lambda to traverse fbx node hierarchy
            std::function<void(ufbx_node*, uint)> traverseNodeHierarchy = [&](ufbx_node* node, uint parent)
            {
                size_t numberOfChildren = node->children.count;
                uint boneIndex = parent;
                // does the node name correspond to a bone name?
                std::string nodeName = node->name.data;
                bool isBone = nameToBoneIndex.contains(nodeName);

                if (isBone)
                {
                    boneIndex = nameToBoneIndex[nodeName];
                    bones[boneIndex].m_Name = nodeName;
                    ufbx_skin_cluster& bone = *fbxSkin.clusters.data[boneIndex];
                    bones[boneIndex].m_InverseBindMatrix = mat4UfbxToGlm(bone.geometry_to_bone);
                    bones[boneIndex].m_ParentJoint = parent;
                }
                for (uint childIndex = 0; childIndex < numberOfChildren; ++childIndex)
                {
                    if (isBone)
                    {
                        std::string childNodeName = node->children.data[childIndex]->name.data;
                        bool childIsBone = nameToBoneIndex.contains(childNodeName);
                        if (childIsBone)
                        {
                            bones[boneIndex].m_Children.push_back(nameToBoneIndex[childNodeName]);
                        }
                    }
                    traverseNodeHierarchy(node->children.data[childIndex], boneIndex);
                }
            };
            traverseNodeHierarchy(m_FbxScene->root_node, Armature::NO_PARENT);
            // m_Skeleton->Traverse();

            int bufferSize = numberOfBones * sizeof(glm::mat4); // in bytes
            m_ShaderData = Buffer::Create(bufferSize);
            m_ShaderData->MapBuffer();
        }

        size_t numberOfAnimations = m_FbxScene->anim_stacks.count;
        for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
        {
            ufbx_anim_stack& fbxAnimation = *m_FbxScene->anim_stacks.data[animationIndex];

            std::string animationName(fbxAnimation.name.data);
            // the fbx includes animations twice,
            // as "armature|name" and "name"
            if (animationName.find("|") != std::string::npos)
            {
                continue;
            }
            LOG_CORE_INFO("name of animation: {0}", animationName);
            std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(animationName);

            animation->SetFirstKeyFrameTime(fbxAnimation.time_begin);
            animation->SetLastKeyFrameTime(fbxAnimation.time_end);

            ufbx_bake_opts bakeOptions = {};
            ufbx_error ufbxError;
            ufbx_unique_ptr<ufbx_baked_anim> fbxBakedAnim{
                ufbx_bake_anim(m_FbxScene, fbxAnimation.anim, &bakeOptions, &ufbxError)};
            if (!fbxBakedAnim)
            {
                char errorBuffer[512];
                ufbx_format_error(errorBuffer, sizeof(errorBuffer), &ufbxError);
                CORE_ASSERT(false, "failed to bake animation, " + std::string(errorBuffer));
                return;
            }

            // helper lambdas to convert asset importer formats to glm
            auto vec3UfbxToGlm = [](ufbx_vec3 const& vec3Ufbx) { return glm::vec3(vec3Ufbx.x, vec3Ufbx.y, vec3Ufbx.z); };
            auto quaternionUfbxToGlmVec4 = [](ufbx_quat const& quaternionUfbx)
            {
                glm::vec4 vec4GLM;
                vec4GLM.x = quaternionUfbx.x;
                vec4GLM.y = quaternionUfbx.y;
                vec4GLM.z = quaternionUfbx.z;
                vec4GLM.w = quaternionUfbx.w;
                return vec4GLM;
            };

            uint channelAndSamplerIndex = 0;
            for (const ufbx_baked_node& fbxChannel : fbxBakedAnim->nodes)
            {
                const uint nodeIndex = fbxChannel.typed_id;
                std::string fbxChannelName(m_FbxScene->nodes[nodeIndex]->name.data);
                // use fbx channels that actually belong to bones
                bool isBone = nameToBoneIndex.contains(fbxChannelName);
                if (isBone)
                {
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
                            uint numberOfKeys = fbxChannel.translation_keys.count;

                            SkeletalAnimation::Sampler sampler;
                            sampler.m_Timestamps.resize(numberOfKeys);
                            sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                            sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                            for (uint key = 0; key < numberOfKeys; ++key)
                            {
                                ufbx_vec3& value = fbxChannel.translation_keys.data[key].value;
                                sampler.m_TRSoutputValuesToBeInterpolated[key] = glm::vec4(vec3UfbxToGlm(value), 0.0f);
                                sampler.m_Timestamps[key] = fbxChannel.translation_keys.data[key].time;
                            }

                            animation->m_Samplers.push_back(sampler);
                        }
                        {
                            uint numberOfKeys = fbxChannel.rotation_keys.count;

                            SkeletalAnimation::Sampler sampler;
                            sampler.m_Timestamps.resize(numberOfKeys);
                            sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                            sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                            for (uint key = 0; key < numberOfKeys; ++key)
                            {
                                ufbx_quat& value = fbxChannel.rotation_keys.data[key].value;
                                sampler.m_TRSoutputValuesToBeInterpolated[key] = quaternionUfbxToGlmVec4(value);
                                sampler.m_Timestamps[key] = fbxChannel.rotation_keys.data[key].time;
                            }

                            animation->m_Samplers.push_back(sampler);
                        }
                        {
                            uint numberOfKeys = fbxChannel.scale_keys.count;

                            SkeletalAnimation::Sampler sampler;
                            sampler.m_Timestamps.resize(numberOfKeys);
                            sampler.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);
                            sampler.m_Interpolation = SkeletalAnimation::InterpolationMethod::LINEAR;
                            for (uint key = 0; key < numberOfKeys; ++key)
                            {
                                ufbx_vec3& value = fbxChannel.scale_keys.data[key].value;
                                sampler.m_TRSoutputValuesToBeInterpolated[key] = glm::vec4(vec3UfbxToGlm(value), 0.0f);
                                sampler.m_Timestamps[key] = fbxChannel.scale_keys.data[key].time;
                            }

                            animation->m_Samplers.push_back(sampler);
                        }
                    }
                    channelAndSamplerIndex += 3;
                }
            }

            m_Animations->Push(animation);
        }

        m_SkeletalAnimation = (m_Animations->Size()) ? true : false;
    }
} // namespace GfxRenderEngine
