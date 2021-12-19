#pragma once
#include "Tokens.h"
#include <vector>

namespace StormDB
{

#define STORM_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

	STORMDB_API enum class LexErrorType
	{
		UnknownToken,
	};

	inline std::string FormatLexErrorType(LexErrorType type)
	{
		switch (type)
		{
		case LexErrorType::UnknownToken:
			return "UnknownToken";
		}
		return "Unknown";
	}

	struct STORMDB_API LexError
	{
	public:
		LexErrorType Type;
		std::string Description;
	};

	struct STORMDB_API LexResult
	{
	public:
		std::vector<Token> Tokens;
		std::vector<LexError> Errors;
	};

	inline std::string FormatLexError(const LexError& error)
	{
		return FormatLexErrorType(error.Type) + " Error: " + error.Description;
	}

	constexpr bool IsNewline(char c)
	{
		return c == '\n';
	}

	constexpr bool IsWhitespace(char c)
	{
		return c == ' ' || c == '\t' || c == '\r' || IsNewline(c);
	}

	constexpr bool IsSymbol(char c)
	{
		return
			c == SYMBOL_ASTERISK[0] ||
			c == SYMBOL_COMMA[0] ||
			c == SYMBOL_LEFT_PAREN[0] ||
			c == SYMBOL_RIGHT_PAREN[0] ||
			c == SYMBOL_PLUS[0] ||
			c == SYMBOL_MINUS[0] ||
			c == SYMBOL_GT[0] ||
			c == SYMBOL_LT[0] ||
			c == SYMBOL_EQUALS[0] ||
			c == SYMBOL_MULTIPLY[0] ||
			c == SYMBOL_DIVIDE[0] ||
			c == SYMBOL_SEMICOLON[0];
	}

	constexpr bool IsWhitespaceOrSymbol(char c)
	{
		return IsWhitespace(c) || IsSymbol(c);
	}

	template<bool IncComments = false>
	LexResult LexSQL(const std::string& source);

}
