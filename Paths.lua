StormOutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Project Directories
StormDBLibDir = "StormDB-Lib/"
StormDBCliDir = "StormDB-Cli/"
StormDBTestDir = "StormDB-Test/"

-- Include directories relative to solution directory
StormIncludeDirs = {}
StormIncludeDirs["spdlog"] =     StormDBLibDir .. "vendor/spdlog/include/"
StormIncludeDirs["StormDB"] =    StormDBLibDir .. "src/"
StormIncludeDirs["Catch"] =      StormDBTestDir .. "vendor/Catch2/"

-- Library directories relative to solution directory
LibraryDirs = {}

-- Links
Links = {}
