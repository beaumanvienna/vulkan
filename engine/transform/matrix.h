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

namespace Matrix
{
    constexpr float NINETY_DEGREES = 1.5707963f;
    constexpr float HUNDRET_EIGHTY_DEGREES = 3.1415926f;
} // namespace Matrix

inline glm::mat4 Scale(glm::vec3 scaleVec) { return glm::scale(glm::mat4(1.0f), scaleVec); }

inline glm::mat4 Translate(glm::vec3 translationVec) { return glm::translate(glm::mat4(1.0f), translationVec); }

inline glm::mat4 Rotate(float angle, glm::vec3 rotationVec) { return glm::rotate(glm::mat4(1.0f), angle, rotationVec); }
