
-- This file is based on https://github.com/kolyden/premake-sdl2
-- License: https://github.com/kolyden/premake-sdl2/blob/master/LICENSE
-- Modified by Team Engine 2021

SDL2_DIR     = "sdl"
SDL2_INCLUDE = SDL2_DIR.."/include"

project "sdl"
    kind "StaticLib"
    language "C"
    
    targetdir (path.join(SDL2_DIR, "build/%{_ACTION}-%{cfg.platform}-%{cfg.buildcfg}"))
    objdir (path.join(SDL2_DIR, "build/%{_ACTION}-%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}"))
    
    defines "HAVE_LIBC"
    includedirs
    { 
        path.join(SDL2_DIR, "include"),
        path.join(SDL2_DIR, "src/video/khronos")
    }
    files
    {
        path.join(SDL2_DIR, "include/**"),
        path.join(SDL2_DIR, "src/*.c"),
        path.join(SDL2_DIR, "src/*.h"),
        path.join(SDL2_DIR, "src/cpuinfo/**"),
        path.join(SDL2_DIR, "src/dynapi/**"),
        path.join(SDL2_DIR, "src/atomic/**"),
        path.join(SDL2_DIR, "src/libm/**"),
        path.join(SDL2_DIR, "src/stdlib/**"),
        path.join(SDL2_DIR, "src/filesystem/dummy/**"),
        path.join(SDL2_DIR, "src/loadso/dummy/**"),
        -- AUDIO --
        path.join(SDL2_DIR, "src/audio/*.c"),
        path.join(SDL2_DIR, "src/audio/*.h"),
        path.join(SDL2_DIR, "src/audio/dummy/**"),
        -- EVENTS --
        path.join(SDL2_DIR, "src/events/*"),
        -- FILE --
        path.join(SDL2_DIR, "src/file/*.c"),
        -- HAPTIC --
        path.join(SDL2_DIR, "src/haptic/*.c"),
        path.join(SDL2_DIR, "src/haptic/*.h"),
        path.join(SDL2_DIR, "src/haptic/dummy/**"),
        -- joystick --
        path.join(SDL2_DIR, "src/joystick/*.c"),
        path.join(SDL2_DIR, "src/joystick/*.h"),
        path.join(SDL2_DIR, "src/joystick/dummy/**"),
        path.join(SDL2_DIR, "src/joystick/hidapi/*.c"),
        path.join(SDL2_DIR, "src/joystick/hidapi/*.h"),
        path.join(SDL2_DIR, "src/joystick/virtual/*"),
        -- LOCALE --
        path.join(SDL2_DIR, "src/locale/*.c"),
        path.join(SDL2_DIR, "src/locale/*.h"),
        path.join(SDL2_DIR, "src/locale/dummy/**"),
        -- POWER --
        path.join(SDL2_DIR, "src/power/*.c"),
        path.join(SDL2_DIR, "src/power/*.h"),
        -- RENDER --
        path.join(SDL2_DIR, "src/render/*.c"),
        path.join(SDL2_DIR, "src/render/*.h"),
        path.join(SDL2_DIR, "src/render/software/**"),
        path.join(SDL2_DIR, "src/render/opengl/**"),
        path.join(SDL2_DIR, "src/render/opengles2/**"),
        -- SENSOR --
        path.join(SDL2_DIR, "src/sensor/*.c"),
        path.join(SDL2_DIR, "src/sensor/*.h"),
        path.join(SDL2_DIR, "src/sensor/dummy/**"),
        -- THREAD --
        path.join(SDL2_DIR, "src/thread/*.c"),
        path.join(SDL2_DIR, "src/thread/*.h"),
        -- TIMER --
        path.join(SDL2_DIR, "src/timer/*.c"),
        path.join(SDL2_DIR, "src/timer/*.h"),
        path.join(SDL2_DIR, "src/timer/dummy/**"),
        -- VIDEO --
        path.join(SDL2_DIR, "src/video/*.c"),
        path.join(SDL2_DIR, "src/video/*.h"),
        path.join(SDL2_DIR, "src/video/dummy/**"),
        path.join(SDL2_DIR, "src/video/yuv2rgb/**"),
        -- MISC --
        path.join(SDL2_DIR, "src/misc/*.c"),
        path.join(SDL2_DIR, "src/misc/*.h"),
    }

    filter "system:windows"
        links { "setupapi", "winmm", "imm32", "version" }
        files
        {
            path.join(SDL2_DIR, "src/audio/directsound/**"),
            path.join(SDL2_DIR, "src/audio/disk/**"),
            path.join(SDL2_DIR, "src/audio/winmm/**"),
            path.join(SDL2_DIR, "src/audio/wasapi/**"),
            path.join(SDL2_DIR, "src/core/windows/**"),
            path.join(SDL2_DIR, "src/events/scancodes_windows.h"),
            path.join(SDL2_DIR, "src/filesystem/windows/**"),
            path.join(SDL2_DIR, "src/haptic/windows/**"),
            path.join(SDL2_DIR, "src/hidapi/windows/hid.c"),
            path.join(SDL2_DIR, "src/joystick/windows/**"),
            path.join(SDL2_DIR, "src/loadso/windows/**"),
            path.join(SDL2_DIR, "src/locale/windows/**"),
            path.join(SDL2_DIR, "src/power/windows/**"),
            path.join(SDL2_DIR, "src/render/direct3d/**"),
            path.join(SDL2_DIR, "src/render/direct3d11/**"),
            path.join(SDL2_DIR, "src/sensor/windows/**"),
            path.join(SDL2_DIR, "src/thread/generic/SDL_syscond.c"),
            path.join(SDL2_DIR, "src/thread/windows/**"),
            path.join(SDL2_DIR, "src/timer/windows/**"),
            path.join(SDL2_DIR, "src/video/windows/**"),
            path.join(SDL2_DIR, "src/misc/windows/**"),
        }
        removefiles
        {
            "**/SDL_render_winrt.*"
        }
        defines
        {
            "SDL_DISABLE_WINDOWS_IME",
            "WIN32",
            "__WIN32__",
        }
        links { "user32", "gdi32", "winmm", "imm32", "ole32", "oleaut32", "version", "uuid" }

    filter "system:linux"

        linkoptions { "-fno-pie -no-pie -mmmx -m3dnow -msse -msse2 -msse3 -Wall -fno-strict-aliasing -fvisibility=hidden -Wdeclaration-after-statement -Werror=declaration-after-statement -pthread" }

        files
        {
            path.join(SDL2_DIR, "src/misc/unix/SDL_sysurl.c"),
            path.join(SDL2_DIR, "src/core/unix/SDL_poll.c"),
            path.join(SDL2_DIR, "src/timer/unix/SDL_systimer.c"),
            path.join(SDL2_DIR, "src/loadso/dlopen/SDL_sysloadso.c"),
            path.join(SDL2_DIR, "src/audio/alsa/SDL_alsa_audio.c"),
            path.join(SDL2_DIR, "src/audio/pulseaudio/SDL_pulseaudio.c"),
            path.join(SDL2_DIR, "src/audio/sndio/SDL_sndioaudio.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_dbus.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_ime.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_ibus.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_fcitx.c"),
            path.join(SDL2_DIR, "src/hidapi/SDL_hidapi.c"),
            path.join(SDL2_DIR, "src/thread/pthread/SDL_systhread.c"),
            path.join(SDL2_DIR, "src/thread/pthread/SDL_syssem.c"),
            path.join(SDL2_DIR, "src/thread/pthread/SDL_sysmutex.c"),
            path.join(SDL2_DIR, "src/thread/pthread/SDL_syscond.c"),
            path.join(SDL2_DIR, "src/thread/pthread/SDL_systls.c"),
            path.join(SDL2_DIR, "src/joystick/steam/SDL_steamcontroller.c"),
            path.join(SDL2_DIR, "src/power/linux/SDL_syspower.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_udev.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_evdev.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_evdev_kbd.c"),
            path.join(SDL2_DIR, "src/core/freebsd/SDL_evdev_kbd_freebsd.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_evdev_capabilities.c"),
            path.join(SDL2_DIR, "src/core/linux/SDL_threadprio.c"),
            path.join(SDL2_DIR, "src/main/dummy/SDL_dummy_main.c"),
            path.join(SDL2_DIR, "src/haptic/linux/SDL_syshaptic.c"),
            path.join(SDL2_DIR, "src/joystick/linux/SDL_sysjoystick.c"),
            path.join(SDL2_DIR, "src/stdlib/SDL_strtokr.c"),
        }
        includedirs
        { 
            "/usr/include/dbus-1.0",
            "/usr/include/ibus-1.0",
            "/usr/include/glib-2.0",
            "/usr/lib/glib-2.0/include",
            "/usr/lib/dbus-1.0/include",
            "/usr/lib/x86_64-linux-gnu/dbus-1.0/include",
            "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
            "/usr/lib64/dbus-1.0/include",
            "/usr/lib64/glib-2.0/include/",
            path.join(SDL2_DIR, "src/hidapi/hidapi"),
        }  

        defines
        {
            "_REENTRANT",
            "MESA_EGL_NO_X11_HEADERS",
            "EGL_NO_X11",
            "HAVE_LINUX_VERSION_H",
        }

    filter "action:vs*"
        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "VC_EXTRALEAN",
        }

    filter { "action:gmake*", "configurations:Debug"}
        buildoptions { "-ggdb -Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Release"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter { "action:gmake*", "configurations:Dist"}
        buildoptions { "-Wall -Wextra -Wpedantic -Wshadow" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }
        optimize "On"


