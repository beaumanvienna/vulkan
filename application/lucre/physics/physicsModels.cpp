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

#include "auxiliary/file.h"
#include "scene/components.h"
#include "physics/physicsBase.h"
#include "renderer/instanceBuffer.h"
#include "renderer/builder/fastgltfBuilder.h"

namespace GfxRenderEngine
{
    void PhysicsBase::CreateGroundPlane(GroundSpec const& groundSpec)
    {
        glm::vec3 const& scale = groundSpec.m_Scale;
        glm::vec3 const& translation = groundSpec.m_Position;
        std::string const& filepath = groundSpec.m_Filepath;
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
        auto groundID = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
        bodyInterface.SetFriction(groundID, groundSpec.m_Friction);
        if (EngineCore::FileExists(filepath))
        {
            auto& sceneGraph = m_Scene.GetSceneGraph();
            auto& entity = m_GameObjects[GAME_OBJECT_GROUND_PLANE];

            // create group node
            entity = m_Registry.Create();
            auto name = m_DictionaryPrefix + "::" + filepath + "::root";
            int groupNode = sceneGraph.CreateNode(SceneGraph::ROOT_NODE, entity, name, m_Dictionary);
            TransformComponent transform{};
            transform.SetScale(scale);
            transform.SetTranslation(glm::vec3{translation.x, translation.y - (scale.y / 2.0f), translation.z});
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

    void PhysicsBase::CreateCar(RVec3 const& position, JPH::Quat const& quaternion)
    {
        [[maybe_unused]] const float sInitialRollAngle = 0;
        [[maybe_unused]] const float sMaxRollAngle = 1.0472;       // 60 degree
        [[maybe_unused]] const float sMaxSteeringAngle = 0.523599; // 30 degree
        [[maybe_unused]] const int sCollisionMode = 2;
        [[maybe_unused]] const bool sFourWheelDrive = false;
        [[maybe_unused]] const bool sAntiRollbar = true;
        [[maybe_unused]] const bool sLimitedSlipDifferentials = true;
        [[maybe_unused]] const bool sOverrideGravity =
            false; ///< If true, gravity is overridden to always oppose the ground normal
        [[maybe_unused]] const float sMaxEngineTorque = 500.0f;
        [[maybe_unused]] const float sClutchStrength = 10.0f;
        [[maybe_unused]] const float sFrontCasterAngle = 0.0f;
        [[maybe_unused]] const float sFrontKingPinAngle = 0.0f;
        [[maybe_unused]] const float sFrontCamber = 0.0f;
        [[maybe_unused]] const float sFrontToe = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionForwardAngle = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionSidewaysAngle = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionMinLength = 0.3f;
        [[maybe_unused]] const float sFrontSuspensionMaxLength = 0.5f;
        [[maybe_unused]] const float sFrontSuspensionFrequency = 1.5f;
        [[maybe_unused]] const float sFrontSuspensionDamping = 0.5f;
        [[maybe_unused]] const float sRearSuspensionForwardAngle = 0.0f;
        [[maybe_unused]] const float sRearSuspensionSidewaysAngle = 0.0f;
        [[maybe_unused]] const float sRearCasterAngle = 0.0f;
        [[maybe_unused]] const float sRearKingPinAngle = 0.0f;
        [[maybe_unused]] const float sRearCamber = 0.0f;
        [[maybe_unused]] const float sRearToe = 0.0f;
        [[maybe_unused]] const float sRearSuspensionMinLength = 0.3f;
        [[maybe_unused]] const float sRearSuspensionMaxLength = 0.5f;
        [[maybe_unused]] const float sRearSuspensionFrequency = 1.5f;
        [[maybe_unused]] const float sRearSuspensionDamping = 1.0f;

        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();
        const float wheel_radius = 0.3f;
        const float wheel_width = 0.1f;
        const float half_vehicle_length = 2.0f;
        const float half_vehicle_width = 0.9f;
        const float half_vehicle_height = 0.2f;

        // Create collision testers
        mCarTesters[0] = new VehicleCollisionTesterRay(Layers::MOVING);
        mCarTesters[1] = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * wheel_width);
        mCarTesters[2] = new VehicleCollisionTesterCastCylinder(Layers::MOVING);

        // Create vehicle body

        RefConst<Shape> car_shape =
            OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height * 2.0f, 0),
                                            new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length)))
                .Create()
                .Get();
        BodyCreationSettings car_body_settings(car_shape, position, quaternion, EMotionType::Dynamic, Layers::MOVING);
        car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
        car_body_settings.mMassPropertiesOverride.mMass = 1500.0f * 2.0f;
        mCarBody = bodyInterface.CreateBody(car_body_settings);
        bodyInterface.AddBody(mCarBody->GetID(), EActivation::Activate);

        // Create vehicle constraint
        VehicleConstraintSettings vehicle;
        vehicle.mDrawConstraintSize = 0.1f;
        vehicle.mMaxPitchRollAngle = sMaxRollAngle;

        // Suspension direction
        Vec3 front_suspension_dir =
            Vec3(Tan(sFrontSuspensionSidewaysAngle), -1, Tan(sFrontSuspensionForwardAngle)).Normalized();
        Vec3 front_steering_axis = Vec3(-Tan(sFrontKingPinAngle), 1, -Tan(sFrontCasterAngle)).Normalized();
        Vec3 front_wheel_up = Vec3(Sin(sFrontCamber), Cos(sFrontCamber), 0);
        Vec3 front_wheel_forward = Vec3(-Sin(sFrontToe), 0, Cos(sFrontToe));
        Vec3 rear_suspension_dir =
            Vec3(Tan(sRearSuspensionSidewaysAngle), -1, Tan(sRearSuspensionForwardAngle)).Normalized();
        Vec3 rear_steering_axis = Vec3(-Tan(sRearKingPinAngle), 1, -Tan(sRearCasterAngle)).Normalized();
        Vec3 rear_wheel_up = Vec3(Sin(sRearCamber), Cos(sRearCamber), 0);
        Vec3 rear_wheel_forward = Vec3(-Sin(sRearToe), 0, Cos(sRearToe));
        Vec3 flip_x(-1, 1, 1);

        const float scaleFriction = 1.0f;
        JPH::LinearCurve longitudinalFriction;
        longitudinalFriction.Reserve(3);
        longitudinalFriction.AddPoint(0.0f, 0.0f * scaleFriction);
        longitudinalFriction.AddPoint(0.06f, 1.2f * scaleFriction);
        longitudinalFriction.AddPoint(0.2f, 1.0f * scaleFriction);

        JPH::LinearCurve lateralFriction;
        lateralFriction.Reserve(3);
        lateralFriction.AddPoint(0.0f, 0.0f * scaleFriction);
        lateralFriction.AddPoint(3.0f, 1.2f * scaleFriction);
        lateralFriction.AddPoint(20.0f, 1.0f * scaleFriction);

        // Wheels, left front
        WheelSettingsWV* w1 = new WheelSettingsWV;
        w1->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
        w1->mSuspensionDirection = front_suspension_dir;
        w1->mSteeringAxis = front_steering_axis;
        w1->mWheelUp = front_wheel_up;
        w1->mWheelForward = front_wheel_forward;
        w1->mSuspensionMinLength = sFrontSuspensionMinLength;
        w1->mSuspensionMaxLength = sFrontSuspensionMaxLength;
        w1->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
        w1->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
        w1->mMaxSteerAngle = sMaxSteeringAngle;
        w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake
        w1->mLateralFriction = lateralFriction;
        w1->mLongitudinalFriction = longitudinalFriction;

        // Right front
        WheelSettingsWV* w2 = new WheelSettingsWV;
        w2->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
        w2->mSuspensionDirection = flip_x * front_suspension_dir;
        w2->mSteeringAxis = flip_x * front_steering_axis;
        w2->mWheelUp = flip_x * front_wheel_up;
        w2->mWheelForward = flip_x * front_wheel_forward;
        w2->mSuspensionMinLength = sFrontSuspensionMinLength;
        w2->mSuspensionMaxLength = sFrontSuspensionMaxLength;
        w2->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
        w2->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
        w2->mMaxSteerAngle = sMaxSteeringAngle;
        w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake
        w2->mLateralFriction = lateralFriction;
        w2->mLongitudinalFriction = longitudinalFriction;

        // Left rear
        WheelSettingsWV* w3 = new WheelSettingsWV;
        w3->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
        w3->mSuspensionDirection = rear_suspension_dir;
        w3->mSteeringAxis = rear_steering_axis;
        w3->mWheelUp = rear_wheel_up;
        w3->mWheelForward = rear_wheel_forward;
        w3->mSuspensionMinLength = sRearSuspensionMinLength;
        w3->mSuspensionMaxLength = sRearSuspensionMaxLength;
        w3->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
        w3->mSuspensionSpring.mDamping = sRearSuspensionDamping;
        w3->mMaxSteerAngle = 0.0f;
        w3->mLateralFriction = lateralFriction;
        w3->mLongitudinalFriction = longitudinalFriction;

        // Right rear
        WheelSettingsWV* w4 = new WheelSettingsWV;
        w4->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
        w4->mSuspensionDirection = flip_x * rear_suspension_dir;
        w4->mSteeringAxis = flip_x * rear_steering_axis;
        w4->mWheelUp = flip_x * rear_wheel_up;
        w4->mWheelForward = flip_x * rear_wheel_forward;
        w4->mSuspensionMinLength = sRearSuspensionMinLength;
        w4->mSuspensionMaxLength = sRearSuspensionMaxLength;
        w4->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
        w4->mSuspensionSpring.mDamping = sRearSuspensionDamping;
        w4->mMaxSteerAngle = 0.0f;
        w2->mLateralFriction = lateralFriction;
        w2->mLongitudinalFriction = longitudinalFriction;

        vehicle.mWheels = {w1, w2, w3, w4};

        for (WheelSettings* w : vehicle.mWheels)
        {
            w->mRadius = wheel_radius;
            w->mWidth = wheel_width;
        }

        WheeledVehicleControllerSettings* controller = new WheeledVehicleControllerSettings;
        vehicle.mController = controller;

        // Differential
        controller->mDifferentials.resize(sFourWheelDrive ? 2 : 1);
        controller->mDifferentials[0].mLeftWheel = 0;
        controller->mDifferentials[0].mRightWheel = 1;
        if (sFourWheelDrive)
        {
            controller->mDifferentials[1].mLeftWheel = 2;
            controller->mDifferentials[1].mRightWheel = 3;

            // Split engine torque
            controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
        }

        // Anti rollbars
        if (sAntiRollbar)
        {
            vehicle.mAntiRollBars.resize(2);
            vehicle.mAntiRollBars[0].mLeftWheel = 0;
            vehicle.mAntiRollBars[0].mRightWheel = 1;
            vehicle.mAntiRollBars[1].mLeftWheel = 2;
            vehicle.mAntiRollBars[1].mRightWheel = 3;
        }

        mCarConstraint = new VehicleConstraint(*mCarBody, vehicle);
        mCarConstraint->SetVehicleCollisionTester(mCarTesters[0]);

        // The vehicle settings were tweaked with a buggy implementation of the longitudinal tire impulses, this meant that
        // PhysicsSettings::mNumVelocitySteps times more impulse could be applied than intended. To keep the behavior of the
        // vehicle the same we increase the max longitudinal impulse by the same factor. In a future version the vehicle will
        // be retweaked.
        static_cast<WheeledVehicleController*>(mCarConstraint->GetController())
            ->SetTireMaxImpulseCallback(
                [](uint, float& outLongitudinalImpulse, float& outLateralImpulse, float inSuspensionImpulse,
                   float inLongitudinalFriction, float inLateralFriction, float, float, float)
                {
                    outLongitudinalImpulse = 10.0f * inLongitudinalFriction * inSuspensionImpulse;
                    outLateralImpulse = inLateralFriction * inSuspensionImpulse;
                });

        m_PhysicsSystem.AddConstraint(mCarConstraint);
        m_PhysicsSystem.AddStepListener(mCarConstraint);
    }

    void PhysicsBase::CreateKart(RVec3 const& position, JPH::Quat const& quaternion)
    {
        [[maybe_unused]] const float sInitialRollAngle = 0;
        [[maybe_unused]] const float sMaxRollAngle = 1.0472;       // 60 degree
        [[maybe_unused]] const float sMaxSteeringAngle = 0.523599; // 30 degree
        [[maybe_unused]] const int sCollisionMode = 2;
        [[maybe_unused]] const bool sFourWheelDrive = true;
        [[maybe_unused]] const bool sAntiRollbar = true;
        [[maybe_unused]] const bool sLimitedSlipDifferentials = true;
        [[maybe_unused]] const bool sOverrideGravity =
            false; ///< If true, gravity is overridden to always oppose the ground normal
        [[maybe_unused]] const float sMaxEngineTorque = 500.0f;
        [[maybe_unused]] const float sClutchStrength = 10.0f;
        [[maybe_unused]] const float sFrontCasterAngle = 0.0f;
        [[maybe_unused]] const float sFrontKingPinAngle = 0.0f;
        [[maybe_unused]] const float sFrontCamber = 0.0f;
        [[maybe_unused]] const float sFrontToe = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionForwardAngle = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionSidewaysAngle = 0.0f;
        [[maybe_unused]] const float sFrontSuspensionMinLength = 0.3f;
        [[maybe_unused]] const float sFrontSuspensionMaxLength = 0.5f;
        [[maybe_unused]] const float sFrontSuspensionFrequency = 1.5f;
        [[maybe_unused]] const float sFrontSuspensionDamping = 0.5f;
        [[maybe_unused]] const float sRearSuspensionForwardAngle = 0.0f;
        [[maybe_unused]] const float sRearSuspensionSidewaysAngle = 0.0f;
        [[maybe_unused]] const float sRearCasterAngle = 0.0f;
        [[maybe_unused]] const float sRearKingPinAngle = 0.0f;
        [[maybe_unused]] const float sRearCamber = 0.0f;
        [[maybe_unused]] const float sRearToe = 0.0f;
        [[maybe_unused]] const float sRearSuspensionMinLength = 0.3f;
        [[maybe_unused]] const float sRearSuspensionMaxLength = 0.5f;
        [[maybe_unused]] const float sRearSuspensionFrequency = 1.5f;
        [[maybe_unused]] const float sRearSuspensionDamping = 1.0f;

        JPH::BodyInterface& bodyInterface = m_PhysicsSystem.GetBodyInterface();
        const float wheel_radius = 0.150f;
        const float wheel_width = 0.18f;
        const float half_vehicle_length = 0.725f;
        const float half_vehicle_width = 0.5f;
        const float half_vehicle_height = 0.1f;
        const float scaleFriction = 1.0f;
        const float wheelHeight = 0.35f;

        // Create collision testers
        mKartTesters[0] = new VehicleCollisionTesterRay(Layers::MOVING);
        mKartTesters[1] = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * wheel_width);
        mKartTesters[2] = new VehicleCollisionTesterCastCylinder(Layers::MOVING);

        // Create vehicle body

        RefConst<Shape> car_shape =
            OffsetCenterOfMassShapeSettings(Vec3(0.0f, 0.0f, 0.0f),
                                            new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length)))
                .Create()
                .Get();
        BodyCreationSettings car_body_settings(car_shape, position, quaternion, EMotionType::Dynamic, Layers::MOVING);
        car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
        car_body_settings.mMassPropertiesOverride.mMass = 2500.0f;
        mKartBody = bodyInterface.CreateBody(car_body_settings);
        bodyInterface.AddBody(mKartBody->GetID(), EActivation::Activate);

        // Create vehicle constraint
        VehicleConstraintSettings vehicle;
        vehicle.mDrawConstraintSize = 0.1f;
        vehicle.mMaxPitchRollAngle = sMaxRollAngle;

        // Suspension direction
        Vec3 front_suspension_dir =
            Vec3(Tan(sFrontSuspensionSidewaysAngle), -1, Tan(sFrontSuspensionForwardAngle)).Normalized();
        Vec3 front_steering_axis = Vec3(-Tan(sFrontKingPinAngle), 1, -Tan(sFrontCasterAngle)).Normalized();
        Vec3 front_wheel_up = Vec3(Sin(sFrontCamber), Cos(sFrontCamber), 0);
        Vec3 front_wheel_forward = Vec3(-Sin(sFrontToe), 0, Cos(sFrontToe));
        Vec3 rear_suspension_dir =
            Vec3(Tan(sRearSuspensionSidewaysAngle), -1, Tan(sRearSuspensionForwardAngle)).Normalized();
        Vec3 rear_steering_axis = Vec3(-Tan(sRearKingPinAngle), 1, -Tan(sRearCasterAngle)).Normalized();
        Vec3 rear_wheel_up = Vec3(Sin(sRearCamber), Cos(sRearCamber), 0);
        Vec3 rear_wheel_forward = Vec3(-Sin(sRearToe), 0, Cos(sRearToe));
        Vec3 flip_x(-1, 1, 1);

        JPH::LinearCurve longitudinalFriction;
        longitudinalFriction.Reserve(3);
        longitudinalFriction.AddPoint(0.0f, 0.0f * scaleFriction);
        longitudinalFriction.AddPoint(0.06f, 1.2f * scaleFriction);
        longitudinalFriction.AddPoint(0.2f, 1.0f * scaleFriction);

        JPH::LinearCurve lateralFriction;
        lateralFriction.Reserve(3);
        lateralFriction.AddPoint(0.0f, 0.0f * scaleFriction);
        lateralFriction.AddPoint(3.0f, 1.2f * scaleFriction);
        lateralFriction.AddPoint(20.0f, 1.0f * scaleFriction);

        // Wheels, left front
        WheelSettingsWV* w1 = new WheelSettingsWV;
        w1->mPosition = Vec3(half_vehicle_width, wheelHeight, half_vehicle_length - 2.0f * wheel_radius);
        w1->mSuspensionDirection = front_suspension_dir;
        w1->mSteeringAxis = front_steering_axis;
        w1->mWheelUp = front_wheel_up;
        w1->mWheelForward = front_wheel_forward;
        w1->mSuspensionMinLength = sFrontSuspensionMinLength;
        w1->mSuspensionMaxLength = sFrontSuspensionMaxLength;
        w1->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
        w1->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
        w1->mMaxSteerAngle = sMaxSteeringAngle;
        w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake
        w1->mLateralFriction = lateralFriction;
        w1->mLongitudinalFriction = longitudinalFriction;

        // Right front
        WheelSettingsWV* w2 = new WheelSettingsWV;
        w2->mPosition = Vec3(-half_vehicle_width, wheelHeight, half_vehicle_length - 2.0f * wheel_radius);
        w2->mSuspensionDirection = flip_x * front_suspension_dir;
        w2->mSteeringAxis = flip_x * front_steering_axis;
        w2->mWheelUp = flip_x * front_wheel_up;
        w2->mWheelForward = flip_x * front_wheel_forward;
        w2->mSuspensionMinLength = sFrontSuspensionMinLength;
        w2->mSuspensionMaxLength = sFrontSuspensionMaxLength;
        w2->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
        w2->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
        w2->mMaxSteerAngle = sMaxSteeringAngle;
        w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake
        w2->mLateralFriction = lateralFriction;
        w2->mLongitudinalFriction = longitudinalFriction;

        // Left rear
        WheelSettingsWV* w3 = new WheelSettingsWV;
        w3->mPosition = Vec3(half_vehicle_width, wheelHeight, -half_vehicle_length + 2.0f * wheel_radius);
        w3->mSuspensionDirection = rear_suspension_dir;
        w3->mSteeringAxis = rear_steering_axis;
        w3->mWheelUp = rear_wheel_up;
        w3->mWheelForward = rear_wheel_forward;
        w3->mSuspensionMinLength = sRearSuspensionMinLength;
        w3->mSuspensionMaxLength = sRearSuspensionMaxLength;
        w3->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
        w3->mSuspensionSpring.mDamping = sRearSuspensionDamping;
        w3->mMaxSteerAngle = 0.0f;
        w3->mLateralFriction = lateralFriction;
        w3->mLongitudinalFriction = longitudinalFriction;

        // Right rear
        WheelSettingsWV* w4 = new WheelSettingsWV;
        w4->mPosition = Vec3(-half_vehicle_width, wheelHeight, -half_vehicle_length + 2.0f * wheel_radius);
        w4->mSuspensionDirection = flip_x * rear_suspension_dir;
        w4->mSteeringAxis = flip_x * rear_steering_axis;
        w4->mWheelUp = flip_x * rear_wheel_up;
        w4->mWheelForward = flip_x * rear_wheel_forward;
        w4->mSuspensionMinLength = sRearSuspensionMinLength;
        w4->mSuspensionMaxLength = sRearSuspensionMaxLength;
        w4->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
        w4->mSuspensionSpring.mDamping = sRearSuspensionDamping;
        w4->mMaxSteerAngle = 0.0f;
        w2->mLateralFriction = lateralFriction;
        w2->mLongitudinalFriction = longitudinalFriction;

        vehicle.mWheels = {w1, w2, w3, w4};

        for (WheelSettings* w : vehicle.mWheels)
        {
            w->mRadius = wheel_radius;
            w->mWidth = wheel_width;
        }

        WheeledVehicleControllerSettings* controller = new WheeledVehicleControllerSettings;
        vehicle.mController = controller;

        // Differential
        controller->mDifferentials.resize(sFourWheelDrive ? 2 : 1);
        controller->mDifferentials[0].mLeftWheel = 0;
        controller->mDifferentials[0].mRightWheel = 1;
        if (sFourWheelDrive)
        {
            controller->mDifferentials[1].mLeftWheel = 2;
            controller->mDifferentials[1].mRightWheel = 3;

            // Split engine torque
            controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
        }

        // Anti rollbars
        if (sAntiRollbar)
        {
            vehicle.mAntiRollBars.resize(2);
            vehicle.mAntiRollBars[0].mLeftWheel = 0;
            vehicle.mAntiRollBars[0].mRightWheel = 1;
            vehicle.mAntiRollBars[1].mLeftWheel = 2;
            vehicle.mAntiRollBars[1].mRightWheel = 3;
        }

        mKartConstraint = new VehicleConstraint(*mKartBody, vehicle);
        mKartConstraint->SetVehicleCollisionTester(mKartTesters[0]);

        // The vehicle settings were tweaked with a buggy implementation of the longitudinal tire impulses, this meant that
        // PhysicsSettings::mNumVelocitySteps times more impulse could be applied than intended. To keep the behavior of the
        // vehicle the same we increase the max longitudinal impulse by the same factor. In a future version the vehicle will
        // be retweaked.
        static_cast<WheeledVehicleController*>(mKartConstraint->GetController())
            ->SetTireMaxImpulseCallback(
                [](uint, float& outLongitudinalImpulse, float& outLateralImpulse, float inSuspensionImpulse,
                   float inLongitudinalFriction, float inLateralFriction, float, float, float)
                {
                    outLongitudinalImpulse = 10.0f * inLongitudinalFriction * inSuspensionImpulse;
                    outLateralImpulse = inLateralFriction * inSuspensionImpulse;
                });

        m_PhysicsSystem.AddConstraint(mKartConstraint);
        m_PhysicsSystem.AddStepListener(mKartConstraint);
    }

} // namespace GfxRenderEngine
