#pragma once
#include "Logging.h"

namespace StormDB
{

	STORMDB_API enum class Datatype : uint8_t
	{
		Integer,
		Float,
		String,
		Boolean,
	};

	STORMDB_API enum class Operator : uint8_t
	{
		Equal,
		NotEqual,
		GreaterThan,
		GreaterEqual,
		LessThan,
		LessEqual,
		Add,
		Sub,
		Mul,
		Div,
		And,
		Or,
	};

	STORMDB_API enum class ColumnFlags : uint8_t
	{
		None = 0,
		AutoIncrement = 1 << 0,
	};

	struct STORMDB_API ColumnDefinition
	{
	public:
		std::string Name;
		Datatype Type;
		ColumnFlags Flags = ColumnFlags::None;
	};

	struct STORMDB_API TableDefinition
	{
	public:
		std::string Name;
		std::unordered_map<std::string, ColumnDefinition> Columns;
	};

	struct STORMDB_API DatabaseDefinition
	{
	public:
		std::string Name;
		std::unordered_map<std::string, TableDefinition> Tables;
	};

}
