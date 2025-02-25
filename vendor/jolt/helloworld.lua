workspace "JoltPhysics"
    architecture "x86_64"
    configurations 
    {
        "Debug",
        "Release"
    }

project "helloWorld"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    buildoptions { "-fdiagnostics-color=always -Wall -Wextra -Wno-unused-parameter" }

    defines
    {
        "JPH_PROFILE_ENABLED",
        "JPH_DEBUG_RENDERER",
        "JPH_OBJECT_STREAM",

        "JPH_USE_AVX",
        "JPH_USE_AVX2",
        "JPH_USE_F16C",
        "JPH_USE_FMADD",
        "JPH_USE_LZCNT",
        "JPH_USE_SSE4_1",
        "JPH_USE_SSE4_2",
        "JPH_USE_TZCNT"
    }

    files 
    { 
        "src/**.h", 
        "src/**.cpp",
    }

    includedirs 
    { 
        "./",
    }
    libdirs
    {
        "vendor/JoltPhysics/Build/Linux_Debug"
    }
    links
    {
        "Jolt",
        "pthread"
    }

    filter { "configurations:Debug" }
        defines { "DEBUG", "VERBOSE" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

    if _ACTION == 'clean' then
        print("clean the build...")
        os.rmdir("./bin")
        os.rmdir("./obj")
        os.remove("./**.make")
        os.remove("./Makefile")
        print("done.")
    end

