project "box2d"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    staticruntime "off"

    targetdir ("build/%{cfg.buildcfg}")
    objdir ("build/obj/%{cfg.buildcfg}")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "include/**.h"
    }

    includedirs
    {
        "include",
        "src"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        optimize "on"
