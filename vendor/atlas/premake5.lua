
-- Team Engine 2021
-- License: https://github.com/beaumanvienna/gfxRenderEngine/blob/master/LICENSE

project "SpriteSheetGenerator"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin/objectFiles")

    defines
    {
        "GENERATOR_VERSION=\"0.0.1\"",
        "SFML_STATIC" 
    }

    files 
    { 
        "*.h", 
        "*.cpp"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter "system:linux"
        prebuildcommands
        {
            "../../scripts/build_sfml.sh"
        }
        includedirs 
        { 
            "../sfml/include"
        }
        libdirs 
        {
            "../sfml/build/lib"
        }
        links
        {
            "sfml-graphics-s",
            "sfml-window-s",
            "sfml-system-s",
            "X11",
            "dl",
            "udev",
            "pthread",
            "Xrandr",
            "Xcursor"
        }

    filter "system:windows"
        includedirs 
        { 
            "../sfml/include/"
        }
        libdirs 
        {
            "../sfml/build/lib"
        }
        links
        {
            "sfml",
            "opengl32",
            "winmm",
            "gdi32",
            "ws2_32"
        }

    filter "system:macosx"
        includedirs 
        { 
            "/opt/homebrew/Cellar/sfml/2.6.0/include"
        }
        libdirs 
        {
            "/opt/homebrew/Cellar/sfml/2.6.0/lib"
        }
        links
        {
            "sfml-graphics",
            "sfml-window",
            "sfml-system",
            "dl",
            "pthread",
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-g -Og" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"


