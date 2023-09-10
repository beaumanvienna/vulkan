
-- Team Engine 2021

project "glfw3"
    kind "StaticLib"
    language "C"

    targetdir ("glfw/build/src")
    objdir ("glfw/build/objectFiles")

    files
    {
        "glfw/include/GLFW/glfw3.h",
        "glfw/include/GLFW/glfw3native.h",
        "glfw/src/glfw_config.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/platform.c",
        "glfw/src/null_init.c",
        "glfw/src/null_window.c",
        "glfw/src/null_monitor.c",
        "glfw/src/null_joystick.c",
    }
    filter "system:linux"
        pic "On"

        systemversion "latest"

        files
        {
            "glfw/src/x11_init.c",
            "glfw/src/x11_monitor.c",
            "glfw/src/x11_window.c",
            "glfw/src/xkb_unicode.c",
            "glfw/src/posix_time.c",
            "glfw/src/posix_thread.c",
            "glfw/src/posix_module.c",
            "glfw/src/linux_joystick.c",
            "glfw/src/osmesa_context.c",
            "glfw/src/egl_context.c",
            "glfw/src/glx_context.c",
        }

        defines
        {
            "_GLFW_X11"
        }

    filter "system:windows"
        systemversion "latest"

        files
        {
            "glfw/src/win32_init.c",
            "glfw/src/win32_joystick.c",
            "glfw/src/win32_monitor.c",
            "glfw/src/win32_time.c",
            "glfw/src/win32_thread.c",
            "glfw/src/win32_window.c",
            "glfw/src/wgl_context.c",
            "glfw/src/egl_context.c",
            "glfw/src/osmesa_context.c"
        }

        defines 
        { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

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
