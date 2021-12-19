#pragma once
#include "Parser.h"
#include "DB/DatabaseMetadata.h"

namespace StormDB
{

	struct STORMDB_API DatabaseContext
	{
	public:
		DatabaseDefinition Definition;
	};

}
