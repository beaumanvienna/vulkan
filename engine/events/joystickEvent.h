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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "engine.h"
#include "events/event.h"

namespace GfxRenderEngine
{

    class JoystickAxisMovedEvent : public Event
    {

    public:
        JoystickAxisMovedEvent(int indexID, int axis, int value) : m_IndexID(indexID), m_Axis(axis), m_Value(value) {}

        inline int GetJoystickIndexID() const { return m_IndexID; }
        inline int GetAxis() const { return m_Axis; }
        inline int GetAxisValue() const { return m_Value; }

        EVENT_CLASS_CATEGORY(EventCategoryJoystick);
        EVENT_CLASS_TYPE(JoystickAxisMoved);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "JoystickAxisMovedEvent: " << m_IndexID << ", m_Axis: " << m_Axis << ", m_Value: " << m_Value;
            return str.str();
        }

    private:
        int m_IndexID, m_Axis, m_Value;
    };

    class JoystickHatMovedEvent : public Event
    {

    public:
        JoystickHatMovedEvent(int indexID, int hat, int value) : m_IndexID(indexID), m_Hat(hat), m_Value(value) {}

        inline int GetJoystickIndexID() const { return m_IndexID; }
        inline int GetHat() const { return m_Hat; }
        inline int GetHatValue() const { return m_Value; }

        EVENT_CLASS_CATEGORY(EventCategoryJoystick);
        EVENT_CLASS_TYPE(JoystickHatMoved);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "JoystickHatMovedEvent: " << m_IndexID << ", m_Hat: " << m_Hat << ", m_Value: " << m_Value;
            return str.str();
        }

    private:
        int m_IndexID, m_Hat, m_Value;
    };

    class JoystickBallMovedEvent : public Event
    {

    public:
        JoystickBallMovedEvent(int indexID, int hat, int xrel, int yrel)
            : m_IndexID(indexID), m_Ball(hat), m_RelativeX(xrel), m_RelativeY(yrel)
        {
        }

        inline int GetJoystickIndexID() const { return m_IndexID; }
        inline int GetBall() const { return m_Ball; }
        inline int GetRelativeX() const { return m_RelativeX; }
        inline int GetRelativeY() const { return m_RelativeY; }

        EVENT_CLASS_CATEGORY(EventCategoryJoystick);
        EVENT_CLASS_TYPE(JoystickBallMoved);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "JoystickBallMovedEvent: " << m_IndexID << ", m_Ball: " << m_Ball << ", m_RelativeX: " << m_RelativeX
                << ", m_RelativeY: " << m_RelativeY;
            return str.str();
        }

    private:
        int m_IndexID, m_Ball, m_RelativeX, m_RelativeY;
    };

    class JoystickButtonEvent : public Event
    {

    public:
        inline int GetJoystickIndexID() const { return m_IndexID; }
        inline int GetJoystickButton() const { return m_JoystickButton; }

        EVENT_CLASS_CATEGORY(EventCategoryJoystick | EventCategoryJoystickButton);

    protected:
        JoystickButtonEvent(int indexID, int joystickButton) : m_IndexID(indexID), m_JoystickButton(joystickButton) {}

    private:
        int m_IndexID;
        int m_JoystickButton;
    };

    class JoystickButtonPressedEvent : public JoystickButtonEvent
    {

    public:
        JoystickButtonPressedEvent(int indexID, int joystickButton) : JoystickButtonEvent(indexID, joystickButton) {}

        EVENT_CLASS_TYPE(JoystickButtonPressed);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "JoystickButtonPressedEvent: " << GetJoystickIndexID() << ", m_JoystickButton: " << GetJoystickButton();
            return str.str();
        }
    };

    class JoystickButtonReleasedEvent : public JoystickButtonEvent
    {

    public:
        JoystickButtonReleasedEvent(int indexID, int joystickButton) : JoystickButtonEvent(indexID, joystickButton) {}

        EVENT_CLASS_TYPE(JoystickButtonReleased);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "JoystickButtonReleasedEvent: " << GetJoystickIndexID() << ", m_JoystickButton: " << GetJoystickButton();
            return str.str();
        }
    };
} // namespace GfxRenderEngine
