
-- Team Engine 2021

-- sdl_mixer
SDL_MIXER_DIR       = "sdl_mixer"
SDL_MIXER_INCLUDE   = SDL_MIXER_DIR.."/include"
SDL2_DIR            = "sdl"

project "sdl_mixer"
    kind "StaticLib"
    language "C"

    targetdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}"))
    objdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}/%{prj.name}"))
    
    defines
    {
        "MUSIC_OGG",
        "MUSIC_WAV",
    }

    includedirs
    {
        path.join(SDL_MIXER_DIR, "include"),
        path.join(SDL_MIXER_DIR, "src/"),
        path.join(SDL_MIXER_DIR, "src/codecs"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity"),
        path.join(SDL_MIXER_DIR, "src/codecs/native_midi"),
        path.join(SDL_MIXER_DIR, "external/flac-1.3.3/include"),
        path.join(SDL_MIXER_DIR, "external/libvorbis-1.3.6/include"),
        path.join(SDL_MIXER_DIR, "external/libogg-1.3.2/include"),
        path.join(SDL2_DIR, "include")
    }
    files
    {
        path.join(SDL_MIXER_DIR, "src/utils.c"),
        path.join(SDL_MIXER_DIR, "src/effects_internal.c"),
        path.join(SDL_MIXER_DIR, "src/effect_position.c"),
        path.join(SDL_MIXER_DIR, "src/effect_stereoreverse.c"),
        path.join(SDL_MIXER_DIR, "src/mixer.c"),
        path.join(SDL_MIXER_DIR, "src/music.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/load_aiff.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/load_voc.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/mp3utils.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_cmd.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_flac.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_fluidsynth.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_mad.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_mikmod.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_modplug.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_mpg123.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_nativemidi.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_ogg.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_opus.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_timidity.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/music_wav.c"),

        path.join(SDL_MIXER_DIR, "src/codecs/timidity/common.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/instrum.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/instrum.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/mix.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/output.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/playmidi.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/readmidi.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/resample.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/timidity.c"),
        path.join(SDL_MIXER_DIR, "src/codecs/timidity/tables.c"),

        path.join(SDL_MIXER_DIR, "src/codecs/native_midi/native_midi_common.c"),

    }

    filter "system:windows"
        files
        {
            path.join(SDL_MIXER_DIR, "version.rc"),
            path.join(SDL_MIXER_DIR, "src/codecs/native_midi/native_midi_win32.c"),
        }
        includedirs
        {
            path.join(SDL_MIXER_DIR, "VisualC/external/include/")
        }
        defines
        {
            "SDL_DISABLE_WINDOWS_IME",
            "WIN32",
            "_WINDOWS",
            "_CRT_SECURE_NO_WARNINGS",
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

-- lib vorbis
LIB_VORBIS_DIR       = "sdl_mixer/external/libvorbis-1.3.6"

project "libvorbis"
    kind "StaticLib"
    language "C"

    targetdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}"))
    objdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}/%{prj.name}"))

    includedirs
    {
        "sdl_mixer/external/libvorbis-1.3.6/include",
        "sdl_mixer/external/libogg-1.3.2/include",
        "sdl_mixer/external/libvorbis-1.3.6/lib/"
    }
    files
    {
        path.join(LIB_VORBIS_DIR, "lib/analysis.c"),
        path.join(LIB_VORBIS_DIR, "lib/bitrate.c"),
        path.join(LIB_VORBIS_DIR, "lib/block.c"),
        path.join(LIB_VORBIS_DIR, "lib/codebook.c"),
        path.join(LIB_VORBIS_DIR, "lib/envelope.c"),
        path.join(LIB_VORBIS_DIR, "lib/floor0.c"),
        path.join(LIB_VORBIS_DIR, "lib/floor1.c"),
        path.join(LIB_VORBIS_DIR, "lib/info.c"),
        path.join(LIB_VORBIS_DIR, "lib/lookup.c"),
        path.join(LIB_VORBIS_DIR, "lib/lpc.c"),
        path.join(LIB_VORBIS_DIR, "lib/lsp.c"),
        path.join(LIB_VORBIS_DIR, "lib/mapping0.c"),
        path.join(LIB_VORBIS_DIR, "lib/mdct.c"),
        path.join(LIB_VORBIS_DIR, "lib/res0.c"),
        path.join(LIB_VORBIS_DIR, "lib/sharedbook.c"),
        path.join(LIB_VORBIS_DIR, "lib/smallft.c"),
        path.join(LIB_VORBIS_DIR, "lib/synthesis.c"),
        path.join(LIB_VORBIS_DIR, "lib/vorbisenc.c"),
        path.join(LIB_VORBIS_DIR, "lib/window.c"),
        path.join(LIB_VORBIS_DIR, "lib/registry.c"),
        path.join(LIB_VORBIS_DIR, "lib/vorbisfile.c"),
        path.join(LIB_VORBIS_DIR, "lib/psy.c")
    }

    filter "system:windows"
        defines
        {
            "WIN32",
            "_WINDOWS",
            "_USRDLL",
            "LIBVORBIS_EXPORTS"
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

-- lib ogg
LIB_OGG_DIR       = "sdl_mixer/external/libogg-1.3.2"

project "libogg"
    kind "StaticLib"
    language "C"

    targetdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}"))
    objdir (path.join(SDL_MIXER_DIR, "build/%{cfg.buildcfg}/%{prj.name}"))

    includedirs
    {
        "sdl_mixer/external/libogg-1.3.2/include"
    }
    files
    {    
        path.join(LIB_OGG_DIR, "src/bitwise.c"),
        path.join(LIB_OGG_DIR, "src/framing.c")
    }

    filter "system:windows"
        defines
        {
            "WIN32",
            "_WINDOWS",
            "_USRDLL",
            "LIBOGG_EXPORTS"
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
