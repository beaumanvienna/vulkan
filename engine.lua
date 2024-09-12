-- engine.lua
project "engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir ("bin-int/%{cfg.buildcfg}")

    defines
    {
        "ENGINE_VERSION=\"0.9.0\""
    }

    files 
    {
        "engine/**.h", 
        "engine/**.cpp",
        "resources/resources.cpp",
        "vendor/glfw/**.h", 
        "vendor/glfw/**.cpp",
        "vendor/stb/**.cpp",
        "vendor/imGuizmo/ImGuizmo.h",
        "vendor/imGuizmo/ImGuizmo.cpp",
        "vendor/imgui/backends/imgui_impl_glfw.cpp",
        "vendor/imgui/backends/imgui_impl_vulkan.cpp",
        "vendor/imgui/*.cpp",
        "vendor/tinygltf/tiny_gltf.cpp",
        "vendor/simdjson/simdjson.cpp",
        "vendor/simdjson/simdjson.h",
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
        "vendor/imgui",
        "vendor/imGuizmo",
        "vendor/spdlog/include",
        "vendor/shaderc/libshaderc/include/shaderc/",
        "vendor/shaderc/libshaderc/include/",
        "vendor/assetImporter/include",
        "vendor/fastgltf/include",
        "vendor/yaml-cpp/include",
        "vendor/tinyObjLoader",
        "vendor/box2d/include",
        "vendor/entt/include",
        "vendor/tinygltf",
        "vendor/simdjson",
        "vendor/json",
        "vendor/sdl/include",
        "vendor/sdl_mixer/include",
        "vendor/thread-pool/include",
        "vendor/tracy/include"
    }

    libdirs
    {
    }

    flags
    {
        "MultiProcessorCompile"
    }

    if USE_PULSEAUDIO then
        defines
        {
            "PULSEAUDIO"
        }
    end

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
            "/usr/include",
            "vendor/pamanager/libpamanager/src",
            "/usr/include/glib-2.0",
            "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
            "/usr/lib/glib-2.0/include/",
            "/usr/lib64/glib-2.0/include/"
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
            "vendor/glfw/src/win32_module.c",
        }
        includedirs 
        {
            "vendor/VulkanSDK/Include",
        }
        links
        {
        }
        libdirs 
        {
        }

    filter "system:macosx"

        linkoptions { "-fno-pie -no-pie" }

        prebuildcommands
        {
        }

        includedirs 
        {
            "/opt/homebrew/Cellar/glib/2.80.4/include/glib-2.0/",
            "/opt/homebrew/Cellar/glib/2.80.4/lib/glib-2.0/include/",
            "/opt/homebrew/include/SDL2/"
        }
        links
        {
        }
        libdirs
        {
        }
        defines
        {
            "MACOSX",
        }

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-reorder -Wno-expansion-to-defined" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-reorder -Wno-expansion-to-defined" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-reorder -Wno-expansion-to-defined" }

    filter { "configurations:Debug" }
        defines
        {
            "DEBUG",
            "PROFILING",
            "TRACY_ENABLE"
        }
        symbols "On"

    filter { "configurations:Release" }
        defines
        {
            "NDEBUG",
            "PROFILING",
            "TRACY_ENABLE"
        }
        optimize "On"

    filter { "configurations:Dist" }
        defines {
            "NDEBUG",
            "DISTRIBUTION_BUILD"
        }
        optimize "On"

    include "vendor/glfw.lua"
    include "vendor/yaml.lua"
    include "vendor/fastgltf.lua"
    include "vendor/ufbx.lua"
    include "vendor/atlas"
    include "vendor/shaderc.lua"
    if ((os.host() ~= "macosx")) then
        include "vendor/sdl_mixer.lua"
        include "vendor/sdl.lua"
    end
    include "vendor/box2d"
    include "vendor/assetImporter"

    if ( (os.host() == "linux") or (os.host() == "windows" and _ACTION == "gmake2")  or (os.host() == "macosx" and _ACTION == "gmake2")) then

        project "resource-system-gnu"
            kind "StaticLib"
            os.execute("glib-compile-resources resources/gnuEmbeddedResources.xml --target=resources/gnuEmbeddedResources.cpp --sourcedir=resources/ --generate-source")
            os.execute("glib-compile-resources resources/gnuEmbeddedResources.xml --target=resources/gnuEmbeddedResources.h   --sourcedir=resources/ --generate-header")

            files 
            { 
                "resources/gnuEmbeddedResources.cpp"
            }

            filter "system:linux"

                includedirs 
                {
                    "/usr/include/glib-2.0",
                    "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
                    "/usr/lib/glib-2.0/include/",
                    "/usr/lib64/glib-2.0/include/"
                }

            filter "system:macosx"

                includedirs 
                {
                    "/opt/homebrew/Cellar/glib/2.80.4/include/glib-2.0/",
                    "/opt/homebrew/Cellar/glib/2.80.4/lib/glib-2.0/include/",
                }
    end

    if os.host() == "windows" then
        include "vendor/sfml.lua"
    end
