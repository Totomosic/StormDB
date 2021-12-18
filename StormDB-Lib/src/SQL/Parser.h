#pragma once
#include "Lexer.h"

namespace StormDB
{

	STORMDB_API enum class StatementType
	{
		Select,
		Insert,
		Update,
		Create,
	};

	STORMDB_API enum class ExpressionType
	{
		Literal,
	};

	struct STORMDB_API SQLExpression
	{
	public:
		ExpressionType Type;
		Token Literal;
	};

	struct STORMDB_API SelectStatement
	{
	public:
		Token Table;
		std::vector<SQLExpression> Columns;
	};

	struct STORMDB_API InsertStatement
	{
	public:
		Token Table;
		std::vector<SQLExpression> Values;
	};

	struct STORMDB_API ColumnDefinition
	{
	public:
		Token Name;
		Token Datatype;
	};

	struct STORMDB_API CreateStatement
	{
	public:
		Token Name;
		std::vector<ColumnDefinition> Columns;
	};

	template<typename T>
	struct STORMDB_API StatementResult
	{
	public:
		T Statement;
		std::string Error = "";
	};

	struct STORMDB_API SQLStatement
	{
	public:
		StatementType Type;
		SelectStatement Select;
		InsertStatement Insert;
		CreateStatement Create;
		std::string Error = "";
	};

	struct STORMDB_API ParseResult
	{
	public:
		std::vector<SQLStatement> Statements;
		std::string Error = "";
	};

	inline Token TokenFromSymbol(const char* symbol)
	{
		Token token;
		token.Type = TokenType::Symbol;
		token.Value = symbol;
		return token;
	}

	inline Token TokenFromKeyword(const char* keyword)
	{
		Token token;
		token.Type = TokenType::Keyword;
		token.Value = keyword;
		return token;
	}

	ParseResult ParseSQL(const std::vector<Token>& tokens);

}
