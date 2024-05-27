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
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The code in this file is based on and inspired by the project
   https://github.com/TheCherno/Hazel. The license of this prject can
   be found under https://github.com/TheCherno/Hazel/blob/master/LICENSE
   */

#pragma once

namespace GfxRenderEngine
{
    enum KeyCode
    {
        ENGINE_KEY_SPACE = 32,
        ENGINE_KEY_APOSTROPHE = 39, /* ' */
        ENGINE_KEY_COMMA = 44,      /* , */
        ENGINE_KEY_MINUS = 45,      /* - */
        ENGINE_KEY_PERIOD = 46,     /* . */
        ENGINE_KEY_SLASH = 47,      /* / */
        ENGINE_KEY_0 = 48,
        ENGINE_KEY_1 = 49,
        ENGINE_KEY_2 = 50,
        ENGINE_KEY_3 = 51,
        ENGINE_KEY_4 = 52,
        ENGINE_KEY_5 = 53,
        ENGINE_KEY_6 = 54,
        ENGINE_KEY_7 = 55,
        ENGINE_KEY_8 = 56,
        ENGINE_KEY_9 = 57,
        ENGINE_KEY_SEMICOLON = 59, /* ; */
        ENGINE_KEY_EQUAL = 61,     /* = */
        ENGINE_KEY_A = 65,
        ENGINE_KEY_B = 66,
        ENGINE_KEY_C = 67,
        ENGINE_KEY_D = 68,
        ENGINE_KEY_E = 69,
        ENGINE_KEY_F = 70,
        ENGINE_KEY_G = 71,
        ENGINE_KEY_H = 72,
        ENGINE_KEY_I = 73,
        ENGINE_KEY_J = 74,
        ENGINE_KEY_K = 75,
        ENGINE_KEY_L = 76,
        ENGINE_KEY_M = 77,
        ENGINE_KEY_N = 78,
        ENGINE_KEY_O = 79,
        ENGINE_KEY_P = 80,
        ENGINE_KEY_Q = 81,
        ENGINE_KEY_R = 82,
        ENGINE_KEY_S = 83,
        ENGINE_KEY_T = 84,
        ENGINE_KEY_U = 85,
        ENGINE_KEY_V = 86,
        ENGINE_KEY_W = 87,
        ENGINE_KEY_X = 88,
        ENGINE_KEY_Y = 89,
        ENGINE_KEY_Z = 90,
        ENGINE_KEY_LEFT_BRACKET = 91,  /* [ */
        ENGINE_KEY_BACKSLASH = 92,     /* \ */
        ENGINE_KEY_RIGHT_BRACKET = 93, /* ] */
        ENGINE_KEY_GRAVE_ACCENT = 96,  /* ` */
        ENGINE_KEY_WORLD_1 = 161,      /* non-US #1 */
        ENGINE_KEY_WORLD_2 = 162,      /* non-US #2 */
        ENGINE_KEY_ESCAPE = 256,
        ENGINE_KEY_ENTER = 257,
        ENGINE_KEY_TAB = 258,
        ENGINE_KEY_BACKSPACE = 259,
        ENGINE_KEY_INSERT = 260,
        ENGINE_KEY_DELETE = 261,
        ENGINE_KEY_RIGHT = 262,
        ENGINE_KEY_LEFT = 263,
        ENGINE_KEY_DOWN = 264,
        ENGINE_KEY_UP = 265,
        ENGINE_KEY_PAGE_UP = 266,
        ENGINE_KEY_PAGE_DOWN = 267,
        ENGINE_KEY_HOME = 268,
        ENGINE_KEY_END = 269,
        ENGINE_KEY_CAPS_LOCK = 280,
        ENGINE_KEY_SCROLL_LOCK = 281,
        ENGINE_KEY_NUM_LOCK = 282,
        ENGINE_KEY_PRINT_SCREEN = 283,
        ENGINE_KEY_PAUSE = 284,
        ENGINE_KEY_F1 = 290,
        ENGINE_KEY_F2 = 291,
        ENGINE_KEY_F3 = 292,
        ENGINE_KEY_F4 = 293,
        ENGINE_KEY_F5 = 294,
        ENGINE_KEY_F6 = 295,
        ENGINE_KEY_F7 = 296,
        ENGINE_KEY_F8 = 297,
        ENGINE_KEY_F9 = 298,
        ENGINE_KEY_F10 = 299,
        ENGINE_KEY_F11 = 300,
        ENGINE_KEY_F12 = 301,
        ENGINE_KEY_F13 = 302,
        ENGINE_KEY_F14 = 303,
        ENGINE_KEY_F15 = 304,
        ENGINE_KEY_F16 = 305,
        ENGINE_KEY_F17 = 306,
        ENGINE_KEY_F18 = 307,
        ENGINE_KEY_F19 = 308,
        ENGINE_KEY_F20 = 309,
        ENGINE_KEY_F21 = 310,
        ENGINE_KEY_F22 = 311,
        ENGINE_KEY_F23 = 312,
        ENGINE_KEY_F24 = 313,
        ENGINE_KEY_F25 = 314,
        ENGINE_KEY_KP_0 = 320,
        ENGINE_KEY_KP_1 = 321,
        ENGINE_KEY_KP_2 = 322,
        ENGINE_KEY_KP_3 = 323,
        ENGINE_KEY_KP_4 = 324,
        ENGINE_KEY_KP_5 = 325,
        ENGINE_KEY_KP_6 = 326,
        ENGINE_KEY_KP_7 = 327,
        ENGINE_KEY_KP_8 = 328,
        ENGINE_KEY_KP_9 = 329,
        ENGINE_KEY_KP_DECIMAL = 330,
        ENGINE_KEY_KP_DIVIDE = 331,
        ENGINE_KEY_KP_MULTIPLY = 332,
        ENGINE_KEY_KP_SUBTRACT = 333,
        ENGINE_KEY_KP_ADD = 334,
        ENGINE_KEY_KP_ENTER = 335,
        ENGINE_KEY_KP_EQUAL = 336,
        ENGINE_KEY_LEFT_SHIFT = 340,
        ENGINE_KEY_LEFT_CONTROL = 341,
        ENGINE_KEY_LEFT_ALT = 342,
        ENGINE_KEY_LEFT_SUPER = 343,
        ENGINE_KEY_RIGHT_SHIFT = 344,
        ENGINE_KEY_RIGHT_CONTROL = 345,
        ENGINE_KEY_RIGHT_ALT = 346,
        ENGINE_KEY_RIGHT_SUPER = 347,
        ENGINE_KEY_MENU = 348
    };
} // namespace GfxRenderEngine
