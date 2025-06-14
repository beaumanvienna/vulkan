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

#pragma once

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyManager.h>

// vehicle
#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>

// debug rendering
#include <TestFramework.h>
#include <Renderer/Renderer.h>
#include <Renderer/DebugRendererImp.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include "engine/JoltDebugRenderer/Renderer/VK/RendererVK.h"
#include <Renderer/Font.h>

#include "engine.h"
#include "scene/scene.h"
#include "physics/physics.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    // Layer that objects can be in, determines which other objects it can collide with
    // Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
    // layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics
    // simulation but only if you do collision testing).
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }; // namespace Layers

    // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
    // a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
    // You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
    // many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
    // your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    }; // namespace BroadPhaseLayers

    using namespace JPH::literals;

    class PhysicsBase : public Physics
    {
    public:
        PhysicsBase() = delete;
        PhysicsBase(Scene& scene);
        virtual void OnUpdate(Timestep timestep, VehicleControl const& vehicleControl) override;
        virtual void CreateGroundPlane(GroundSpec const& groundSpec) override;
        virtual void LoadModels(CarParameters const& carParameters) override;
        virtual void SetGameObject(uint gameObject, entt::entity gameObjectID) override;
        virtual void SetWheelTranslation(uint wheelNumber, glm::mat4 const& translation) override;
        virtual void SetWheelScale(uint wheelNumber, glm::mat4 const& translation) override;
        virtual void Draw(GfxRenderEngine::Camera const& cam0) override;
        virtual void CreateMeshTerrain(entt::entity, const std::string& filepath, float friction) override;
        virtual void SetCarHeightOffset(float carHeightOffset) override;

    private:
        void CreateSphere(glm::vec3 const& scale, glm::vec3 const& translation);
        void CreateMushroom(glm::vec3 const& scale, glm::vec3 const& translation);
        void CreateVehicle(RVec3 const& position, JPH::Quat const& quaternion);
        void SyncPhysicsToGraphics();

    private:
        /// Class that determines if two object layers can collide
        class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
        {
        public:
            virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
            {
                switch (inObject1)
                {
                    case Layers::NON_MOVING:
                        return inObject2 == Layers::MOVING; // Non moving only collides with moving
                    case Layers::MOVING:
                        return true; // Moving collides with everything
                    default:
                        JPH_ASSERT(false);
                        return false;
                }
            }
        };

        /// Class that determines if an object layer can collide with a broadphase layer
        class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
        {
        public:
            virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
            {
                switch (inLayer1)
                {
                    case Layers::NON_MOVING:
                        return inLayer2 == BroadPhaseLayers::MOVING;
                    case Layers::MOVING:
                        return true;
                    default:
                        JPH_ASSERT(false);
                        return false;
                }
            }
        };

        // BroadPhaseLayerInterface implementation
        // This defines a mapping between object and broadphase layers.
        class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
        {
        public:
            BPLayerInterfaceImpl()
            {
                // Create a mapping table from object to broad phase layer
                mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
                mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            }

            virtual uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

            virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
            {
                JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
                return mObjectToBroadPhase[inLayer];
            }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
            {
                switch ((JPH::BroadPhaseLayer::Type)inLayer)
                {
                    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                        return "NON_MOVING";
                    case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                        return "MOVING";
                    default:
                        JPH_ASSERT(false);
                        return "INVALID";
                }
            }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

        private:
            JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
        };

        // Callback for traces, connect this to your own trace function if you have one
        static void TraceImpl(const char* inFMT, ...)
        {
            // Format the message
            va_list list;
            va_start(list, inFMT);
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), inFMT, list);
            va_end(list);

            // Print to the TTY
            std::cout << buffer << std::endl;
        }

#ifdef JPH_ENABLE_ASSERTS

        // Callback for asserts, connect this to your own assert handler if you have one
        static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
        {
            // Print to the TTY
            std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "")
                      << std::endl;

            // Breakpoint
            return true;
        };

#endif // JPH_ENABLE_ASSERTS

    private:
        JPH::PhysicsSystem m_PhysicsSystem;
        // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get
        // an error. Note: This value is low because this is a simple test. For a real project use something in the order of
        // 65536.
        const uint cMaxBodies = 1024;

        // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the
        // default settings.
        const uint cNumBodyMutexes = 0;

        // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this
        // buffer too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is
        // slightly less efficient. Note: This value is low because this is a simple test. For a real project use something
        // in the order of 65536.
        const uint cMaxBodyPairs = 1024;

        // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are
        // detected than this number then these contacts will be ignored and bodies will start interpenetrating / fall
        // through the world. Note: This value is low because this is a simple test. For a real project use something in the
        // order of 10240.
        const uint cMaxContactConstraints = 1024;

        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at BroadPhaseLayerInterfaceTable or BroadPhaseLayerInterfaceMask for a simpler interface.
        BPLayerInterfaceImpl broad_phase_layer_interface;

        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectVsBroadPhaseLayerFilterTable or ObjectVsBroadPhaseLayerFilterMask for a simpler
        // interface.
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        // Also have a look at ObjectLayerPairFilterTable or ObjectLayerPairFilterMask for a simpler interface.
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;

        // We need a temp allocator for temporary allocations during the physics update. We're
        // pre-allocating 10 MB to avoid having to do allocations during the physics update.
        // B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
        // If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
        // malloc / free.
        std::unique_ptr<JPH::TempAllocatorImpl> m_pTempAllocator;

        // We need a job system that will execute physics jobs on multiple threads. Typically
        // you would implement the JobSystem interface yourself and let Jolt Physics run on top
        // of your own job scheduler. JobSystemThreadPool is an example implementation.
        std::unique_ptr<JPH::JobSystemThreadPool> m_pJobSystem;

        inline glm::mat4& ConvertToGLMMat4(JPH::RMat44& jphMat) { return *reinterpret_cast<glm::mat4*>(&jphMat); }
        inline JPH::Vec3 const& ConvertToVec3(glm::vec3 const& vec3GLM)
        {
            return *reinterpret_cast<JPH::Vec3 const*>(&vec3GLM);
        }
        inline JPH::Quat const ConvertToQuat(glm::vec3 const& vec3EulerAnglesGLM)
        {
            glm::quat quaternion = glm::quat(vec3EulerAnglesGLM);
            glm::quat* ptr = &quaternion;
            JPH::Quat* ptrQuatJPH = reinterpret_cast<JPH::Quat*>(ptr);
            return *ptrQuatJPH;
        }

    private:
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<Font> m_Font;

        JPH::BodyManager::DrawSettings m_DrawSettings;
        std::unique_ptr<JPH::DebugRenderer> m_DebugRenderer;

    private:
        Scene& m_Scene;
        Registry& m_Registry;
        Dictionary& m_Dictionary;
        std::string m_DictionaryPrefix;
        static constexpr bool NO_SCENE_GRAPH = false;
        JPH::BodyID m_GroundID; // set invalid by default constructor
        JPH::BodyID m_SphereID;
        JPH::BodyID m_MushroomID;
        std::vector<JPH::BodyID> m_ActiveBodies;

        std::array<entt::entity, GameObjects::NUM_GAME_OBJECTS> m_GameObjects;
        std::array<glm::mat4, Physics::NUM_WHEELS> m_WheelTranslation;
        std::array<glm::mat4, Physics::NUM_WHEELS> m_WheelScale;

        // vehicle
        Body* mCarBody;                            ///< The vehicle
        Ref<VehicleConstraint> mVehicleConstraint; ///< The vehicle constraint
        Ref<VehicleCollisionTester> mTesters[3];   ///< Collision testers for the wheel

        static inline float sInitialRollAngle = 0;
        static inline float sMaxRollAngle = 1.0472;       // 60 degree
        static inline float sMaxSteeringAngle = 0.523599; // 30 degree
        static inline int sCollisionMode = 2;
        static inline bool sFourWheelDrive = false;
        static inline bool sAntiRollbar = true;
        static inline bool sLimitedSlipDifferentials = true;
        static inline bool sOverrideGravity = false; ///< If true, gravity is overridden to always oppose the ground normal
        static inline float sMaxEngineTorque = 500.0f;
        static inline float sClutchStrength = 10.0f;
        static inline float sFrontCasterAngle = 0.0f;
        static inline float sFrontKingPinAngle = 0.0f;
        static inline float sFrontCamber = 0.0f;
        static inline float sFrontToe = 0.0f;
        static inline float sFrontSuspensionForwardAngle = 0.0f;
        static inline float sFrontSuspensionSidewaysAngle = 0.0f;
        static inline float sFrontSuspensionMinLength = 0.3f;
        static inline float sFrontSuspensionMaxLength = 0.5f;
        static inline float sFrontSuspensionFrequency = 1.5f;
        static inline float sFrontSuspensionDamping = 0.5f;
        static inline float sRearSuspensionForwardAngle = 0.0f;
        static inline float sRearSuspensionSidewaysAngle = 0.0f;
        static inline float sRearCasterAngle = 0.0f;
        static inline float sRearKingPinAngle = 0.0f;
        static inline float sRearCamber = 0.0f;
        static inline float sRearToe = 0.0f;
        static inline float sRearSuspensionMinLength = 0.3f;
        static inline float sRearSuspensionMaxLength = 0.5f;
        static inline float sRearSuspensionFrequency = 1.5f;
        static inline float sRearSuspensionDamping = 1.0f;
        float m_CarHeightOffset;
    };
} // namespace GfxRenderEngine
