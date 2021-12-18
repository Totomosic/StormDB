workspace "StormDB"
    architecture "x64"

    configurations
    {
        "Dist",
        "Debug",
        "Release",
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter "system:linux"
        configurations
        {
            "DistShared",
            "ReleaseShared",
        }

include ("Paths.lua")

include (StormDBLibDir)
include (StormDBCliDir)
include (StormDBTestDir)
