/* Copyright (c) 2013-2020 PPSSPP project
   https://github.com/hrydgard/ppsspp/blob/master/LICENSE.TXT

   Engine Copyright (c) 2021-2022 Engine Development Team
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

#include <vector>
namespace GfxRenderEngine
{
    enum
    {
        DEVICE_ID_ANY = -1,
        DEVICE_ID_DEFAULT = 0,
        DEVICE_ID_KEYBOARD = 1,
        DEVICE_ID_MOUSE = 2,
        DEVICE_ID_PAD_0 = 10,
        DEVICE_ID_PAD_1 = 11,
        DEVICE_ID_PAD_2 = 12,
        DEVICE_ID_PAD_3 = 13,
        DEVICE_ID_PAD_4 = 14,
        DEVICE_ID_PAD_5 = 15,
        DEVICE_ID_PAD_6 = 16,
        DEVICE_ID_PAD_7 = 17,
        DEVICE_ID_PAD_8 = 18,
        DEVICE_ID_PAD_9 = 19,
        DEVICE_ID_X360_0 = 20,
        DEVICE_ID_X360_1 = 21,
        DEVICE_ID_X360_2 = 22,
        DEVICE_ID_X360_3 = 23,
        DEVICE_ID_ACCELEROMETER = 30,
    };

    const int MAX_NUM_PADS = 10;

    const char* SCREEN_GetDeviceName(int deviceId);

    enum
    {
        PAD_BUTTON_A = 1,
        PAD_BUTTON_B = 2,
        PAD_BUTTON_X = 4,
        PAD_BUTTON_Y = 8,
        PAD_BUTTON_LBUMPER = 16,
        PAD_BUTTON_RBUMPER = 32,
        PAD_BUTTON_START = 64,
        PAD_BUTTON_SELECT = 128,
        PAD_BUTTON_UP = 256,
        PAD_BUTTON_DOWN = 512,
        PAD_BUTTON_LEFT = 1024,
        PAD_BUTTON_RIGHT = 2048,

        PAD_BUTTON_MENU = 4096,
        PAD_BUTTON_BACK = 8192,
    };

    class SCREEN_KeyDef
    {
    public:
        SCREEN_KeyDef() : deviceId(0), keyCode(0) {}
        SCREEN_KeyDef(int devId, int k) : deviceId(devId), keyCode(k) {}
        int deviceId;
        int keyCode;

        bool operator<(const SCREEN_KeyDef& other) const
        {
            if (deviceId < other.deviceId)
                return true;
            if (deviceId > other.deviceId)
                return false;
            if (keyCode < other.keyCode)
                return true;
            return false;
        }
        bool operator==(const SCREEN_KeyDef& other) const
        {
            if (deviceId != other.deviceId && deviceId != DEVICE_ID_ANY && other.deviceId != DEVICE_ID_ANY)
                return false;
            if (keyCode != other.keyCode)
                return false;
            return true;
        }
    };

    enum
    {
        TOUCH_MOVE = 1 << 0,
        TOUCH_DOWN = 1 << 1,
        TOUCH_UP = 1 << 2,
        TOUCH_CANCEL = 1 << 3,
        TOUCH_WHEEL = 1 << 4,
        TOUCH_MOUSE = 1 << 5,
        TOUCH_RELEASE_ALL = 1 << 6,

        TOUCH_TOOL_MASK = 7 << 10,
        TOUCH_TOOL_UNKNOWN = 0 << 10,
        TOUCH_TOOL_FINGER = 1 << 10,
        TOUCH_TOOL_STYLUS = 2 << 10,
        TOUCH_TOOL_MOUSE = 3 << 10,
        TOUCH_TOOL_ERASER = 4 << 10,
    };

    struct SCREEN_TouchInput
    {
        float x;
        float y;
        int id;
        int flags;
        double timestamp;
    };

#undef KEY_DOWN
#undef KEY_UP

    enum
    {
        KEY_DOWN = 1 << 0,
        KEY_UP = 1 << 1,
        KEY_HASWHEELDELTA = 1 << 2,
        KEY_IS_REPEAT = 1 << 3,
        KEY_CHAR = 1 << 4,
    };

    struct SCREEN_KeyInput
    {
        SCREEN_KeyInput() {}
        SCREEN_KeyInput(int devId, int code, int fl) : deviceId(devId), keyCode(code), flags(fl) {}
        int deviceId;
        int keyCode;
        int flags;
    };

    struct SCREEN_AxisInput
    {
        int deviceId;
        int axisId;
        float value;
        int flags;
    };
} // namespace GfxRenderEngine
