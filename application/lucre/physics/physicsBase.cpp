// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// file vendor/jolt/src/HelloWorld.cpp from Jolt merged into Lucre

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

#include "scene/components.h"
#include "physics/physicsBase.h"
#include "renderer/instanceBuffer.h"
#include "renderer/model.h"
#include "renderer/builder/fastgltfVertexLoader.h"

namespace GfxRenderEngine
{
    PhysicsBase::PhysicsBase(Scene& scene)
        : m_Scene{scene}, m_Registry{scene.GetRegistry()}, m_Dictionary{scene.GetDictionary()}, m_DictionaryPrefix{"PHSX"}
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
        // Create renderer
        m_Renderer = std::make_unique<RendererVK>();
        m_Renderer->Initialize();

        // debug renderer
        m_DebugRenderer = std::make_unique<DebugRendererImp>(m_Renderer.get(), nullptr /*m_Font.get()*/);

        m_DrawSettings.mDrawShape = true;
        m_DrawSettings.mDrawBoundingBox = true;
        m_DrawSettings.mDrawShapeWireframe = true;
    }

    void PhysicsBase::LoadModels()
    {
        {
            glm::vec3 scale{1.0f, 1.0f, 1.0f};
            glm::vec3 translation{0.2f, 5.5f, 5.0f};
            CreateSphere(scale, translation);
        }
        {
            glm::vec3 scale{1.0f, 1.0f, 1.0f};
            glm::vec3 translation{0.0f, 6.0f, 18.0f};
            CreateMushroom(scale, translation);
        }
        {
            RVec3 position(2.00273f, 15.0f, 30.1575f);
            glm::vec3 rotation(0.0f, 1.41213f, 0.0f);
            JPH::Quat const quaternion = ConvertToQuat(rotation);
            CreateVehicle(position, quaternion);
        }
    }

    void PhysicsBase::OnUpdate(Timestep timestep, VehicleControl const& vehicleControl)
    {

        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the
        // simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
        const int cCollisionSteps = 1;
        // On user input, assure that the car is active
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();
        auto carID = mCarBody->GetID();
        if (vehicleControl.inRight != 0.0f || vehicleControl.inForward != 0.0f)
        {
            bodyInterface.ActivateBody(carID);
        }
        { // update vehicle
            auto vehicleController = static_cast<WheeledVehicleController*>(mVehicleConstraint->GetController());
            vehicleController->SetDriverInput(vehicleControl.inForward, vehicleControl.inRight, vehicleControl.inBrake,
                                              vehicleControl.inHandBrake);
        }

        // Step the world
        float speedFactor = 1.0f;
        m_PhysicsSystem.Update(timestep * speedFactor, cCollisionSteps, m_pTempAllocator.get(), m_pJobSystem.get());

        if (auto& gameObject = m_GameObjects[GameObjects::GAME_OBJECT_MUSHROOM]; gameObject != entt::null)
        {
            JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(m_MushroomID);
            auto& transform = m_Registry.get<TransformComponent>(gameObject);
            transform.SetTranslation(glm::vec3{position.GetX(), position.GetY(), position.GetZ()});
        }
        if (auto& gameObject = m_GameObjects[GameObjects::GAME_OBJECT_SPHERE]; gameObject != entt::null)
        {
            JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(m_SphereID);
            auto& transform = m_Registry.get<TransformComponent>(gameObject);
            transform.SetTranslation(glm::vec3{position.GetX(), position.GetY(), position.GetZ()});
        }

        // car body
        if (auto& gameObject = m_GameObjects[GameObjects::GAME_OBJECT_CAR]; gameObject != entt::null)
        {
            JPH::RVec3 position = bodyInterface.GetCenterOfMassPosition(carID);
            JPH::Quat rotation = bodyInterface.GetRotation(carID);
            auto& transform = m_Registry.get<TransformComponent>(gameObject);
            transform.SetTranslation(glm::vec3{position.GetX(), position.GetY(), position.GetZ()});
            transform.SetRotation(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ()));
        }

        // wheels
        if ((m_GameObjects[GameObjects::GAME_OBJECT_WHEEL_FRONT_LEFT] != entt::null) &&
            (m_GameObjects[GameObjects::GAME_OBJECT_WHEEL_FRONT_RIGHT] != entt::null) &&
            (m_GameObjects[GameObjects::GAME_OBJECT_WHEEL_REAR_LEFT] != entt::null) &&
            (m_GameObjects[GameObjects::GAME_OBJECT_WHEEL_REAR_RIGHT] != entt::null))
        {
            for (uint w = 0; w < Physics::NUM_WHEELS; ++w)
            {
                JPH::RMat44 wheelTransformJPH = mVehicleConstraint->GetWheelLocalTransform(w,               // wheel index
                                                                                           Vec3::sAxisX(),  // inWheelRight
                                                                                           Vec3::sAxisY()); // inWheelUp
                glm::mat4 wheelLocalTransformGLM =
                    m_WheelTranslation[w] * ConvertToGLMMat4(wheelTransformJPH) * m_WheelScale[w];

                JPH::RMat44 carTransformJPH = bodyInterface.GetWorldTransform(carID);
                glm::mat4 carTransformGLM = ConvertToGLMMat4(carTransformJPH);
                glm::mat4 wheelGlobalTransformGLM = carTransformGLM * wheelLocalTransformGLM;

                entt::entity wheelGameObject = m_GameObjects[GAME_OBJECT_WHEEL_FRONT_LEFT + w];
                auto& transform = m_Registry.get<TransformComponent>(wheelGameObject);
                transform.SetMat4Local(wheelGlobalTransformGLM);
            }
        }
    }

    void PhysicsBase::Draw(GfxRenderEngine::Camera const& cam0)
    {
        if (!m_DebugRenderer)
        {
            return;
        }

        JPH::CameraState camera(cam0);
        m_Renderer->BeginFrame(camera, 1.0f /*world scale*/, cam0);
        static_cast<DebugRendererImp*>(m_DebugRenderer.get())->Clear();
        m_PhysicsSystem.DrawBodies(m_DrawSettings,        // const BodyManager::DrawSettings &inSettings
                                   m_DebugRenderer.get(), // DebugRenderer* inRenderer
                                   nullptr                // const BodyDrawFilter* inBodyFilter = nullptr
        );
        static_cast<DebugRendererImp*>(m_DebugRenderer.get())->Draw();
        m_Renderer->EndFrame();
    }

    void PhysicsBase::SyncPhysicsToGraphics()
    {
        auto& lockInterface = m_PhysicsSystem.GetBodyLockInterface();
        // iterate through all active physics bodies
        for (const JPH::BodyID& bodyID : m_ActiveBodies)
        {
            // scoped lock for the body for multi-threadding
            JPH::BodyLockRead lock(lockInterface, bodyID);
            if (lock.Succeeded())
            {
                const JPH::Body& body = lock.GetBody();

                // Get physics transform
                JPH::Vec3 position = body.GetPosition();
                JPH::Quat rotation = body.GetRotation();

                // Update corresponding graphics object
                auto entityID = static_cast<entt::entity>(body.GetUserData());
                if (m_Registry.valid(entityID) && m_Registry.all_of<TransformComponent>(entityID))
                {
                    glm::vec3 eulerAngles{rotation.GetEulerAngles().GetX(), rotation.GetEulerAngles().GetY(),
                                          rotation.GetEulerAngles().GetZ()};
                    const glm::quat rotationGraphics = glm::quat(eulerAngles);
                    TransformComponent& transform = m_Registry.get<TransformComponent>(entityID);
                    transform.SetRotation(rotationGraphics);
                    transform.SetTranslation({position.GetX(), position.GetY(), position.GetZ()});
                }
            }
        }
    }

    void PhysicsBase::SetGameObject(uint gameObject, entt::entity gameObjectID) { m_GameObjects[gameObject] = gameObjectID; }

    void PhysicsBase::SetWheelTranslation(uint wheelNumber, glm::mat4 const& translation)
    {
        m_WheelTranslation[wheelNumber] = translation;
    }

    void PhysicsBase::SetWheelScale(uint wheelNumber, glm::mat4 const& scale) { m_WheelScale[wheelNumber] = scale; }

    void PhysicsBase::CreateMeshTerrain(entt::entity entityID, const std::string& filepath)
    {
        if (m_Registry.valid(entityID) && m_Registry.all_of<TransformComponent>(entityID))
        {
            TransformComponent& transformComponent = m_Registry.get<TransformComponent>(entityID);

            TriangleList triangles;
            FastgltfVertexLoader fastgltfVertexLoader(filepath, triangles);
            if (fastgltfVertexLoader.Load())
            {
                // Floor
                JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();
                Body& floor = *bodyInterface.CreateBody(BodyCreationSettings(new MeshShapeSettings(triangles),
                                                                             RVec3::sZero(), Quat::sIdentity(),
                                                                             EMotionType::Static, Layers::NON_MOVING));
                bodyInterface.AddBody(floor.GetID(), EActivation::DontActivate);
                JPH::Vec3 const& position = ConvertToVec3(transformComponent.GetTranslation());
                bodyInterface.SetPosition(floor.GetID(), position, EActivation::DontActivate);
                JPH::Quat const quaternion = ConvertToQuat(transformComponent.GetRotation());
                bodyInterface.SetRotation(floor.GetID(), quaternion, EActivation::DontActivate);
            }
        }
    }

} // namespace GfxRenderEngine
