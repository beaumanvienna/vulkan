-- premake5.lua
workspace "vulkanRenderEngine"
    architecture "x86_64"
    startproject "engine"
    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

project "engine"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    defines
    {
        "ENGINE_VERSION=\"0.1.1\"",
        "PROFILING"
    }

    files 
    {
        "engine/**.h", 
        "engine/**.cpp",
        "application/**.h", 
        "application/**.cpp",
        "resources/resources.cpp",
        "vendor/glfw/**.h", 
        "vendor/glfw/**.cpp",
        "vendor/stb/**.cpp",
    }

    includedirs 
    {
        "engine",
        "engine/log",
        "engine/audio",
        "engine/auxiliary",
        "engine/renderer",
        "engine/events",
        "engine/scene",
        "engine/platform/",
        "engine/platform/SDL",
        "engine/platform/Vulkan",
        "application",
        "application/lucre",
        "resources",
        "vendor",
        "vendor/glfw/include",
        "vendor/stb",
        "vendor/glm",
        "vendor/spdlog/include",
        "vendor/sdl/include",
        "vendor/sdl_mixer/include",
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
            "resources/gnuEmbeddedResources.cpp"
        }
        includedirs 
        {
            "/usr/include",
            "vendor/pamanager/libpamanager/src",

            -- resource system: glib-2.0
            -- this should actually be `pkg-config glib-2.0 --cflags`
            "/usr/include/glib-2.0",
            "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
            "/usr/lib/glib-2.0/include/",
            "/usr/lib64/glib-2.0/include/",
            -- end resource system: glib-2.0
        }
        links
        {
            "sdl_mixer",
            "m",
            "dl", 
            "pthread",
            "glfw3",
            "vulkan",
            "X11",
            "Xrandr",
            "Xi",
            "libpamanager",
            "sdl",
            "pulse",
            "glib-2.0",
            "gio-2.0",
            "libvorbis",
            "libogg",
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

    filter { "action:gmake*"}
        prebuildcommands
        {
            "scripts/compileShaders.sh"
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

    include "vendor/glfw.lua"
    include "vendor/SPIRV-Cross.lua"
    include "vendor/shaderc.lua"
    include "vendor/sdl_mixer.lua"
    include "vendor/sdl.lua"

    if os.host() == "linux" then

        include "vendor/pamanager/libpamanager/libpamanager.lua"

    end

    if ( (os.host() == "linux") or (os.host() == "windows" and _ACTION == "gmake2") ) then

        project "resource-system-gnu"
            kind "StaticLib"
            os.execute("glib-compile-resources resources/gnuEmbeddedResources.xml --target=resources/gnuEmbeddedResources.cpp --sourcedir=resources/ --generate-source")
            os.execute("glib-compile-resources resources/gnuEmbeddedResources.xml --target=resources/gnuEmbeddedResources.h   --sourcedir=resources/ --generate-header")
    end
