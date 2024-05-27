
-- Team Engine 2021

project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("yaml-cpp/build")
    objdir ("yaml-cpp/build")

    files
    {
        "yaml-cpp/src/**.h",
        "yaml-cpp/src/**.cpp",
        "yaml-cpp/include/**.h"
    }

    includedirs
    {
        "yaml-cpp/include"
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
