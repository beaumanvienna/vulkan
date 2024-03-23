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

#include "engine.h"

namespace GfxRenderEngine
{
    class ControllerConfiguration
    {

    public:
        enum TextID
        {
            TEXT1 = 0,
            TEXT2
        };

        enum ConfigState
        {
            STATE_CONF_BUTTON_DPAD_UP = 0,   // 0
            STATE_CONF_BUTTON_DPAD_DOWN,     // 1
            STATE_CONF_BUTTON_DPAD_LEFT,     // 2
            STATE_CONF_BUTTON_DPAD_RIGHT,    // 3
            STATE_CONF_BUTTON_A,             // 4
            STATE_CONF_BUTTON_B,             // 5
            STATE_CONF_BUTTON_X,             // 6
            STATE_CONF_BUTTON_Y,             // 7
            STATE_CONF_BUTTON_BACK,          // 8
            STATE_CONF_BUTTON_GUIDE,         // 9
            STATE_CONF_BUTTON_START,         // 10
            STATE_CONF_BUTTON_LEFTSTICK,     // 11
            STATE_CONF_BUTTON_RIGHTSTICK,    // 12
            STATE_CONF_BUTTON_LEFTSHOULDER,  // 13
            STATE_CONF_BUTTON_RIGHTSHOULDER, // 14
            STATE_CONF_AXIS_LEFTSTICK_X,     // 15
            STATE_CONF_AXIS_LEFTSTICK_Y,     // 16
            STATE_CONF_AXIS_RIGHTSTICK_X,    // 17
            STATE_CONF_AXIS_RIGHTSTICK_Y,    // 18
            STATE_CONF_AXIS_LEFTTRIGGER,     // 19
            STATE_CONF_AXIS_RIGHTTRIGGER,    // 20
            STATE_CONF_BUTTON_LEFTTRIGGER,   // 21
            STATE_CONF_BUTTON_RIGHTTRIGGER,  // 22
            STATE_CONF_SKIP_ITEM,            // 23
            STATE_CONF_MAX                   // 24
        };

        enum ButtonCodes
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

        enum ControllerID
        {
            NO_CONTROLLER = -1,
            FIRST_CONTROLLER = 0,
            SECOND_CONTROLLER,
            THIRD_CONTROLLER,
            FOURTH_CONTROLLER
        };

        enum ReportedConfigState
        {
            REPORTED_STATE_INACTIVE = -1,
            REPORTED_STATE_UP,
            REPORTED_STATE_DOWN,
            REPORTED_STATE_LEFT,
            REPORTED_STATE_RIGHT,
            REPORTED_STATE_SOUTH,
            REPORTED_STATE_EAST,
            REPORTED_STATE_WEST,
            REPORTED_STATE_NORTH,
            REPORTED_STATE_LSTICK,
            REPORTED_STATE_RSTICK,
            REPORTED_STATE_LTRIGGER,
            REPORTED_STATE_RTRIGGER,
            REPORTED_STATE_SELECT,
            REPORTED_STATE_START,
            REPORTED_STATE_GUIDE
        };

    public:
        ControllerConfiguration() {}
        ~ControllerConfiguration() {}

        void Start(int controllerID);
        void Reset(void);
        bool IsRunning() const { return m_Running; }
        bool UpdateControllerText() const { return m_UpdateControllerText; }
        void ResetControllerText() { m_UpdateControllerText = false; }
        int GetControllerID() const { return m_ControllerID; }
        std::string& GetText(int textNumber) { return textNumber ? m_Text2 : m_Text1; }
        bool MappingCreated() const { return m_MappingCreated; }

        void StatemachineConf(int cmd);
        void StatemachineConfHat(int hat, int value);
        void StatemachineConfAxis(int cmd, bool negative);
        void SkipConfigStep() { StatemachineConf(STATE_CONF_SKIP_ITEM); }
        int GetConfigurationStep() { return m_ReportedState; }
        std::string GetDatabaseEntry() const { return m_DatabaseEntry; }

    private:
        bool CheckAxis(int cmd);
        bool CheckTrigger(int cmd);
        void SetControllerConfText(std::string text1, std::string text2 = "");
        void SetMapping(void);

    private:
        std::string m_Text1, m_Text2;
        bool m_UpdateControllerText;

        int m_ConfigurationState;
        int m_ControllerButton[STATE_CONF_MAX];

        int m_SecondRun;
        int m_SecondRunHat;
        int m_SecondRunValue;

        bool m_Running = false;
        int m_ControllerID = NO_CONTROLLER;

        int m_Axis[4];
        bool m_AxisValue[4];
        int m_AxisIterator;

        int m_Hat[4];
        int m_HatValue[4];
        int m_HatIterator;

        int m_CountX, m_CountY, m_ValueX, m_ValueY;
        bool m_MappingCreated;
        int m_ReportedState;
        std::string m_DatabaseEntry;
    };
} // namespace GfxRenderEngine
