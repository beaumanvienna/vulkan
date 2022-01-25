-- engine.lua
project "engine"
    kind "StaticLib"
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
        "resources/resources.cpp",
        "vendor/glfw/**.h", 
        "vendor/glfw/**.cpp",
        "vendor/stb/**.cpp",
    }

    includedirs 
    {
        "./",
        "engine",
        "engine/platform/Vulkan",
        "vendor",
        "vendor/glfw/include",
        "vendor/stb",
        "vendor/glm",
        "vendor/spdlog/include",
        "vendor/sdl/include",
        "vendor/sdl_mixer/include",
        "vendor/yaml-cpp/include",
        "vendor/tinyObjLoader",
    }

    libdirs
    {
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
        }
        libdirs 
        {
        }

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

    filter { "configurations:Dist" }
        defines {
            "NDEBUG",
            "DISTRIBUTION_BUILD"
        }
        optimize "On"

    include "vendor/glfw.lua"
    include "vendor/yaml.lua"
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
