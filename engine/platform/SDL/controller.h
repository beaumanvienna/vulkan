/* Controller Copyright (c) 2021 Controller Development Team
   https://github.com/beaumanvienna/gfxRenderController

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

#include <list>
#include <SDL.h>
#include <memory>
#include <vector>
#include <functional>

#include "engine.h"
#include "events/event.h"
#include "platform/SDL/controllerConfiguration.h"

namespace GfxRenderEngine
{
    class Controller
    {

    public:
        static const int MAX_NUMBER_OF_CONTROLLERS = 4;
        static const int ANALOG_DEAD_ZONE = 15000;

        enum ControllerSticks
        {
            LEFT_STICK,
            RIGHT_STICK
        };

        enum ID
        {
            NO_CONTROLLER = -1,
            FIRST_CONTROLLER = 0,
            SECOND_CONTROLLER,
            THIRD_CONTROLLER,
            FOURTH_CONTROLLER
        };

        enum Axis
        {
            LEFT_STICK_HORIZONTAL = 0,
            LEFT_STICK_VERTICAL,
            RIGHT_STICK_HORIZONTAL,
            RIGHT_STICK_VERTICAL,
            LEFT_TRIGGER,
            RIGHT_TRIGGER
        };

        enum ControllerCode
        {
            BUTTON_INVALID = -1,
            BUTTON_A,             // 0
            BUTTON_B,             // 1
            BUTTON_X,             // 2
            BUTTON_Y,             // 3
            BUTTON_BACK,          // 4
            BUTTON_GUIDE,         // 5
            BUTTON_START,         // 6
            BUTTON_LEFTSTICK,     // 7
            BUTTON_RIGHTSTICK,    // 8
            BUTTON_LEFTSHOULDER,  // 9
            BUTTON_RIGHTSHOULDER, // 10
            BUTTON_DPAD_UP,       // 11
            BUTTON_DPAD_DOWN,     // 12
            BUTTON_DPAD_LEFT,     // 13
            BUTTON_DPAD_RIGHT,    // 14
            BUTTON_MAX            // 15
        };

        Controller();
        ~Controller();

        bool Start();
        bool Restart();
        void OnUpdate();
        void Shutdown();

        void AddController(int indexID);
        void PrintJoyInfo(int indexID);
        void RemoveController(int instanceID);
        uint GetCount() const { return m_Controllers.size(); }
        SDL_GameController* GetGameController(int indexID) const;
        SDL_Joystick* GetJoystick(int indexID) const;
        void CloseAllControllers();
        bool CheckControllerIsSupported(int indexID);
        bool CheckMapping(SDL_JoystickGUID guid, bool& mappingOK, std::string& name);
        bool FindGuidInFile(std::string& filename, char* text2match, int length, std::string* lineRet);
        bool FindGuidInFile(const char* path /* GNU */, int resourceID /* MSVC */,
                            const std::string& resourceClass /* MSVC */, char* text2match, int length, std::string* lineRet);
        bool AddControllerToInternalDB(std::string entry);
        void RemoveDuplicatesInDB(void);
        int GetActiveController() { return m_ActiveController; }
        void EventLoop(SDL_Event& SDLevent);
        void ConfigEventLoop(SDL_Event& SDLevent);
        void SetNormalEventLoop()
        {
            m_EventLoop = [this](SDL_Event& SDLevent) { EventLoop(SDLevent); };
        }
        void SetConfigEventLoop()
        {
            m_EventLoop = [this](SDL_Event& SDLevent) { ConfigEventLoop(SDLevent); };
        }
        void StartConfig(int controllerID);
        bool ConfigIsRunning() const { return m_ControllerConfiguration.IsRunning(); }
        int GetConfigurationStep() { return m_ControllerConfiguration.GetConfigurationStep(); }
        int GetConfigurationActiveController() { return m_ControllerConfiguration.GetControllerID(); }
        bool MappingCreated() const { return m_ControllerConfiguration.MappingCreated(); }
        void GetGUID(int controllerID, std::string& guid);
        std::string GetName(int controllerID);

        void SetEventCallback(const EventCallbackFunction& callback);

    public:
        static ControllerConfiguration m_ControllerConfiguration;

    private:
        static constexpr auto DEBOUNCE_TIME = 500ms;

    private:
        bool m_Initialzed;
        EventCallbackFunction m_EventCallback;
        std::string m_Gamecontrollerdb, m_InternalDB;

        class ControllerData
        {
        public:
            int m_InstanceID;
            int m_IndexID;
            SDL_Joystick* m_Joystick;
            SDL_GameController* m_GameController;
            std::string m_Name;
            std::string m_NameDB;
            bool m_MappingOK;

            ControllerData();
            ~ControllerData();
        };

        std::list<ControllerData> m_Controllers;
        std::vector<int> m_InstanceToIndex;
        int m_ActiveController;
        std::function<void(SDL_Event& SDLevent)> m_EventLoop;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_TimeStamp;
    };
} // namespace GfxRenderEngine
