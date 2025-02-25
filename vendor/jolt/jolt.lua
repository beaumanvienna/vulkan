project "jolt"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir "Jolt/bin/%{cfg.buildcfg}"
    objdir ("Jolt/bin-int/%{cfg.buildcfg}")

    files
    {
        "Jolt/Core/Factory.cpp",
        "Jolt/AABBTree/AABBTreeBuilder.cpp",
        "Jolt/Core/Color.cpp",
        "Jolt/Core/IssueReporting.cpp",
        "Jolt/Core/JobSystemSingleThreaded.cpp",
        "Jolt/Core/JobSystemThreadPool.cpp",
        "Jolt/Core/JobSystemWithBarrier.cpp",
        "Jolt/Core/LinearCurve.cpp",
        "Jolt/Core/Memory.cpp",
        "Jolt/Core/Profiler.cpp",
        "Jolt/Core/Semaphore.cpp",
        "Jolt/Core/RTTI.cpp",
        "Jolt/Core/StringTools.cpp",
        "Jolt/Core/TickCounter.cpp",
        "Jolt/Geometry/ConvexHullBuilder.cpp",
        "Jolt/Geometry/ConvexHullBuilder2D.cpp",
        "Jolt/Geometry/Indexify.cpp",
        "Jolt/Math/Vec3.cpp",
        "Jolt/Geometry/OrientedBox.cpp",
        "Jolt/ObjectStream/SerializableObject.cpp",
        "Jolt/Physics/Body/Body.cpp",
        "Jolt/Physics/Body/BodyCreationSettings.cpp",
        "Jolt/Physics/Body/BodyInterface.cpp",
        "Jolt/Physics/Body/BodyManager.cpp",
        "Jolt/Physics/Body/MassProperties.cpp",
        "Jolt/Physics/Body/MotionProperties.cpp",
        "Jolt/Physics/Character/Character.cpp",
        "Jolt/Physics/Character/CharacterVirtual.cpp",
        "Jolt/Physics/Character/CharacterBase.cpp",
        "Jolt/Physics/Collision/BroadPhase/BroadPhase.cpp",
        "Jolt/Physics/Collision/BroadPhase/BroadPhaseBruteForce.cpp",
        "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuadTree.cpp",
        "Jolt/Physics/Collision/BroadPhase/QuadTree.cpp",
        "Jolt/Physics/Collision/CastSphereVsTriangles.cpp",
        "Jolt/Physics/Collision/CastConvexVsTriangles.cpp",
        "Jolt/Physics/Collision/CollideConvexVsTriangles.cpp",
        "Jolt/Physics/Collision/CollideSphereVsTriangles.cpp",
        "Jolt/Physics/Collision/CollisionDispatch.cpp",
        "Jolt/Physics/Collision/CollisionGroup.cpp",
        "Jolt/Physics/Collision/EstimateCollisionResponse.cpp",
        "Jolt/Physics/Collision/GroupFilter.cpp",
        "Jolt/Physics/Collision/GroupFilterTable.cpp",
        "Jolt/Physics/Collision/ManifoldBetweenTwoFaces.cpp",
        "Jolt/Physics/Collision/NarrowPhaseQuery.cpp",
        "Jolt/Physics/Collision/NarrowPhaseStats.cpp",
        "Jolt/Physics/Collision/PhysicsMaterial.cpp",
        "Jolt/Physics/Collision/PhysicsMaterialSimple.cpp",
        "Jolt/Physics/Collision/Shape/BoxShape.cpp",
        "Jolt/Physics/Collision/Shape/CapsuleShape.cpp",
        "Jolt/Physics/Collision/Shape/CompoundShape.cpp",
        "Jolt/Physics/Collision/Shape/ConvexShape.cpp",
        "Jolt/Physics/Collision/Shape/CylinderShape.cpp",
        "Jolt/Physics/Collision/Shape/ConvexHullShape.cpp",
        "Jolt/Physics/Collision/Shape/DecoratedShape.cpp",
        "Jolt/Physics/Collision/Shape/EmptyShape.cpp",
        "Jolt/Physics/Collision/Shape/HeightFieldShape.cpp",
        "Jolt/Physics/Collision/Shape/MeshShape.cpp",
        "Jolt/Physics/Collision/Shape/MutableCompoundShape.cpp",
        "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.cpp",
        "Jolt/Physics/Collision/Shape/PlaneShape.cpp",
        "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.cpp",
        "Jolt/Physics/Collision/Shape/ScaledShape.cpp",
        "Jolt/Physics/Collision/Shape/Shape.cpp",
        "Jolt/Physics/Collision/Shape/SphereShape.cpp",
        "Jolt/Physics/Collision/Shape/StaticCompoundShape.cpp",
        "Jolt/Physics/Collision/Shape/TaperedCapsuleShape.cpp",
        "Jolt/Physics/Collision/Shape/TaperedCylinderShape.cpp",
        "Jolt/Physics/Collision/Shape/TriangleShape.cpp",
        "Jolt/Physics/Collision/TransformedShape.cpp",
        "Jolt/Physics/Constraints/ConeConstraint.cpp",
        "Jolt/Physics/Constraints/Constraint.cpp",
        "Jolt/Physics/Constraints/ConstraintManager.cpp",
        "Jolt/Physics/Constraints/ContactConstraintManager.cpp",
        "Jolt/Physics/Constraints/DistanceConstraint.cpp",
        "Jolt/Physics/Constraints/FixedConstraint.cpp",
        "Jolt/Physics/Constraints/GearConstraint.cpp",
        "Jolt/Physics/Constraints/HingeConstraint.cpp",
        "Jolt/Physics/Constraints/MotorSettings.cpp",
        "Jolt/Physics/Constraints/PathConstraint.cpp",
        "Jolt/Physics/Constraints/PathConstraintPath.cpp",
        "Jolt/Physics/Constraints/PathConstraintPathHermite.cpp",
        "Jolt/Physics/Constraints/PointConstraint.cpp",
        "Jolt/Physics/Constraints/PulleyConstraint.cpp",
        "Jolt/Physics/Constraints/RackAndPinionConstraint.cpp",
        "Jolt/Physics/Constraints/SixDOFConstraint.cpp",
        "Jolt/Physics/Constraints/SliderConstraint.cpp",
        "Jolt/Physics/Constraints/SpringSettings.cpp",
        "Jolt/Physics/Constraints/SwingTwistConstraint.cpp",
        "Jolt/Physics/Constraints/TwoBodyConstraint.cpp",
        "Jolt/Physics/DeterminismLog.cpp",
        "Jolt/Physics/IslandBuilder.cpp",
        "Jolt/Physics/LargeIslandSplitter.cpp",
        "Jolt/Physics/PhysicsScene.cpp",
        "Jolt/Physics/PhysicsSystem.cpp",
        "Jolt/Physics/PhysicsUpdateContext.cpp",
        "Jolt/Physics/Ragdoll/Ragdoll.cpp",
        "Jolt/Physics/SoftBody/SoftBodyCreationSettings.cpp",
        "Jolt/Physics/SoftBody/SoftBodyMotionProperties.cpp",
        "Jolt/Physics/SoftBody/SoftBodyShape.cpp",
        "Jolt/Physics/SoftBody/SoftBodySharedSettings.cpp",
        "Jolt/Physics/StateRecorderImpl.cpp",
        "Jolt/Physics/Vehicle/MotorcycleController.cpp",
        "Jolt/Physics/Vehicle/TrackedVehicleController.cpp",
        "Jolt/Physics/Vehicle/VehicleAntiRollBar.cpp",
        "Jolt/Physics/Vehicle/VehicleCollisionTester.cpp",
        "Jolt/Physics/Vehicle/VehicleController.cpp",
        "Jolt/Physics/Vehicle/VehicleConstraint.cpp",
        "Jolt/Physics/Vehicle/VehicleDifferential.cpp",
        "Jolt/Physics/Vehicle/VehicleEngine.cpp",
        "Jolt/Physics/Vehicle/VehicleTrack.cpp",
        "Jolt/Physics/Vehicle/VehicleTransmission.cpp",
        "Jolt/Physics/Vehicle/Wheel.cpp",
        "Jolt/Physics/Vehicle/WheeledVehicleController.cpp",
        "Jolt/RegisterTypes.cpp",
        "Jolt/Renderer/DebugRenderer.cpp",
        "Jolt/Renderer/DebugRendererPlayback.cpp",
        "Jolt/Renderer/DebugRendererRecorder.cpp",
        "Jolt/Renderer/DebugRendererSimple.cpp",
        "Jolt/Skeleton/SkeletalAnimation.cpp",
        "Jolt/Skeleton/Skeleton.cpp",
        "Jolt/Skeleton/SkeletonMapper.cpp",
        "Jolt/Skeleton/SkeletonPose.cpp",
        "Jolt/TriangleSplitter/TriangleSplitter.cpp",
        "Jolt/TriangleSplitter/TriangleSplitterBinning.cpp",
        "Jolt/TriangleSplitter/TriangleSplitterMean.cpp",
        "Jolt/ObjectStream/ObjectStream.cpp",
        "Jolt/ObjectStream/ObjectStreamBinaryIn.cpp",
        "Jolt/ObjectStream/ObjectStreamBinaryOut.cpp",
        "Jolt/ObjectStream/ObjectStreamIn.cpp",
        "Jolt/ObjectStream/ObjectStreamOut.cpp",
        "Jolt/ObjectStream/ObjectStreamTextIn.cpp",
        "Jolt/ObjectStream/ObjectStreamTextOut.cpp",
        "Jolt/ObjectStream/TypeDeclarations.cpp"
    }

    includedirs
    {
        "./"
    }


    defines
    {
        "JPH_PROFILE_ENABLED",
        "JPH_DEBUG_RENDERER",
        "JPH_OBJECT_STREAM",

        "JPH_USE_AVX",
        "JPH_USE_AVX2",
        "JPH_USE_F16C",
        "JPH_USE_FMADD",
        "JPH_USE_LZCNT",
        "JPH_USE_SSE4_1",
        "JPH_USE_SSE4_2",
        "JPH_USE_TZCNT"
    }

    filter "system:linux"
        pic "On"

    filter "configurations:Debug"
        runtime "Debug"
        defines { "DEBUG" }
        symbols "on"

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -ggdb -fno-rtti -fno-exceptions -std=c++17 -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c -mfma -mfpmath=sse" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -fno-rtti -fno-exceptions -O3 -std=c++17 -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c -mfma -mfpmath=sse" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -fno-rtti -fno-exceptions -O3 -std=c++17 -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c -mfma -mfpmath=sse" }

    filter "configurations:Release"
        runtime "Release"
        defines { "NDEBUG" }
        optimize "on"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"

    if _ACTION == 'clean' then
        print("cleaning jolt ...")
        os.rmdir("./Jolt/bin")
        os.rmdir("./Jolt/bin-int")
        os.remove("./Jolt/**.make")
        os.remove("./Jolt/Makefile")
        print("done.")
    end