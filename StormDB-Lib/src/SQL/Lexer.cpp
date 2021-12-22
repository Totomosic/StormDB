#include "Lexer.h"
#include <algorithm>

namespace StormDB
{

	struct Cursor
	{
	public:
		uint32_t Position = 0;
		SourceLocation Location;
	};

	struct LexerResult
	{
	public:
		bool Found = false;
		StormDB::Token Token;
	};

	using Lexer = LexerResult(*)(const std::string&, Cursor&);

	// Increment cursor position based on whether the whitespace is a newline, etc.
	void IncrementWhitespace(char whitespace, Cursor& cursor)
	{
		cursor.Position++;
		if (whitespace == ' ' || whitespace == '\t')
			cursor.Location.Column++;
		else if (whitespace == '\n')
			cursor.Location.Line++;
	}

	std::string::const_iterator FindNextWhitespaceOrSymbol(const std::string& source, const Cursor& cursor)
	{
		return std::find_if(source.begin() + cursor.Position + 1, source.end(), IsWhitespaceOrSymbol);
	}

	const char* LongestSubstring(const std::string& source, const Cursor& cursor, const char* const* options, size_t optionCount, int separation, bool requiresWhitespace)
	{
		const char* maxMatch = nullptr;
		int maxSize = 0;

		for (int i = 0; i < optionCount; i++)
		{
			const char* option = options[i];
			size_t optionLength = strlen(option);
			if (optionLength > maxSize && cursor.Position + optionLength <= source.size())
			{
				bool found = true;
				for (size_t j = 0; j < optionLength; j++)
				{
					if (std::tolower(source[cursor.Position + j]) != option[j])
					{
						found = false;
						break;
					}
				}
				if (found && (!requiresWhitespace || cursor.Position + optionLength == source.size() || IsWhitespaceOrSymbol(source[cursor.Position + optionLength])))
				{
					maxMatch = option;
					maxSize = optionLength;
					if (maxSize > separation)
						break;
				}
			}
		}

		return maxMatch;
	}

	void IncrementCursor(Cursor& cursor, int amount)
	{
		cursor.Position += amount;
		cursor.Location.Column += amount;
	}

	// TODO: Better validation
	bool ValidateIdentifier(const std::string& identifier)
	{
		if (identifier.empty())
			return false;
		if (!std::isalpha(identifier[0]))
			return false;
		return true;
	}

	LexerResult LexKeyword(const std::string& source, Cursor& cursor)
	{
		LexerResult result;
		const char* keyword = LongestSubstring(source, cursor, KEYWORDS, STORM_ARRAY_LENGTH(KEYWORDS), KEYWORD_SEPARATION, true);
		if (keyword != nullptr)
		{
			result.Found = true;
			result.Token.Location = cursor.Location;
			result.Token.Type = TokenType::Keyword;
			result.Token.Value = keyword;
			IncrementCursor(cursor, result.Token.Value.size());
		}
		return result;
	}

	LexerResult LexSymbol(const std::string& source, Cursor& cursor)
	{
		LexerResult result;
		const char* symbol = LongestSubstring(source, cursor, SYMBOLS, STORM_ARRAY_LENGTH(SYMBOLS), SYMBOL_SEPARATION, false);
		if (symbol != nullptr)
		{
			result.Found = true;
			result.Token.Location = cursor.Location;
			result.Token.Type = TokenType::Symbol;
			result.Token.Value = symbol;
			IncrementCursor(cursor, result.Token.Value.size());
		}
		return result;
	}

	LexerResult LexIdentifier(const std::string& source, Cursor& cursor)
	{
		// TODO: Support "" and [] quoted identifiers (can use keywords and spaces)
		LexerResult result;
		auto nextWhitespace = FindNextWhitespaceOrSymbol(source, cursor);
		size_t tokenLength = nextWhitespace - (source.begin() + cursor.Position);
		std::string identifier = source.substr(cursor.Position, tokenLength);
		// Check identifier uses correct characters
		if (ValidateIdentifier(identifier))
		{
			result.Found = true;
			result.Token.Location = cursor.Location;
			result.Token.Type = TokenType::Identifier;
			result.Token.Value = identifier;
			std::transform(result.Token.Value.begin(), result.Token.Value.end(), result.Token.Value.begin(), [](char c) { return std::tolower(c); });
			IncrementCursor(cursor, tokenLength);
		}
		return result;
	}

	LexerResult LexComment(const std::string& source, Cursor& cursor)
	{
		LexerResult result;
		if (cursor.Position < source.size() - 1)
		{
			// Comments start with --
			if (source[cursor.Position] == '-' && source[cursor.Position + 1] == '-')
			{
				auto newLine = std::find_if(source.begin() + cursor.Position + 2, source.end(), IsNewline);
				result.Found = true;
				result.Token.Location = cursor.Location;
				result.Token.Type = TokenType::Comment;
				result.Token.Value = newLine != source.end() ? source.substr(cursor.Position + 2, newLine - (source.begin() + cursor.Position + 2)) : source.substr(cursor.Position + 2);
				IncrementCursor(cursor, result.Token.Value.size() + 2);
			}
		}
		return result;
	}

	// Lex a string literal (eg. 'test')
	LexerResult LexStringLiteral(const std::string& source, Cursor& cursor)
	{
		LexerResult result;
		// Expect a single quote
		if (source[cursor.Position] == '\'')
		{
			std::string value = "";
			for (size_t i = 1; cursor.Position + i < source.size(); i++)
			{
				char c = source[cursor.Position + i];
				if (c != '\'')
					value += c;
				else
				{
					// Single quotes can be escaped using '' (2 consecutive single quotes)
					if (i < source.size() - 1 && source[cursor.Position + i + 1] == '\'')
					{
						value += '\'';
						i++;
					}
					else
					{
						result.Found = true;
						result.Token.Location = cursor.Location;
						result.Token.Type = TokenType::StringLiteral;
						result.Token.Value = value;
						IncrementCursor(cursor, i + 1);
						break;
					}
				}
			}
		}
		return result;
	}

	// Lex a numeric literal (eg. -5.45)
	LexerResult LexNumericLiteral(const std::string& source, Cursor& cursor)
	{
		LexerResult result;
		if (source[cursor.Position] == '-' || std::isdigit(source[cursor.Position]))
		{
			auto nextWhitespace = FindNextWhitespaceOrSymbol(source, cursor);
			size_t count = nextWhitespace - (source.begin() + cursor.Position);
			bool hasPoint = false;
			bool valid = false;
			for (size_t i = 0; i < count; i++)
			{
				if (source[cursor.Position + i] == '-' && i == 0)
				{
					continue;
				}
				if (std::isdigit(source[cursor.Position + i]))
				{
					valid = true;
					continue;
				}
				if (source[cursor.Position + i] == '.' && !hasPoint)
				{
					hasPoint = true;
					// Requires digits after the '.'
					valid = false;
					continue;
				}
				valid = false;
				break;
			}
			if (valid)
			{
				result.Found = true;
				result.Token.Location = cursor.Location;
				result.Token.Type = TokenType::NumericLiteral;
				result.Token.Value = source.substr(cursor.Position, count);
				IncrementCursor(cursor, count);
			}
		}
		return result;
	}

	// Utility method to provide additional context for error messages
	std::string GuessNextToken(const std::string& source, const Cursor& cursor)
	{
		auto nextWhitespace = FindNextWhitespaceOrSymbol(source, cursor);
		if (nextWhitespace == source.end())
			return source.substr(cursor.Position);
		return source.substr(cursor.Position, nextWhitespace - (source.begin() + cursor.Position));
	}

	template<bool IncComments>
	LexResult LexSQL(const std::string& source)
	{
		LexResult result;
		Cursor cursor;

		// Order of lexing is important
		constexpr Lexer lexers[] = { LexKeyword, LexComment, LexNumericLiteral, LexStringLiteral, LexSymbol, LexIdentifier };

		while (cursor.Position < source.size())
		{
			// Skip whitespace
			if (IsWhitespace(source[cursor.Position]))
			{
				IncrementWhitespace(source[cursor.Position], cursor);
				continue;
			}
			bool found = false;
			// Try to find a lexer method for identifying a token at the current cursor location
			for (const Lexer& lexer : lexers)
			{
				LexerResult token = lexer(source, cursor);
				if (token.Found)
				{
					// Potentially ignore comment tokens
					if (IncComments || token.Token.Type != TokenType::Comment)
						result.Tokens.push_back(token.Token);
					found = true;
					break;
				}
			}
			if (!found)
			{
				// Unknown token
				LexError error;
				error.Type = LexErrorType::UnknownToken;
				std::string guessedToken = GuessNextToken(source, cursor);
				if (!guessedToken.empty())
					guessedToken = " " + guessedToken;
				std::string previousToken = "";
				if (!result.Tokens.empty())
					previousToken = " after " + result.Tokens.back().Value;
				error.Description = "[" + FormatSourceLocation(cursor.Location) + "] Unable to lex token" + guessedToken + previousToken;
				result.Errors.push_back(error);
				break;
			}
		}

		if (result.Errors.empty())
		{
			Token eof;
			eof.Type = TokenType::EndOfFile;
			eof.Location = cursor.Location;
			result.Tokens.push_back(eof);
		}

		return result;
	}

	template LexResult LexSQL<true>(const std::string&);
	template LexResult LexSQL<false>(const std::string&);

}
