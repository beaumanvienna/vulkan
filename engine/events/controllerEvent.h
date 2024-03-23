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
#include "platform/SDL/controller.h"

namespace GfxRenderEngine
{

    class ControllerAxisMovedEvent : public Event
    {

    public:
        ControllerAxisMovedEvent(int indexID, int axis, int value) : m_IndexID(indexID), m_Axis(axis), m_Value(value) {}

        inline int GetControllerIndexID() const { return m_IndexID; }
        inline int GetAxis() const { return m_Axis; }
        inline int GetAxisValue() const { return m_Value; }

        EVENT_CLASS_CATEGORY(EventCategoryController);
        EVENT_CLASS_TYPE(ControllerAxisMoved);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "ControllerAxisMovedEvent: m_IndexID: " << m_IndexID << ", m_Axis: " << m_Axis
                << ", m_Value: " << m_Value;
            return str.str();
        }

    private:
        int m_IndexID, m_Axis, m_Value;
    };

    class ControllerButtonEvent : public Event
    {

    public:
        inline int GetControllerIndexID() const { return m_IndexID; }
        inline int GetControllerButton() const { return m_ControllerButton; }

        EVENT_CLASS_CATEGORY(EventCategoryController | EventCategoryControllerButton);

    protected:
        ControllerButtonEvent(int indexID, int controllerButton) : m_IndexID(indexID), m_ControllerButton(controllerButton)
        {
        }

    private:
        int m_IndexID;
        int m_ControllerButton;
    };

    class ControllerButtonPressedEvent : public ControllerButtonEvent
    {

    public:
        ControllerButtonPressedEvent(int indexID, int controllerButton) : ControllerButtonEvent(indexID, controllerButton) {}

        EVENT_CLASS_TYPE(ControllerButtonPressed);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "ControllerButtonPressedEvent: m_IndexID: " << GetControllerIndexID()
                << ", m_ControllerButton: " << GetControllerButton();
            return str.str();
        }
    };

    class ControllerButtonReleasedEvent : public ControllerButtonEvent
    {

    public:
        ControllerButtonReleasedEvent(int indexID, int controllerButton) : ControllerButtonEvent(indexID, controllerButton)
        {
        }

        EVENT_CLASS_TYPE(ControllerButtonReleased);

        std::string ToString() const override
        {
            std::stringstream str;
            str << "ControllerButtonReleasedEvent: m_IndexID: " << GetControllerIndexID()
                << ", m_ControllerButton: " << GetControllerButton();
            return str.str();
        }
    };
} // namespace GfxRenderEngine
