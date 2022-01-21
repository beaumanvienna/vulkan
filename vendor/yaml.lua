
-- Team Engine 2021

project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

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

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"
