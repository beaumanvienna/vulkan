/* Engine Copyright (c) 2025 Engine Development Team
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

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include "scene/components.h"
#include "physics/physicsBase.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/fastgltfBuilder.h"

namespace GfxRenderEngine
{
    void PhysicsBase::CreateGroundPlane(glm::vec3 const& scale, glm::vec3 const& translation)
    {
        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking
        // and a non-locking variant of this. We're going to use the locking version (even though we're not planning to
        // access bodies from multiple threads)
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        // Define the size of the ground
        JPH::Vec3 groundSize(scale.x, scale.y, scale.z);

        // Create a box shape for the ground
        JPH::Ref<JPH::BoxShape> groundShape = new JPH::BoxShape(groundSize / 2.0f);

        // Define body creation settings
        JPH::BodyCreationSettings settings(groundShape,                                            // Shape
                                           JPH::Vec3(translation.x, translation.y, translation.z), // Position
                                           JPH::Quat::sIdentity(),                                 // No rotation
                                           JPH::EMotionType::Static,                               // Static body
                                           Layers::NON_MOVING                                      // Collision layer
        );

        // Create the body
        m_GroundID = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);

        {
            std::string filepath("application/lucre/models/mario/debug box.glb");
            auto& sceneGraph = m_Scene.GetSceneGraph();
            auto& entity = m_GameObjects[GAME_OBJECT_GROUND_PLANE];

            // create group node
            entity = m_Registry.Create();
            auto name = m_DictionaryPrefix + "::" + filepath + "::root";
            int groupNode = sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
            TransformComponent transform{};
            transform.SetScale(scale);
            transform.SetTranslation(translation);
            m_Registry.emplace<TransformComponent>(entity, transform);

            FastgltfBuilder builder(filepath, m_Scene, groupNode);
            builder.SetDictionaryPrefix(m_DictionaryPrefix);
            uint numberOfInstances = 1;
            std::vector<entt::entity> firstInstances;
            builder.Load(numberOfInstances, firstInstances);
        }
    }

    void PhysicsBase::CreateMushroom(glm::vec3 const& scale, glm::vec3 const& translation)
    {
        auto& entity = m_GameObjects[GameObjects::GAME_OBJECT_MUSHROOM];

        std::string filepath("application/lucre/models/mario/mushroom.glb");
        auto& sceneGraph = m_Scene.GetSceneGraph();

        // create group node
        entity = m_Registry.Create();
        auto name = m_DictionaryPrefix + "::" + filepath + "::root";
        int groupNode = sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
        TransformComponent transform{};
        transform.SetTranslation(translation);
        transform.SetScale(scale);
        m_Registry.emplace<TransformComponent>(entity, transform);
        {
            FastgltfBuilder builder(filepath, m_Scene, groupNode);
            builder.SetDictionaryPrefix(m_DictionaryPrefix);
            uint numberOfInstances = 1;
            std::vector<entt::entity> firstInstances;
            builder.Load(numberOfInstances, firstInstances);
        }

        if (entity == entt::null)
        {
            return;
        }

        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        float translationX = transform.GetTranslation().x;
        float translationY = transform.GetTranslation().y;
        float translationZ = transform.GetTranslation().z;

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                                  JPH::RVec3(translationX, translationY, translationZ),
                                                  JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
        m_MushroomID = bodyInterface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
        // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
        // (note that if we had used CreateBody then we could have set the velocity straight on the body before
        // adding it to the physics system)
        bodyInterface.SetLinearVelocity(m_MushroomID, 2.0f * JPH::Vec3(0.0f, 0.0f, -2.5f));
        bodyInterface.SetRestitution(m_MushroomID, 0.8f);
    }

    void PhysicsBase::CreateSphere(glm::vec3 const& scale, glm::vec3 const& translation)
    {
        auto& entity = m_GameObjects[GameObjects::GAME_OBJECT_SPHERE];

        std::string filepath("application/lucre/models/mario/sphere.glb");
        auto& sceneGraph = m_Scene.GetSceneGraph();

        // create group node
        entity = m_Registry.Create();
        auto name = m_DictionaryPrefix + "::" + filepath + "::root";
        int groupNode = sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
        TransformComponent transform{};
        transform.SetTranslation(translation);
        transform.SetScale(scale);
        m_Registry.emplace<TransformComponent>(entity, transform);
        {
            FastgltfBuilder builder(filepath, m_Scene, groupNode);
            builder.SetDictionaryPrefix(m_DictionaryPrefix);
            uint numberOfInstances = 1;
            std::vector<entt::entity> firstInstances;
            builder.Load(numberOfInstances, firstInstances);
        }

        if (entity == entt::null)
        {
            return;
        }

        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        float translationX = transform.GetTranslation().x;
        float translationY = transform.GetTranslation().y;
        float translationZ = transform.GetTranslation().z;

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                                  JPH::RVec3(translationX, translationY, translationZ),
                                                  JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
        m_SphereID = bodyInterface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
        // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
        // (note that if we had used CreateBody then we could have set the velocity straight on the body before
        // adding it to the physics system)
        bodyInterface.SetRestitution(m_SphereID, 0.8f);
    }

} // namespace GfxRenderEngine
