#pragma once
#include "Logging.h"

namespace StormDB
{

	struct STORMDB_API SourceLocation
	{
	public:
		uint32_t Line = 0;
		uint32_t Column = 0;
	};

	STORMDB_API enum class TokenType
	{
		None,
		EndOfFile,

		Keyword,
		Symbol,
		Identifier,
		Comment,

		StringLiteral,
		NumericLiteral,
	};

	inline std::string FormatTokenType(TokenType type)
	{
		switch (type)
		{
		case TokenType::Keyword:
			return "KEYWORD";
		case TokenType::Symbol:
			return "SYMBOL";
		case TokenType::Identifier:
			return "IDENTIFIER";
		case TokenType::Comment:
			return "COMMENT";
		case TokenType::StringLiteral:
			return "STRING";
		case TokenType::NumericLiteral:
			return "NUMBER";
		case TokenType::EndOfFile:
			return "EOF";
		}
		return "Unknown";
	}

	struct STORMDB_API Token
	{
	public:
		TokenType Type = TokenType::None;
		SourceLocation Location;
		std::string Value;
	};

	inline bool operator==(const Token& left, const Token& right)
	{
		return left.Type == right.Type && left.Value == right.Value;
	}

	inline bool operator!=(const Token& left, const Token& right)
	{
		return !(left == right);
	}

	constexpr bool TokenValid(const Token& token)
	{
		return token.Type != TokenType::None;
	}

	inline std::string FormatSourceLocation(const SourceLocation& location)
	{
		return std::to_string(location.Line + 1) + ":" + std::to_string(location.Column + 1);
	}

	inline std::string FormatToken(const Token& token)
	{
		return "{ \"type\": " + FormatTokenType(token.Type) + ", \"location\": " + FormatSourceLocation(token.Location) + ", \"value\": \"" + token.Value + "\" }";
	}

	constexpr char* KEYWORD_SELECT = "select";
	constexpr char* KEYWORD_FROM = "from";
	constexpr char* KEYWORD_AS = "as";
	constexpr char* KEYWORD_TABLE = "table";
	constexpr char* KEYWORD_CREATE = "create";
	constexpr char* KEYWORD_INSERT = "insert";
	constexpr char* KEYWORD_DROP = "drop";
	constexpr char* KEYWORD_WHERE = "where";
	constexpr char* KEYWORD_INTO = "into";
	constexpr char* KEYWORD_VALUES = "values";
	constexpr char* KEYWORD_INT = "int";
	constexpr char* KEYWORD_TEXT = "text";
	constexpr char* KEYWORD_BOOL = "boolean";
	constexpr char* KEYWORD_TRUE = "true";
	constexpr char* KEYWORD_FALSE = "false";
	constexpr char* KEYWORD_NULL = "null";
	constexpr char* KEYWORD_AND = "and";
	constexpr char* KEYWORD_OR = "or";

	constexpr char* KEYWORDS[] = {
		KEYWORD_SELECT,
		KEYWORD_FROM,
		KEYWORD_AS,
		KEYWORD_TABLE,
		KEYWORD_CREATE,
		KEYWORD_INSERT,
		KEYWORD_DROP,
		KEYWORD_WHERE,
		KEYWORD_INTO,
		KEYWORD_VALUES,
		KEYWORD_INT,
		KEYWORD_TEXT,
		KEYWORD_BOOL,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
		KEYWORD_NULL,
		KEYWORD_AND,
		KEYWORD_OR,
	};
	// Maximum number of shared characters between any 2 KEYWORD values
	// Eg. if a matching word is > KEYWORD_SEPARATION it is guaranteed to be the maximum
	constexpr int KEYWORD_SEPARATION = 3;

	constexpr char* SYMBOL_SEMICOLON = ";";
	constexpr char* SYMBOL_ASTERISK = "*";
	constexpr char* SYMBOL_COMMA = ",";
	constexpr char* SYMBOL_LEFT_PAREN = "(";
	constexpr char* SYMBOL_RIGHT_PAREN = ")";
	constexpr char* SYMBOL_EQUALS = "=";
	constexpr char* SYMBOL_NOT_EQUALS = "<>";
	constexpr char* SYMBOL_NOT_EQUALS_2 = "!=";
	constexpr char* SYMBOL_GTE = ">=";
	constexpr char* SYMBOL_GT = ">";
	constexpr char* SYMBOL_LTE = "<=";
	constexpr char* SYMBOL_LT = "<";
	constexpr char* SYMBOL_PLUS = "+";
	constexpr char* SYMBOL_MINUS = "-";
	constexpr char* SYMBOL_MULTIPLY = "*";
	constexpr char* SYMBOL_DIVIDE = "/";

	constexpr char* SYMBOLS[] = {
		SYMBOL_SEMICOLON,
		SYMBOL_ASTERISK,
		SYMBOL_COMMA,
		SYMBOL_LEFT_PAREN,
		SYMBOL_RIGHT_PAREN,
		SYMBOL_EQUALS,
		SYMBOL_NOT_EQUALS,
		SYMBOL_NOT_EQUALS_2,
		SYMBOL_GTE,
		SYMBOL_LTE,
		SYMBOL_GT,
		SYMBOL_LT,
		SYMBOL_PLUS,
		SYMBOL_MINUS,
		SYMBOL_MULTIPLY,
		SYMBOL_DIVIDE,
	};

	constexpr int SYMBOL_SEPARATION = 1;

}
