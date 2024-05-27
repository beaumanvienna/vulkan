
-- Team Engine 2024

project "fastgltf"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "fastgltf/bin/%{cfg.buildcfg}"
    objdir ("fastgltf/bin-int/%{cfg.buildcfg}")

    files
    {
        "fastgltf/src/**.cpp",
        "fastgltf/include/**.h"
    }

    includedirs
    {
        "glm",
        "spdlog/include/",
        "fastgltf/include",
        "simdjson",
        "../engine",
    }

    filter "system:linux"
        pic "On"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"
