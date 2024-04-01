
-- Team Engine 2024

project "ufbx"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "ufbx/bin/%{cfg.buildcfg}"
    objdir ("ufbx/bin-int/%{cfg.buildcfg}")

    files
    {
        "ufbx/**.c",
        "ufbx/**.h"
    }

    includedirs
    {
        "ufbx"
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
