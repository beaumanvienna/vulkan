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

#include "engine.h"
#include "coreSettings.h"
#include "gui/common.h"

namespace GfxRenderEngine
{

    std::string CoreSettings::m_EngineVersion;
    RendererAPI::API CoreSettings::m_RendererAPI;
    bool CoreSettings::m_EnableFullscreen;
    bool CoreSettings::m_EnableSystemSounds;
    std::string CoreSettings::m_BlacklistedDevice;
    int CoreSettings::m_UITheme;

    void CoreSettings::InitDefaults()
    {
        m_EngineVersion = ENGINE_VERSION;
        m_RendererAPI = RendererAPI::VULKAN;
        m_EnableFullscreen = false;
        m_EnableSystemSounds = true;
        m_BlacklistedDevice = "empty";
        m_UITheme = THEME_RETRO;
    }

    void CoreSettings::RegisterSettings()
    {
        m_SettingsManager->PushSetting<std::string>("EngineVersion", &m_EngineVersion);
        m_SettingsManager->PushSetting<RendererAPI::API>("RendererAPI", &m_RendererAPI);
        m_SettingsManager->PushSetting<bool>("EnableFullscreen", &m_EnableFullscreen);
        m_SettingsManager->PushSetting<bool>("EnableSystemSounds", &m_EnableSystemSounds);
        m_SettingsManager->PushSetting<std::string>("BlacklstedDevice", &m_BlacklistedDevice);
        m_SettingsManager->PushSetting<int>("UITheme", &m_UITheme);
    }

    void CoreSettings::PrintSettings() const
    {
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "EngineVersion", m_EngineVersion);
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "RendererAPI", m_RendererAPI);
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "EnableFullscreen", m_EnableFullscreen);
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "EnableSystemSounds", m_EnableSystemSounds);
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "BlacklistedDevice", m_BlacklistedDevice);
        LOG_CORE_INFO("CoreSettings: key '{0}', value is {1}", "UITheme", m_UITheme);
    }
} // namespace GfxRenderEngine
