project "StormDB-Lib"
    location ""
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("../bin/" .. StormOutputDir .. "/StormDB-Lib")
    objdir ("../bin-int/" .. StormOutputDir .. "/StormDB-Lib")

    files
    {
        "src/**.h",
        "src/**.cpp",
    }
    
    includedirs
    {
        "../%{StormIncludeDirs.spdlog}",
        "src",
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "STORMDB_PLATFORM_WINDOWS",
            "STORMDB_BUILD_STATIC",
            "_CRT_SECURE_NO_WARNINGS",
            "NOMINMAX",
        }

    filter "system:linux"
        systemversion "latest"

        defines
        {
            "STORMDB_PLATFORM_LINUX",
            "STORMDB_BUILD_STATIC",
        }

        links
        {
            "pthread",
        }

    filter "system:macosx"
        systemversion "latest"

        defines
        {
            "STORMDB_PLATFORM_MAC",
            "STORMDB_BUILD_STATIC",
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

    filter "configurations:ReleaseShared"
        defines "STORMDB_RELEASE"
        runtime "Release"
        optimize "on"

        buildoptions { "-fPIC" }

    filter "configurations:DistShared"
        defines "STORMDB_DIST"
        runtime "Release"
        optimize "on"

        buildoptions { "-fPIC" }
