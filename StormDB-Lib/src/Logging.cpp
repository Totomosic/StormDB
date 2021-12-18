#if (!defined(EMSCRIPTEN) || !defined(STORMDB_DIST))
#include "Logging.h"

namespace StormDB
{

	bool Logger::s_Initialized = false;
	std::shared_ptr<spdlog::logger> Logger::s_Logger;

	void Logger::Init()
	{
		if (!s_Initialized)
		{
			spdlog::set_pattern("%^[%T] %n: %v%$");
			s_Logger = spdlog::stdout_color_mt("[STORM]");
			s_Logger->set_level(spdlog::level::level_enum::trace);
			s_Initialized = true;
		}
	}

}
#endif
