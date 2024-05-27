/* Engine Copyright (c) 2022 Engine Development Team
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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#pragma once

#include <memory>

#include "engine.h"
#include "SDL/controller.h"
#include "events/controllerEvent.h"
#include "platform/mouseButtonCodes.h"
#include "platform/keyCodes.h"

namespace GfxRenderEngine
{
    class Input
    {

    public:
        static void Start(Controller* controller);

        // keyboard
        static bool IsKeyPressed(const KeyCode key);

        // mouse
        static bool IsMouseButtonPressed(const MouseCode button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();

        // controller
        static glm::vec2 GetControllerStick(const int indexID, Controller::ControllerSticks stick);
        static float GetControllerTrigger(const int indexID, Controller::Axis axis);
        static bool IsControllerButtonPressed(const int indexID, const Controller::ControllerCode button);
        static uint GetControllerCount();
        static int GetActiveController();
        static void StartControllerConfig(int controllerID);
        static bool ConfigurationRunning();
        static int GetConfigurationStep();
        static int GetConfigurationActiveController();
        static int ControllerMappingCreated();
        static std::string GetControlerName(int controllerID);
        static void GetControllerGUID(int controllerID, std::string& guid);

        static void* GetControllerJoy(const int indexID);
        static void* GetControllerGamecontroller(const int indexID);

    private:
        static Controller* m_Controller;
    };
} // namespace GfxRenderEngine
