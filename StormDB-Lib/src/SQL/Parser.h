#pragma once
#include "Lexer.h"
#include "Optional.h"
#include <memory>

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
		None,
		Literal,
		Binary,
	};

	struct BinaryExpression;

	struct STORMDB_API SQLExpression
	{
	public:
		ExpressionType Type = ExpressionType::None;
		Token Literal;
		std::shared_ptr<BinaryExpression> Binary;
	};

	struct STORMDB_API BinaryExpression
	{
	public:
		SQLExpression Left;
		SQLExpression Right;
		Token Operator;
	};

	struct STORMDB_API SelectStatement
	{
	public:
		Token Table;
		std::vector<SQLExpression> Columns;
		SQLExpression Where;
	};

	struct STORMDB_API InsertStatement
	{
	public:
		Token Table;
		std::vector<SQLExpression> Values;
	};

	struct STORMDB_API SQLColumnDefinition
	{
	public:
		Token Name;
		Token Datatype;
	};

	struct STORMDB_API CreateStatement
	{
	public:
		Token Name;
		std::vector<SQLColumnDefinition> Columns;
	};

	struct STORMDB_API SQLStatement
	{
	public:
		StatementType Type;
		SelectStatement Select;
		InsertStatement Insert;
		CreateStatement Create;
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

	Optional<SQLExpression> ParseExpression(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters);

	std::vector<Token> FilterComments(const std::vector<Token>& tokens);
	ParseResult ParseSQL(const std::vector<Token>& tokens);

}
