-- premake5.lua
workspace "vulkanRenderEngine"
    architecture "x86_64"
    startproject "lucre"
    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

project "lucre"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    objdir ("bin-int/%{cfg.buildcfg}")

    defines
    {
        "LUCRE_VERSION=\"0.4.2\"",
        "PROFILING"
    }

    files 
    {
        "application/**.h", 
        "application/**.cpp",
        "vendor/tinygltf/tiny_gltf.cpp",
    }

    includedirs 
    {
        "./",
        "application",
        "application/lucre",
        "engine",
        "vendor",
        "vendor/imgui",
        "resources",
        "vendor/sdl/include",
        "vendor/spdlog/include",
        "vendor/yaml-cpp/include",
        "vendor/tinyObjLoader",
        "vendor/box2d/include",
        "vendor/entt/include",
        "vendor/json",
        "vendor/glm",
        "vendor/stb",
    }

    libdirs
    {
    }

    flags
    {
        "MultiProcessorCompile"
    }

    links
    {
        "engine",
        "glfw3",
        "sdl_mixer",
        "sdl",
        "libvorbis",
        "libogg",
        "yaml-cpp",
        "box2d",
        "shaderc",
        "shaderc_util",
        "SPIRV-Tools-opt",
        "SPIRV-Tools",
        "MachineIndependent",
        "OSDependent",
        "GenericCodeGen",
        "OGLCompiler",
        "SPIRV",
    }

    prebuildcommands
    {
    }

    filter "system:linux"

        linkoptions { "-fno-pie -no-pie" }

        files 
        { 
        }
        includedirs 
        {
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
            "m",
            "dl", 
            "vulkan",
            "pthread",
            "X11",
            "Xrandr",
            "Xi",
            "libpamanager",
            "pulse",
            "glib-2.0",
            "gio-2.0",
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
            "resources/windowsEmbeddedResources.rc",
        }
        includedirs 
        {
            "vendor/VulkanSDK/Include",
        }
        links
        {
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
            "vulkan-1",
        }
        libdirs 
        {
            "vendor/VulkanSDK/Lib",
        }

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"
        kind "ConsoleApp"

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

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

    if _ACTION == 'clean' then
        print("clean the build...")
        os.rmdir("./bin")
        os.rmdir("./bin-int")
        os.remove("./engine.cfg")
        os.remove("./imgui.ini")
        os.remove("./*.scene")
        os.remove("./resources/gnuEmbeddedResources.cpp")
        os.remove("./resources/gnuEmbeddedResources.h")
        os.remove("./**.make")
        os.remove("./Makefile")
        os.remove("./vendor/pamanager/libpamanager/Makefile")
        os.remove("./vendor/atlas/Makefile")
        os.remove("./vendor/box2d/Makefile")
        os.remove("./profiling (open with chrome tracing).json")
        os.rmdir("./vendor/atlas/bin/")
        os.remove("./vendor/atlas/MakefileMakefile")
        os.rmdir("./vendor/box2d/bin")
        os.rmdir("./vendor/box2d/bin-int")
        os.rmdir("./vendor/glfw/build")
        os.rmdir("./vendor/pamanager/libpamanager/bin")
        os.rmdir("./vendor/pamanager/libpamanager/obj")
        os.rmdir("./vendor/sdl/build")
        os.rmdir("./vendor/sdl_mixer/build")
        os.rmdir("./vendor/sfml/build")
        os.rmdir("./vendor/shaderc/bin")
        os.rmdir("./vendor/shaderc/bin-int")
        os.rmdir("./vendor/yaml-cpp/build")
        print("done.")
    end

    include "engine.lua"

