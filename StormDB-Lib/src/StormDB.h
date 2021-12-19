#pragma once
#include "SQL/Lexer.h"
#include "SQL/Reconstruct.h"
#include "SQL/Parser.h"
#include "SQL/Compiler.h"

namespace StormDB
{

	inline void Init()
	{
		Logger::Init();
	}

}
