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

#include <Renderer/DebugRendererImp.h>
#include "engine/JoltDebugRenderer/Renderer/VK/RendererVK.h"

#include "scene/components.h"
#include "physics/physicsBase.h"
#include "renderer/instanceBuffer.h"

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

        // Create font
        m_Font = std::make_unique<Font>(m_Renderer.get());
        // m_Font->Create("Roboto-Regular", 24);

        // debug renderer
        m_DebugRenderer = std::make_unique<DebugRendererImp>(m_Renderer.get(), m_Font.get());

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
    }

    void PhysicsBase::OnUpdate(Timestep timestep)
    {

        // If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the
        // simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
        const int cCollisionSteps = 1;

        // Step the world
        float speedFactor = 1.0f;
        m_PhysicsSystem.Update(timestep * speedFactor, cCollisionSteps, m_pTempAllocator.get(), m_pJobSystem.get());

        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();

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
    }

    void PhysicsBase::Draw(GfxRenderEngine::Camera const& cam0)
    {
        if (!m_DebugRenderer)
        {
            return;
        }

        JPH::CameraState camera(cam0);
        m_Renderer->BeginFrame(camera, 1.0f /*world scale*/);
        static_cast<DebugRendererImp*>(m_DebugRenderer.get())->Clear();
        m_PhysicsSystem.DrawBodies(m_DrawSettings,        // const BodyManager::DrawSettings &inSettings
                                   m_DebugRenderer.get(), // DebugRenderer* inRenderer
                                   nullptr                // const BodyDrawFilter* inBodyFilter = nullptr
        );
        static_cast<DebugRendererImp*>(m_DebugRenderer.get())->Draw();
        m_Renderer->EndFrame();
    }

} // namespace GfxRenderEngine
