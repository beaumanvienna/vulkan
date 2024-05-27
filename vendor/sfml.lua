
-- Team Engine 2021
-- License: https://github.com/beaumanvienna/gfxRenderEngine/blob/master/LICENSE

project "sfml"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "sfml/build/lib"
    objdir    "sfml/build/obj"

    includedirs
    {
        "sfml/include",
        "sfml/src",
        "sfml/extlibs/headers/vulkan",
        "sfml/extlibs/headers/stb_image",
        "sfml/extlibs/headers/glad/include",
        "sfml/extlibs/headers/freetype2"
    }
            
    files
    { 
        "sfml/src/SFML/System/Win32/*.cpp",
        "sfml/src/SFML/System/*.cpp",
        "sfml/src/SFML/Graphics/*.cpp",
        "sfml/src/SFML/Window/*.cpp",
        "sfml/src/SFML/Window/Win32/*.cpp"
    }
    
    defines
    { 
        "WIN32",
        "_WINDOWS",
        "SFML_STATIC",
        "STBI_FAILURE_USERMSG",
        "_CRT_SECURE_NO_DEPRECATE",
        "_SCL_SECURE_NO_WARNINGS",
        "UNICODE",
        "_UNICODE"
    }


    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    configuration "Debug"
        defines { "DEBUG=1", "_DEBUG=1" }
        symbols "On"

    configuration "Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"

