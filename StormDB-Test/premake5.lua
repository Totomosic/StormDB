project "StormDB-Test"
    location ""
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("../bin/" .. StormOutputDir .. "/StormDB-Test")
    objdir ("../bin-int/" .. StormOutputDir .. "/StormDB-Test")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "vendor/Catch2/catch_amalgamated.hpp",
        "vendor/Catch2/catch_amalgamated.cpp"
    }

    includedirs
    {
        "src",
        "../%{StormIncludeDirs.spdlog}",
        "../%{StormIncludeDirs.StormDB}",
        "../%{StormIncludeDirs.Catch}",
    }

    links
    {
        "StormDB-Lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "STORMDB_PLATFORM_WINDOWS",
            "STORMDB_BUILD_STATIC",
            "_CRT_SECURE_NO_WARNINGS",
            "NOMINMAX"
        }

    filter "system:linux"
        systemversion "latest"

        removeconfigurations { "DistShared", "ReleaseShared" }

        defines
        {
            "STORMDB_PLATFORM_LINUX",
            "STORMDB_BUILD_STATIC"
        }

        links
        {
            "pthread"
        }

    filter "system:macosx"
        systemversion "latest"

        defines
        {
            "STORMDB_PLATFORM_MAC",
            "STORMDB_BUILD_STATIC"
        }

    filter "configurations:Debug"
        defines "STORMDB_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "STORMDB_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "STORMDB_DIST"
        runtime "Release"
        optimize "on"
