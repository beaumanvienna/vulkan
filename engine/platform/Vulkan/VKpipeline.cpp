/* Engine Copyright (c) 2021 Engine Development Team 
   https://github.com/beaumanvienna/gfxRenderEngine

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

#include <fstream>

#include "VKpipeline.h"

VK_Pipeline::VK_Pipeline(const std::string& filePathVertexShader_SPV, const std::string& filePathFragmentShader_SPV)
{
    CreateGraphicsPipeline(filePathVertexShader_SPV, filePathFragmentShader_SPV);
}

VK_Pipeline::~VK_Pipeline()
{
}

std::vector<char> VK_Pipeline::readFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        LOG_CORE_CRITICAL("failed to open file: {0}", filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    // populate the buffer
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;

}

void VK_Pipeline::CreateGraphicsPipeline(const std::string& filePathVertexShader_SPV, const std::string& filePathFragmentShader_SPV)
{
    auto vertCode = readFile(filePathVertexShader_SPV);
    auto fragCode = readFile(filePathFragmentShader_SPV);
    
    LOG_APP_INFO("Vertex Shader Code Size: {0}", vertCode.size());
    LOG_APP_INFO("Fragment Shader Code Size: {0}", fragCode.size());
}
