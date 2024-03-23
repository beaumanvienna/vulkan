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

#include <fstream>
#include <shaderc/shaderc.hpp>

#include "engine.h"
#include "auxiliary/file.h"

#include "VKshader.h"

namespace GfxRenderEngine
{

    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
    {
        shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type,
                                           const char* requestingSource, size_t includeDepth) override;
        void ReleaseInclude(shaderc_include_result* data) override;
        static std::string ReadFile(const std::string& filepath);
    };

    VK_Shader::VK_Shader(const std::string& sourceFilepath, const std::string& spirvFilepath, bool optimize)
        : m_SourceFilepath(sourceFilepath), m_SpirvFilepath(spirvFilepath), m_Optimize(optimize)
    {
        LOG_CORE_INFO("compiling {0}", sourceFilepath);
        ReadFile();
        Compile();
    }

    void VK_Shader::ReadFile()
    {
        std::string sourceCode;
        std::ifstream in(m_SourceFilepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size > 0)
            {
                sourceCode.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&sourceCode[0], size);
            }
            else
            {
                LOG_CORE_ERROR("VK_Shader: Could not read shader file '{0}'", m_SourceFilepath);
            }
        }
        else
        {
            LOG_CORE_ERROR("VK_Shader: Could not open shader file '{0}'", m_SourceFilepath);
        }

        m_SourceCode = sourceCode;
    }

    void VK_Shader::Compile()
    {

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        m_Ok = false;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2); // 1.3?
        options.SetIncluder(std::make_unique<ShaderIncluder>());

        if (m_Optimize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
        }

        shaderc_shader_kind shaderType;
        std::string extension = EngineCore::GetFileExtension(m_SourceFilepath);
        if (extension.find(".vert") != std::string::npos)
        {
            shaderType = shaderc_vertex_shader;
        }
        else if (extension.find(".frag") != std::string::npos)
        {
            shaderType = shaderc_fragment_shader;
        }
        else
        {
            LOG_CORE_ERROR("VK_Shader: Could not determine shader type from extension (allowed: .vert and .frag");
            return;
        }

        // precompile
        // shaderc::PreprocessedSourceCompilationResult precompileResult
        auto precompileResult = compiler.PreprocessGlsl(m_SourceCode, shaderType, m_SourceFilepath.c_str(), options);
        if (precompileResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            LOG_CORE_ERROR("VK_Shader: Could not preompile shader {0}, error message: {1}", m_SourceFilepath,
                           precompileResult.GetErrorMessage());
            return;
        }

        // compile
        // shaderc::SpvCompilationResult compileResult
        auto compileResult = compiler.CompileGlslToSpv(m_SourceCode, shaderType, m_SourceFilepath.c_str(), options);
        if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            LOG_CORE_ERROR("VK_Shader: Could not compile shader {0}, error message: {1}", m_SourceFilepath,
                           compileResult.GetErrorMessage());
            return;
        }

        std::ofstream outputFile(m_SpirvFilepath, std::ios::out | std::ios::binary);
        if (outputFile.is_open())
        {
            auto buffer = std::vector<uint>(compileResult.cbegin(), compileResult.cend());
            outputFile.write((char*)buffer.data(), buffer.size() * sizeof(uint));
            outputFile.flush();
            m_Ok = true;
        }
    }

    shaderc_include_result* ShaderIncluder::GetInclude(const char* requestedSource, shaderc_include_type type,
                                                       const char* requestingSource, size_t includeDepth)
    {
        std::string msg = std::string(requestingSource);
        msg += std::to_string(type);
        msg += static_cast<char>(includeDepth);

        const std::string name = std::string(requestedSource);
        const std::string contents = ReadFile(name);

        auto container = new std::array<std::string, 2>;
        (*container)[0] = name;
        (*container)[1] = contents;

        auto data = new shaderc_include_result;

        data->user_data = container;

        data->source_name = (*container)[0].data();
        data->source_name_length = (*container)[0].size();

        data->content = (*container)[1].data();
        data->content_length = (*container)[1].size();

        return data;
    }

    void ShaderIncluder::ReleaseInclude(shaderc_include_result* data)
    {
        delete static_cast<std::array<std::string, 2>*>(data->user_data);
        delete data;
    }

    std::string ShaderIncluder::ReadFile(const std::string& filepath)
    {
        std::string sourceCode;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size > 0)
            {
                sourceCode.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&sourceCode[0], size);
            }
            else
            {
                LOG_CORE_WARN("ShaderIncluder::ReadFile: Could not read shader file '{0}'", filepath);
            }
        }
        else
        {
            LOG_CORE_WARN("ShaderIncluder::ReadFile Could not open shader file '{0}'", filepath);
        }
        return sourceCode;
    }
} // namespace GfxRenderEngine
