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

#include "physics/physicsBase.h"

#include <Jolt/Jolt.h>

// Jolt includes
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

namespace GfxRenderEngine
{
    PhysicsBase::PhysicsBase(Scene& scene)
        : m_Scene{scene}, m_Registry{scene.GetRegistry()}, m_Dictionary{scene.GetDictionary()}
    {

        // Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you
        // want (see Memory.h). This needs to be done before any other Jolt function is called.
        JPH::RegisterDefaultAllocator();
        // Install trace and assert callbacks
        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

        // Create a factory, this class is responsible for creating instances of classes based on their name or hash and is
        // mainly used for deserialization of saved data. It is not directly used in this example but still required.
        JPH::Factory::sInstance = new JPH::Factory();

        // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
        // If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch
        // before calling this function. If you implement your own default material (PhysicsMaterial::sDefault) make sure to
        // initialize it before this function or else this function will create one for you.
        JPH::RegisterTypes();

        m_pTempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        m_pJobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                                  static_cast<int>(std::thread::hardware_concurrency()) - 1);

        m_PhysicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface,
                             object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

        m_GameObject = m_Dictionary.Retrieve("SL::application/lucre/models/mario/sphere.glb::0::root");
    }

    void PhysicsBase::CreateGroundPlane()
    {
        // The main way to interact with the bodies in the physics system is through the body interface. There is a locking
        // and a non-locking variant of this. We're going to use the locking version (even though we're not planning to
        // access bodies from multiple threads)
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        // Define the size of the ground
        JPH::Vec3 groundSize(50.0f, 0.4f, 50.0f); // 50x50 plane, with a small thickness

        // Create a box shape for the ground
        JPH::Ref<JPH::BoxShape> groundShape = new JPH::BoxShape(groundSize);

        // Define body creation settings
        JPH::BodyCreationSettings settings(groundShape,              // Shape
                                           JPH::Vec3(0, 0.8f, 0),    // Position (y = 0.8 to sit at y = 1.2)
                                           JPH::Quat::sIdentity(),   // No rotation
                                           JPH::EMotionType::Static, // Static body
                                           Layers::NON_MOVING        // Collision layer
        );

        // Create the body
        m_GroundID = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
    }

    void PhysicsBase::CreateSphere()
    {
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        // Now create a dynamic body to bounce on the floor
        // Note that this uses the shorthand version of creating and adding a body to the world
        JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f), JPH::RVec3(0.0_r, 8.0_r, 0.0_r),
                                                  JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
        m_SphereID = bodyInterface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
        // Now you can interact with the dynamic body, in this case we're going to give it a velocity.
        // (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to
        // the physics system)
        bodyInterface.SetLinearVelocity(m_SphereID, JPH::Vec3(0.0f, -5.0f, 0.0f));
        bodyInterface.SetRestitution(m_SphereID, 0.8f);
    }

    void PhysicsBase::OnUpdate(Timestep timestep)
    {
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the
        // simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
        const int cCollisionSteps = 1;

        // Step the world
        m_PhysicsSystem.Update(timestep, cCollisionSteps, m_pTempAllocator.get(), m_pJobSystem.get());

        { // Output current position and velocity of the sphere
            JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(m_SphereID);
            [[maybe_unused]] JPH::Vec3 velocity = bodyInterface.GetLinearVelocity(m_SphereID);

            if (m_GameObject != entt::null)
            {
                auto& transform = m_Registry.get<TransformComponent>(m_GameObject);
                transform.SetTranslation(glm::vec3{position.GetX(), position.GetY(), position.GetZ()});
            }
        }
    }
} // namespace GfxRenderEngine
