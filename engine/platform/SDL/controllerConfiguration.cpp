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

#include "platform/SDL/controllerConfiguration.h"
#include "platform/input.h"

namespace GfxRenderEngine
{

    void ControllerConfiguration::Start(int controllerID)
    {
        Reset();
        m_Running = true;
        m_ControllerID = controllerID;
        SetControllerConfText("press dpad up", "(or use ENTER to skip this button)");
        m_MappingCreated = false;
    }

    void ControllerConfiguration::Reset(void)
    {
        m_Running = false;
        m_ControllerID = NO_CONTROLLER;

        for (int i = 0; i < STATE_CONF_MAX; i++)
        {
            m_ControllerButton[i] = STATE_CONF_SKIP_ITEM;
        }

        for (int i = 0; i < 4; i++)
        {
            m_Hat[i] = -1;
            m_HatValue[i] = -1;
            m_Axis[i] = -1;
            m_AxisValue[i] = false;
        }
        m_HatIterator = 0;
        m_AxisIterator = 0;
        m_SecondRun = -1;
        m_SecondRunHat = -1;
        m_SecondRunValue = -1;

        m_ConfigurationState = STATE_CONF_BUTTON_DPAD_UP;
        m_ReportedState = REPORTED_STATE_UP;

        m_UpdateControllerText = false;
        m_Text1 = m_Text2 = "";
    }

    void ControllerConfiguration::StatemachineConf(int cmd)
    {
        if ((cmd == STATE_CONF_SKIP_ITEM) && (m_ConfigurationState > STATE_CONF_BUTTON_RIGHTSHOULDER))
        {
            StatemachineConfAxis(STATE_CONF_SKIP_ITEM, false);
            return;
        }

        if ((Input::GetActiveController() == m_ControllerID) || (cmd == STATE_CONF_SKIP_ITEM))
        {
            switch (m_ConfigurationState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    if (m_SecondRun == -1)
                    {
                        SetControllerConfText("press dpad up", "(or use ENTER to skip this button)");
                        m_SecondRun = cmd;
                    }
                    else if (m_SecondRun == cmd)
                    {
                        m_ControllerButton[m_ConfigurationState] = cmd;
                        m_ConfigurationState = STATE_CONF_BUTTON_DPAD_DOWN;
                        m_ReportedState = REPORTED_STATE_DOWN;
                        SetControllerConfText("press dpad down");
                        m_SecondRun = -1;
                    }
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_DPAD_LEFT;
                    m_ReportedState = REPORTED_STATE_LEFT;
                    SetControllerConfText("press dpad left");
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_DPAD_RIGHT;
                    m_ReportedState = REPORTED_STATE_RIGHT;
                    SetControllerConfText("press dpad right");
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_A;
                    m_ReportedState = REPORTED_STATE_SOUTH;
                    SetControllerConfText("press south button (lower)");
                    break;
                case STATE_CONF_BUTTON_A:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_B;
                    m_ReportedState = REPORTED_STATE_EAST;
                    SetControllerConfText("press east button (right)");
                    break;
                case STATE_CONF_BUTTON_B:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_X;
                    m_ReportedState = REPORTED_STATE_WEST;
                    SetControllerConfText("press west button (left)");
                    break;
                case STATE_CONF_BUTTON_X:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_Y;
                    m_ReportedState = REPORTED_STATE_NORTH;
                    SetControllerConfText("press north button (upper)");
                    break;
                case STATE_CONF_BUTTON_Y:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_LEFTSTICK;
                    m_ReportedState = REPORTED_STATE_LSTICK;
                    SetControllerConfText("press left stick button");
                    break;
                case STATE_CONF_BUTTON_LEFTSTICK:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_RIGHTSTICK;
                    m_ReportedState = REPORTED_STATE_RSTICK;
                    SetControllerConfText("press right stick button");
                    break;
                case STATE_CONF_BUTTON_RIGHTSTICK:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_LEFTSHOULDER;
                    m_ReportedState = REPORTED_STATE_LTRIGGER;
                    SetControllerConfText("press left front shoulder");
                    break;
                case STATE_CONF_BUTTON_LEFTSHOULDER:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_RIGHTSHOULDER;
                    m_ReportedState = REPORTED_STATE_RTRIGGER;
                    SetControllerConfText("press right front shoulder");
                    break;
                case STATE_CONF_BUTTON_RIGHTSHOULDER:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_BACK;
                    m_ReportedState = REPORTED_STATE_SELECT;
                    SetControllerConfText("press select button");
                    break;
                case STATE_CONF_BUTTON_BACK:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_START;
                    m_ReportedState = REPORTED_STATE_START;
                    SetControllerConfText("press start button");
                    break;
                case STATE_CONF_BUTTON_START:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_BUTTON_GUIDE;
                    m_ReportedState = REPORTED_STATE_GUIDE;
                    SetControllerConfText("press guide button");
                    break;
                case STATE_CONF_BUTTON_GUIDE:
                    m_ControllerButton[m_ConfigurationState] = cmd;
                    m_ConfigurationState = STATE_CONF_AXIS_LEFTSTICK_X;
                    m_ReportedState = REPORTED_STATE_LSTICK;
                    SetControllerConfText("twirl left stick");
                    m_CountX = 0;
                    m_CountY = 0;
                    m_ValueX = -1;
                    m_ValueY = -1;
                    break;
                case STATE_CONF_AXIS_LEFTTRIGGER:
                    m_ControllerButton[STATE_CONF_BUTTON_LEFTTRIGGER] = cmd;
                    m_ConfigurationState = STATE_CONF_AXIS_RIGHTTRIGGER;
                    m_ReportedState = REPORTED_STATE_RTRIGGER;
                    SetControllerConfText("press right rear shoulder");
                    break;
                case STATE_CONF_AXIS_RIGHTTRIGGER:
                    m_ControllerButton[STATE_CONF_BUTTON_RIGHTTRIGGER] = cmd;
                    SetMapping();
                    break;
                default:
                    (void)0;
                    break;
            }
        }
    }

    void ControllerConfiguration::StatemachineConfAxis(int cmd, bool negative)
    {
        if ((m_Running) && (m_ConfigurationState >= STATE_CONF_AXIS_LEFTSTICK_X))
        {
            if ((Input::GetActiveController() == m_ControllerID) || (cmd == STATE_CONF_SKIP_ITEM))
            {
                switch (m_ConfigurationState)
                {
                    case STATE_CONF_AXIS_LEFTSTICK_X:
                    case STATE_CONF_AXIS_LEFTSTICK_Y:
                        if (CheckAxis(cmd))
                        {
                            m_CountX = 0;
                            m_CountY = 0;
                            m_ValueX = -1;
                            m_ValueY = -1;

                            m_ConfigurationState = STATE_CONF_AXIS_RIGHTSTICK_X;
                            m_ReportedState = REPORTED_STATE_RSTICK;
                            SetControllerConfText("twirl right stick");
                        }
                        break;
                    case STATE_CONF_AXIS_RIGHTSTICK_X:
                    case STATE_CONF_AXIS_RIGHTSTICK_Y:
                        if (cmd == STATE_CONF_SKIP_ITEM)
                        {
                            m_CountX = 0;
                            m_CountY = 0;
                            m_ValueX = -1;
                            m_ValueY = -1;

                            m_ConfigurationState = STATE_CONF_AXIS_LEFTTRIGGER;
                            m_ReportedState = REPORTED_STATE_LTRIGGER;
                            SetControllerConfText("press left rear shoulder");
                        }
                        else if ((cmd != m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]) &&
                                 (cmd != m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]))
                        {
                            if (CheckAxis(cmd))
                            {
                                m_CountX = 0;
                                m_CountY = 0;
                                m_ValueX = -1;
                                m_ValueY = -1;

                                m_ConfigurationState = STATE_CONF_AXIS_LEFTTRIGGER;
                                m_ReportedState = REPORTED_STATE_LTRIGGER;
                                SetControllerConfText("press left rear shoulder");
                            }
                        }
                        break;
                    case STATE_CONF_AXIS_LEFTTRIGGER:
                        if (cmd == STATE_CONF_SKIP_ITEM)
                        {
                            m_CountX = 0;
                            m_ValueX = -1;

                            m_ConfigurationState = STATE_CONF_AXIS_RIGHTTRIGGER;
                            m_ReportedState = REPORTED_STATE_RTRIGGER;
                            SetControllerConfText("press right rear shoulder");
                        }
                        else if ((cmd != m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]) &&
                                 (cmd != m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]))
                        {
                            if (CheckTrigger(cmd))
                            {
                                m_CountX = 0;
                                m_ValueX = -1;

                                m_ConfigurationState = STATE_CONF_AXIS_RIGHTTRIGGER;
                                m_ReportedState = REPORTED_STATE_RTRIGGER;
                                SetControllerConfText("press right rear shoulder");
                            }
                        }

                        break;
                    case STATE_CONF_AXIS_RIGHTTRIGGER:
                        if (cmd == STATE_CONF_SKIP_ITEM)
                        {
                            m_CountX = 0;
                            m_ValueX = -1;
                            SetMapping();
                        }
                        else if (cmd != m_ControllerButton[STATE_CONF_AXIS_LEFTTRIGGER])
                        {
                            if (CheckTrigger(cmd))
                            {
                                m_CountX = 0;
                                m_ValueX = -1;
                                SetMapping();
                            }
                        }
                        break;
                    default:
                        (void)0;
                        break;
                }
            }
        }
        else if ((m_Running) && (m_ConfigurationState <= STATE_CONF_BUTTON_DPAD_RIGHT))
        {
            if ((Input::GetActiveController() == m_ControllerID) || (cmd == STATE_CONF_SKIP_ITEM))
            {
                m_Axis[m_AxisIterator] = cmd;
                m_AxisValue[m_AxisIterator] = negative;
                switch (m_ConfigurationState)
                {
                    case STATE_CONF_BUTTON_DPAD_UP:
                        SetControllerConfText("press dpad down");
                        break;
                    case STATE_CONF_BUTTON_DPAD_DOWN:
                        SetControllerConfText("press dpad left");
                        break;
                    case STATE_CONF_BUTTON_DPAD_LEFT:
                        SetControllerConfText("press dpad right");
                        break;
                    case STATE_CONF_BUTTON_DPAD_RIGHT:
                        SetControllerConfText("press south button (lower)");
                        break;
                    default:
                        (void)0;
                        break;
                }
                m_ConfigurationState++;
                m_ReportedState++;
                m_AxisIterator++;
            }
        }
    }

    bool ControllerConfiguration::CheckAxis(int cmd)
    {
        if (cmd == STATE_CONF_SKIP_ITEM)
            return true;

        bool ok = false;

        if ((m_CountX > 10) && (m_CountY > 10))
        {
            m_ControllerButton[m_ConfigurationState] = m_ValueX;
            m_ControllerButton[m_ConfigurationState + 1] = m_ValueY;
            ok = true;
        }

        if ((m_ValueX != -1) && (m_ValueY != -1))
        {
            if (m_ValueX == cmd)
                m_CountX++;
            if (m_ValueY == cmd)
                m_CountY++;
        }

        if ((m_ValueX != -1) && (m_ValueY == -1))
        {
            if (m_ValueX > cmd)
            {
                m_ValueY = m_ValueX;
                m_ValueX = cmd;
            }
            else if (m_ValueX != cmd)
            {
                m_ValueY = cmd;
            }
        }

        if ((m_ValueX == -1) && (m_ValueY == -1))
        {
            m_ValueX = cmd;
        }

        return ok;
    }

    bool ControllerConfiguration::CheckTrigger(int cmd)
    {
        if (cmd == STATE_CONF_SKIP_ITEM)
            return true;
        bool ok = false;

        if (m_CountX > 100)
        {
            m_ControllerButton[m_ConfigurationState] = m_ValueX;
            ok = true;
        }

        if (m_ValueX != -1)
        {
            if (m_ValueX == cmd)
                m_CountX++;
        }

        if (m_ValueX == -1)
        {
            m_ValueX = cmd;
        }

        return ok;
    }

    void ControllerConfiguration::StatemachineConfHat(int hat, int value)
    {
        if (m_ConfigurationState > STATE_CONF_BUTTON_DPAD_RIGHT)
            return;

        if (Input::GetActiveController() == m_ControllerID)
        {
            m_Hat[m_HatIterator] = hat;
            m_HatValue[m_HatIterator] = value;

            switch (m_ConfigurationState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    if ((m_SecondRunHat == -1) && (m_SecondRunValue == -1))
                    {
                        SetControllerConfText("press dpad up again");
                        m_SecondRunHat = hat;
                        m_SecondRunValue = value;
                    }
                    else if ((m_SecondRunHat == hat) && (m_SecondRunValue == value))
                    {
                        m_HatIterator++;
                        m_ConfigurationState = STATE_CONF_BUTTON_DPAD_DOWN;
                        m_ReportedState = REPORTED_STATE_DOWN;
                        SetControllerConfText("press dpad down");
                        m_SecondRunHat = -1;
                        m_SecondRunValue = -1;
                    }
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    m_ConfigurationState = STATE_CONF_BUTTON_DPAD_LEFT;
                    m_ReportedState = REPORTED_STATE_LEFT;
                    SetControllerConfText("press dpad left");
                    m_HatIterator++;
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    m_ConfigurationState = STATE_CONF_BUTTON_DPAD_RIGHT;
                    m_ReportedState = REPORTED_STATE_RIGHT;
                    SetControllerConfText("press dpad right");
                    m_HatIterator++;
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    m_ConfigurationState = STATE_CONF_BUTTON_A;
                    m_ReportedState = REPORTED_STATE_SOUTH;
                    SetControllerConfText("press south button (lower)");
                    m_HatIterator++;
                    break;
                default:
                    (void)0;
                    break;
            }
        }
    }
    void ControllerConfiguration::SetControllerConfText(std::string text1, std::string text2)
    {
        m_Text1 = text1;
        if (text2 != "")
            m_Text2 = text2;

        m_UpdateControllerText = true;
    }

    void ControllerConfiguration::SetMapping(void)
    {
        std::string name;

        name = Input::GetControlerName(m_ControllerID);
        int pos;
        while (static_cast<size_t>(pos = name.find(",")) != std::string::npos)
        {
            name = name.erase(pos, 1);
        }
        if (name.length() > 45)
            name = name.substr(0, 45);

        Input::GetControllerGUID(m_ControllerID, m_DatabaseEntry);
        m_DatabaseEntry = m_DatabaseEntry + "," + name;

        if (m_ControllerButton[STATE_CONF_BUTTON_A] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",a:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_A]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_B] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",b:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_B]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_BACK] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",back:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_BACK]);
        }

        if (m_ControllerButton[STATE_CONF_BUTTON_DPAD_DOWN] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry =
                m_DatabaseEntry + ",dpdown:b" + std::to_string(m_ControllerButton[STATE_CONF_BUTTON_DPAD_DOWN]);
        }
        else if ((m_Hat[1] != -1) && (m_HatValue[1] != -1))
        {
            m_DatabaseEntry = m_DatabaseEntry + ",dpdown:h" + std::to_string(m_Hat[1]) + "." + std::to_string(m_HatValue[1]);
        }
        else if (m_Axis[1] != -1)
        {
            if (m_AxisValue[1])
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpdown:-a" + std::to_string(m_Axis[1]);
            }
            else
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpdown:+a" + std::to_string(m_Axis[1]);
            }
        }

        if (m_ControllerButton[STATE_CONF_BUTTON_DPAD_LEFT] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry =
                m_DatabaseEntry + ",dpleft:b" + std::to_string(m_ControllerButton[STATE_CONF_BUTTON_DPAD_LEFT]);
        }
        else if ((m_Hat[2] != -1) && (m_HatValue[2] != -1))
        {
            m_DatabaseEntry = m_DatabaseEntry + ",dpleft:h" + std::to_string(m_Hat[2]) + "." + std::to_string(m_HatValue[2]);
        }
        else if (m_Axis[2] != -1)
        {
            if (m_AxisValue[2])
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpleft:-a" + std::to_string(m_Axis[2]);
            }
            else
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpleft:+a" + std::to_string(m_Axis[2]);
            }
        }

        if (m_ControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry =
                m_DatabaseEntry + ",dpright:b" + std::to_string(m_ControllerButton[STATE_CONF_BUTTON_DPAD_RIGHT]);
        }
        else if ((m_Hat[3] != -1) && (m_HatValue[3] != -1))
        {
            m_DatabaseEntry =
                m_DatabaseEntry + ",dpright:h" + std::to_string(m_Hat[3]) + "." + std::to_string(m_HatValue[3]);
        }
        else if (m_Axis[3] != -1)
        {
            if (m_AxisValue[3])
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpright:-a" + std::to_string(m_Axis[3]);
            }
            else
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpright:+a" + std::to_string(m_Axis[3]);
            }
        }

        if (m_ControllerButton[STATE_CONF_BUTTON_DPAD_UP] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry = m_DatabaseEntry + ",dpup:b" + std::to_string(m_ControllerButton[STATE_CONF_BUTTON_DPAD_UP]);
        }
        else if ((m_Hat[0] != -1) && (m_HatValue[0] != -1))
        {
            m_DatabaseEntry = m_DatabaseEntry + ",dpup:h" + std::to_string(m_Hat[0]) + "." + std::to_string(m_HatValue[0]);
        }
        else if (m_Axis[0] != -1)
        {
            if (m_AxisValue[0])
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpup:-a" + std::to_string(m_Axis[0]);
            }
            else
            {
                m_DatabaseEntry = m_DatabaseEntry + ",dpup:+a" + std::to_string(m_Axis[0]);
            }
        }

        if (m_ControllerButton[STATE_CONF_BUTTON_GUIDE] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",guide:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_GUIDE]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",leftshoulder:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_LEFTSHOULDER]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_LEFTSTICK] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",leftstick:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_LEFTSTICK]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_LEFTTRIGGER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",lefttrigger:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_LEFTTRIGGER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",lefttrigger:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_LEFTTRIGGER]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_X] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",leftx:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",lefty:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",rightshoulder:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_RIGHTSHOULDER]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_RIGHTSTICK] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",rightstick:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_RIGHTSTICK]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",righttrigger:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_RIGHTTRIGGER]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_RIGHTTRIGGER] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",righttrigger:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_RIGHTTRIGGER]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",rightx:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]);
        }
        if (m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",righty:a";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_START] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",start:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_START]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_X] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",x:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_X]);
        }
        if (m_ControllerButton[STATE_CONF_BUTTON_Y] != STATE_CONF_SKIP_ITEM)
        {
            m_DatabaseEntry += ",y:b";
            m_DatabaseEntry += std::to_string(m_ControllerButton[STATE_CONF_BUTTON_Y]);
        }
        m_DatabaseEntry += ",platform:Linux,";

        m_MappingCreated = true;
        SetControllerConfText("Start controller setup (" + std::to_string(m_ControllerID + 1) + ")");
        LOG_CORE_INFO("Mapping created!");
    }
} // namespace GfxRenderEngine
