/* Engine Copyright (c) 2025 Engine Development Team
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

#include "stb_image.h"
#include "tinyexr/tinyexr.h"

#include "auxiliary/file.h"
#include "renderer/hiResImage.h"

namespace GfxRenderEngine
{
    HiResImage::HiResImage()
        : m_Filename{}, m_Width{0}, m_Height{0}, m_Buffer{nullptr}, m_ImageType{ImageType::UNDEFINED}, m_Initialized{false}
    {
    }

    bool HiResImage::Init(std::string const& filename)
    {
        bool fileExists = EngineCore::FileExists(filename);
        CORE_ASSERT(fileExists, "IBLBuilder::HiResImage file not found " + filename);
        if (!fileExists)
        {
            return false;
        }

        auto extension = EngineCore::GetFileExtension(filename);
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return tolower(c); });

        if (extension == ".exr")
        {
            const char* err = nullptr;

            int ret = LoadEXR(&m_Buffer, &m_Width, &m_Height, filename.c_str(), &err);
            if ((ret != TINYEXR_SUCCESS) || (!m_Buffer))
            {
                if (err)
                {
                    std::string errorMessage("IBLBuilder::HiResImage: TinyEXR failed to load EXR image: ");
                    errorMessage += std::string(err);
                    LOG_APP_CRITICAL("{0}, filename '{1}'", errorMessage, filename);
                    FreeEXRErrorMessage(err);
                }
                return false;
            }
            m_ImageType = HiResImage::ImageType::EXR;
        }
        else if (extension == ".hdr")
        {
            int channels;
            const uint desiredNumberOfChannels = 4;

            // stbi_loadf loads HDR image as floats
            m_Buffer = stbi_loadf(filename.c_str(), &m_Width, &m_Height, &channels, desiredNumberOfChannels);
            if (!m_Buffer)
            {
                std::string errorMessage("IBLBuilder::HiResImage: STB failed to load HDR image: ");
                errorMessage += std::string(stbi_failure_reason());
                LOG_APP_CRITICAL("{0} {1}", errorMessage, filename);
                return false;
            }
            m_ImageType = HiResImage::ImageType::HDR;
        }
        else
        {
            LOG_APP_CRITICAL("unsupported extension '{0}' of filename {1}", extension, filename);
            return false;
        }
        // all good
        m_Filename = filename;
        m_Initialized = true;
        return true;
    }

    HiResImage::HiResImage::~HiResImage()
    {
        if (m_Buffer)
        {
            switch (m_ImageType)
            {
                case HiResImage::ImageType::EXR:
                {
                    free(m_Buffer);
                    break;
                }
                case HiResImage::ImageType::HDR:
                {
                    stbi_image_free(m_Buffer);
                    break;
                }
                default:
                    break;
            };
        }
    }

} // namespace GfxRenderEngine
