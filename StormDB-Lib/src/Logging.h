#pragma once
#include <memory>
#include <string>
#include <functional>
#include <cstdint>
#include <iostream>

#define STORMDB_API

#if (!defined(EMSCRIPTEN) || !defined(STORMDB_DIST))
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

namespace StormDB
{

	class STORMDB_API Logger
	{
	private:
		static bool s_Initialized;
		static std::shared_ptr<spdlog::logger> s_Logger;

	public:
		static void Init();
		static inline spdlog::logger& GetLogger() { return *s_Logger; }
	};

}

#endif

#if defined(STORMDB_DIST) || defined(EMSCRIPTEN)
#define STORMDB_TRACE(...)
#define STORMDB_INFO(...)
#define STORMDB_WARN(...)
#define STORMDB_ERROR(...)
#define STORMDB_FATAL(...)

#define STORMDB_ASSERT(arg, ...)

#define STORMDB_DEBUG_ONLY(x)
#else

#define STORMDB_TRACE(...) ::StormDB::Logger::GetLogger().trace(__VA_ARGS__)
#define STORMDB_INFO(...) ::StormDB::Logger::GetLogger().info(__VA_ARGS__)
#define STORMDB_WARN(...) ::StormDB::Logger::GetLogger().warn(__VA_ARGS__)
#define STORMDB_ERROR(...) ::StormDB::Logger::GetLogger().error(__VA_ARGS__)
#define STORMDB_FATAL(...) ::StormDB::Logger::GetLogger().critical(__VA_ARGS__)

#ifdef STORMDB_PLATFORM_WINDOWS
#define STORMDB_ASSERT(arg, ...) { if (!(arg)) { STORMDB_FATAL(__VA_ARGS__); __debugbreak(); } }
#else
#define STORMDB_ASSERT(arg, ...) { if (!(arg)) { STORMDB_FATAL(__VA_ARGS__); } }
#endif

#define STORMDB_DEBUG_ONLY(x) x
#endif
