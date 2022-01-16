-- premake5.lua
workspace "vulkanRenderEngine"
    architecture "x86_64"
    startproject "vulkan"
    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

project "vulkan"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    defines
    {
        "ENGINE_VERSION=\"0.1.1\"",
    }

    files 
    { 
        "engine/**.h", 
        "engine/**.cpp",
        "application/**.h", 
        "application/**.cpp",
        "vendor/glfw/**.h", 
        "vendor/glfw/**.cpp",
        "vendor/stb/**.cpp",
    }

    includedirs 
    { 
        "engine",
        "engine/log",
        "engine/auxiliary",
        "engine/platform/",
        "engine/platform/Vulkan",
        "vendor/glfw/include",
        "vendor/stb",
        "vendor/glm",
        "vendor/spdlog/include",
        "application",
    }

    libdirs
    {
        "vendor/glfw/build/src",
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter "system:linux"

        linkoptions { "-fno-pie -no-pie" }

        prebuildcommands
        {
        }

        files 
        { 
        }
        includedirs 
        { 
        }
        links
        {
            "m",
            "dl", 
            "pthread",
            "glfw3",
        }
        libdirs
        {
        }
        defines
        {
            "LINUX",
        }

    filter "system:windows"
        defines
        {
        }
        files 
        {
        }
        links
        {
            "glfw3",
            "imagehlp", 
            "dinput8", 
            "dxguid", 
            "user32", 
            "gdi32", 
            "imm32", 
            "ole32",
            "oleaut32",
            "shell32",
            "version",
            "uuid",
            "Setupapi",
        }
        libdirs 
        {
        }

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"
        kind "ConsoleApp"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"
        kind "ConsoleApp"

    filter { "configurations:Dist" }
        defines {
            "NDEBUG",
            "DISTRIBUTION_BUILD"
        }
        optimize "On"
        kind "WindowedApp"

    -- activates profiling macros to output
    -- a json file for chrome://tracing
    filter { }
        defines { "PROFILING"}

    include "vendor/glfw.lua"
