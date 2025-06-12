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

#include "engine.h"

namespace GfxRenderEngine
{

    class Debug
    {
    public:
        static void PrintMat4(const std::string& name, const glm::mat4& matrix)
        {
            std::cout << name << std::endl;
            for (int row = 0; row < 4; row++)
            {
                for (int column = 0; column < 4; column++)
                {
                    std::cout << matrix[column][row] << " ";
                }
                std::cout << std::endl;
            }
        }

        static void PrintVec2(const std::string& name, const glm::vec2& vector)
        {
            std::cout << name << " ";
            for (int row = 0; row < 2; row++)
            {
                std::cout << vector[row] << " ";
            }
            std::cout << std::endl;
        }

        static void PrintVec3(const std::string& name, const glm::vec3& vector)
        {
            std::cout << name << " ";
            for (int row = 0; row < 3; row++)
            {
                std::cout << vector[row] << " ";
            }
            std::cout << std::endl;
        }

        static void PrintVec4(const std::string& name, const glm::vec4& vector)
        {
            std::cout << name << " ";
            for (int row = 0; row < 4; row++)
            {
                std::cout << vector[row] << " ";
            }
            std::cout << std::endl;
        }

        static void PrintIVec4(const std::string& name, const glm::ivec4& vector)
        {
            std::cout << name << " ";
            for (int row = 0; row < 4; row++)
            {
                std::cout << vector[row] << " ";
            }
            std::cout << std::endl;
        }

        static void PrintQuaternion(const std::string& name, const glm::quat& quaternion)
        {
            std::cout << name << " ";
            std::cout << quaternion.w << " " << quaternion.x << " " << quaternion.y << " " << quaternion.z;
            std::cout << std::endl;
        }
    };
} // namespace GfxRenderEngine
