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
#include "renderer/builder/fastgltfBuilder.h"
#include "renderer/skeletalAnimation/skeleton.h"
#include "renderer/model.h"

namespace GfxRenderEngine
{

    void FastgltfBuilder::LoadSkeletonsGltf()
    {

        size_t numberOfSkeletons = m_GltfAsset.skins.size();
        if (!numberOfSkeletons)
        {
            return;
        }
        ZoneScopedNC("FastgltfBuilder::LoadSkeletonsGltf", 0x00ff00);
        if (numberOfSkeletons > 1)
        {
            LOG_CORE_WARN("A model should only have a single skin/armature/skeleton. Using skin 0.");
        }

        m_Animations = std::make_shared<SkeletalAnimations>();
        m_Skeleton = std::make_shared<Armature::Skeleton>();

        // use skeleton 0 from the glTF model to fill the skeleton
        {
            const fastgltf::Skin& glTFSkin = m_GltfAsset.skins[0];

            // does it have information about joints?
            if (glTFSkin.inverseBindMatrices.has_value()) // glTFSkin.inverseBindMatrices refers to an gltf accessor
            {
                auto& joints =
                    m_Skeleton
                        ->m_Joints; // just a reference to the joints std::vector of that skeleton (to make code easier)

                // set up number of joints
                size_t numberOfJoints = glTFSkin.joints.size();
                // resize the joints vector of the skeleton object (to be filled)
                joints.resize(numberOfJoints);
                m_Skeleton->m_ShaderData.m_FinalJointsMatrices.resize(numberOfJoints);

                // set up name of skeleton
                m_Skeleton->m_Name = glTFSkin.name;
                LOG_CORE_INFO("name of skeleton: {0}", m_Skeleton->m_Name);

                // retrieve array of inverse bind matrices of all joints
                // --> first, retrieve raw data and copy into a std::vector
                const glm::mat4* inverseBindMatrices;
                {
                    size_t count = 0;
                    fastgltf::AccessorType type;
                    fastgltf::ComponentType componentType = LoadAccessor<glm::mat4>(
                        m_GltfAsset.accessors[glTFSkin.inverseBindMatrices.value()], inverseBindMatrices, &count, &type);
                    CORE_ASSERT(type == fastgltf::AccessorType::Mat4, "unexpected type");
                    CORE_ASSERT(fastgltf::getGLComponentType(componentType) == GL_FLOAT, "unexpected component type");
                    // assert # of matrices matches # of joints
                    CORE_ASSERT(count == numberOfJoints, "accessor.count != numberOfJoints");
                }

                // loop over all joints from gltf model and fill the skeleton with joints
                for (size_t jointIndex = 0; jointIndex < numberOfJoints; ++jointIndex)
                {
                    int globalGltfNodeIndex = glTFSkin.joints[jointIndex];
                    auto& joint = joints[jointIndex]; // just a reference for easier code
                    joint.m_InverseBindMatrix = inverseBindMatrices[jointIndex];
                    joint.m_Name = m_GltfAsset.nodes[globalGltfNodeIndex].name;

                    // set up map "global node" to "joint index"
                    m_Skeleton->m_GlobalNodeToJointIndex[globalGltfNodeIndex] = jointIndex;
                }

                int rootJoint = glTFSkin.joints[0]; // the code here always works
                                                    // but the gltf field skins.skeleton can be ignored

                LoadJoint(rootJoint, Armature::NO_PARENT);
            }
            // m_Skeleton->Traverse();

            // create a buffer to be used in the shader for the joint matrices
            // The gltf model has multiple animations, all applied to the same skeleton
            int numberOfJoints = m_Skeleton->m_Joints.size();
            int bufferSize = numberOfJoints * sizeof(glm::mat4); // in bytes
            m_ShaderData = Buffer::Create(bufferSize);
            m_ShaderData->MapBuffer();
        }

        size_t numberOfAnimations = m_GltfAsset.animations.size();
        for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
        {
            auto& gltfAnimation = m_GltfAsset.animations[animationIndex];
            std::string name(gltfAnimation.name);
            LOG_CORE_INFO("name of animation: {0}", name);
            std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(name);

            // Samplers
            size_t numberOfSamplers = gltfAnimation.samplers.size();
            animation->m_Samplers.resize(numberOfSamplers);
            for (size_t samplerIndex = 0; samplerIndex < numberOfSamplers; ++samplerIndex)
            {
                fastgltf::AnimationSampler glTFSampler = gltfAnimation.samplers[samplerIndex];
                auto& sampler = animation->m_Samplers[samplerIndex];

                sampler.m_Interpolation = static_cast<SkeletalAnimation::InterpolationMethod>(glTFSampler.interpolation);

                // get timestamp
                {
                    size_t count = 0;
                    const float* timestampBuffer;
                    fastgltf::ComponentType componentType =
                        LoadAccessor<float>(m_GltfAsset.accessors[glTFSampler.inputAccessor], timestampBuffer, &count);

                    if (fastgltf::getGLComponentType(componentType) == GL_FLOAT)
                    {
                        sampler.m_Timestamps.resize(count);
                        for (size_t index = 0; index < count; ++index)
                        {
                            sampler.m_Timestamps[index] = timestampBuffer[index];
                        }
                    }
                    else
                    {
                        CORE_ASSERT(false, "FastgltfBuilder::LoadSkeletonsGltf: cannot handle timestamp format");
                    }
                }

                // Read sampler keyframe output translate/rotate/scale values
                {
                    size_t count = 0;
                    fastgltf::AccessorType type;
                    const uint* buffer;
                    LoadAccessor<uint>(m_GltfAsset.accessors[glTFSampler.outputAccessor], buffer, &count, &type);

                    switch (type)
                    {
                        case fastgltf::AccessorType::Vec3:
                        {
                            const glm::vec3* outputBuffer = reinterpret_cast<const glm::vec3*>(buffer);
                            sampler.m_TRSoutputValuesToBeInterpolated.resize(count);
                            for (size_t index = 0; index < count; index++)
                            {
                                sampler.m_TRSoutputValuesToBeInterpolated[index] = glm::vec4(outputBuffer[index], 0.0f);
                            }
                            break;
                        }
                        case fastgltf::AccessorType::Vec4:
                        {
                            const glm::vec4* outputBuffer = reinterpret_cast<const glm::vec4*>(buffer);
                            sampler.m_TRSoutputValuesToBeInterpolated.resize(count);
                            for (size_t index = 0; index < count; index++)
                            {
                                sampler.m_TRSoutputValuesToBeInterpolated[index] = glm::vec4(outputBuffer[index]);
                            }
                            break;
                        }
                        default:
                        {
                            CORE_ASSERT(false, "void FastgltfBuilder::LoadSkeletonsGltf(...): accessor type not found");
                            break;
                        }
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
            // Each node of the skeleton has channels that point to samplers
            size_t numberOfChannels = gltfAnimation.channels.size();
            animation->m_Channels.resize(numberOfChannels);
            for (size_t channelIndex = 0; channelIndex < numberOfChannels; ++channelIndex)
            {
                fastgltf::AnimationChannel glTFChannel = gltfAnimation.channels[channelIndex];
                SkeletalAnimation::Channel& channel = animation->m_Channels[channelIndex];
                channel.m_SamplerIndex = glTFChannel.samplerIndex;
                CORE_ASSERT(glTFChannel.nodeIndex.has_value(), "FastgltfBuilder::LoadSkeletonsGltf() no target node");
                channel.m_Node = glTFChannel.nodeIndex.value();
                switch (glTFChannel.path)
                {
                    case fastgltf::AnimationPath::Scale:
                    {
                        channel.m_Path = SkeletalAnimation::Path::SCALE;
                        break;
                    }
                    case fastgltf::AnimationPath::Rotation:
                    {
                        channel.m_Path = SkeletalAnimation::Path::ROTATION;
                        break;
                    }
                    case fastgltf::AnimationPath::Translation:
                    {
                        channel.m_Path = SkeletalAnimation::Path::TRANSLATION;
                        break;
                    }
                    default:
                    {
                        LOG_CORE_CRITICAL("path not supported");
                        break;
                    }
                }
            }
            m_Animations->Push(animation);
        }

        m_SkeletalAnimation = (m_Animations->Size()) ? true : false;
    }

    // recursive function via global gltf nodes (which have children)
    // tree structure links (local) skeleton joints
    void FastgltfBuilder::LoadJoint(int globalGltfNodeIndex, int parentJoint)
    {
        int currentJoint = m_Skeleton->m_GlobalNodeToJointIndex[globalGltfNodeIndex];
        auto& joint = m_Skeleton->m_Joints[currentJoint]; // a reference to the current joint

        joint.m_ParentJoint = parentJoint;

        // process children (if any)
        size_t numberOfChildren = m_GltfAsset.nodes[globalGltfNodeIndex].children.size();
        if (numberOfChildren > 0)
        {
            joint.m_Children.resize(numberOfChildren);
            for (size_t childIndex = 0; childIndex < numberOfChildren; ++childIndex)
            {
                uint globalGltfNodeIndexForChild = m_GltfAsset.nodes[globalGltfNodeIndex].children[childIndex];
                joint.m_Children[childIndex] = m_Skeleton->m_GlobalNodeToJointIndex[globalGltfNodeIndexForChild];
                LoadJoint(globalGltfNodeIndexForChild, currentJoint);
            }
        }
    }
} // namespace GfxRenderEngine
