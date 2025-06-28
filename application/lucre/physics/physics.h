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

#include <memory>

#include "engine.h"
#include "scene/scene.h"
#include "auxiliary/timestep.h"

namespace GfxRenderEngine
{
    class Physics
    {
    public:
        enum VehicleType
        {
            CAR = 0,
            KART
        };

        struct VehicleControl
        {
            float inForward{0.0f};
            float inRight{0.0f};
            float inBrake{0.0f};
            float inHandBrake{0.0f};
        };

        enum GameObjects
        {
            GAME_OBJECT_GROUND_PLANE = 0,
            GAME_OBJECT_SPHERE,
            GAME_OBJECT_MUSHROOM,
            GAME_OBJECT_CAR,
            GAME_OBJECT_WHEEL_FRONT_LEFT,
            GAME_OBJECT_WHEEL_FRONT_RIGHT,
            GAME_OBJECT_WHEEL_REAR_LEFT,
            GAME_OBJECT_WHEEL_REAR_RIGHT,
            GAME_OBJECT_KART,
            GAME_OBJECT_KART_WHEEL_FRONT_LEFT,
            GAME_OBJECT_KART_WHEEL_FRONT_RIGHT,
            GAME_OBJECT_KART_WHEEL_REAR_LEFT,
            GAME_OBJECT_KART_WHEEL_REAR_RIGHT,
            NUM_GAME_OBJECTS
        };

        enum WheelNumbers
        {
            WHEEL_FRONT_LEFT = 0,
            WHEEL_FRONT_RIGHT,
            WHEEL_REAR_LEFT,
            WHEEL_REAR_RIGHT,
            NUM_WHEELS
        };

        struct CarParameters
        {
            glm::vec3 m_Position{0.0f};
            glm::vec3 m_Rotation{0.0f};
        };

        struct GroundSpec
        {
            glm::vec3 m_Scale{0.0f};
            glm::vec3 m_Position{0.0f};
            std::string m_Filepath;
            float m_Friction{2.0f};
        };

    public:
        virtual ~Physics() = default;
        virtual void OnUpdate(Timestep timestep, VehicleControl const& vehicleControl, VehicleType vehicleType) = 0;
        virtual void CreateGroundPlane(GroundSpec const& groundSpec) = 0;
        virtual void LoadModels(CarParameters const& carParameters, CarParameters const& kartParameters) = 0;
        virtual void CreateMeshTerrain(entt::entity, const std::string& filepath, float friction) = 0;
        virtual void Draw(GfxRenderEngine::Camera const& cam0) = 0;
        virtual void SetGameObject(uint gameObject, entt::entity gameObjectID) = 0;
        virtual void SetWheelTranslation(uint wheelNumber, glm::mat4 const& translation) = 0;
        virtual void SetWheelScale(uint wheelNumber, glm::mat4 const& scale) = 0;
        virtual void SetKartWheelTranslation(uint wheelNumber, glm::mat4 const& translation) = 0;
        virtual void SetKartWheelScale(uint wheelNumber, glm::mat4 const& scale) = 0;
        virtual void SetCarHeightOffset(float carHeightOffset) = 0;
        virtual void SetKartHeightOffset(float kartHeightOffset) = 0;

        static std::unique_ptr<Physics> Create(Scene& scene);
    };
} // namespace GfxRenderEngine
