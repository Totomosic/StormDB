#include "Reconstruct.h"
#include <algorithm>
#include <map>

namespace StormDB
{

	using Reconstructor = void(*)(const Token&, std::string&, SourceLocation&);

	void AddWhitespace(const SourceLocation& current, const SourceLocation& required, std::string& str)
	{
		if (required.Line > current.Line)
		{
			str += std::string(required.Line - current.Line, '\n');
			if (required.Column > 0)
			{
				str += std::string(required.Column, ' ');
			}
		}
		else if (required.Column > current.Column)
		{
			str += std::string(required.Column - current.Column, ' ');
		}
	}

	static void ReconstructKeyword(const Token& token, std::string& str, SourceLocation& location)
	{
		std::string value = token.Value;
		std::transform(value.begin(), value.end(), value.begin(), [](char c) { return std::toupper(c); });
		str += value;
		location = token.Location;
		location.Column += value.size();
	}

	static void ReconstructComment(const Token& token, std::string& str, SourceLocation& location)
	{
		str += "--" + token.Value;
		location = token.Location;
		location.Column += token.Value.size() + 2;
	}

	static void ReconstructString(const Token& token, std::string& str, SourceLocation& location)
	{
		std::string value = "'";
		for (char c : token.Value)
		{
			if (c == '\'')
				value += "''";
			else
				value += c;
		}
		value += '\'';
		str += value;
		location = token.Location;
		location.Column += value.size();
	}

	static void ReconstructEOF(const Token& token, std::string& str, SourceLocation& location)
	{
	}

	static void ReconstructDefault(const Token& token, std::string& str, SourceLocation& location)
	{
		str += token.Value;
		location = token.Location;
		location.Column += token.Value.size();
	}

	static std::map<TokenType, Reconstructor> RECONSTRUCTOR_MAP = {
		{ TokenType::Keyword, ReconstructKeyword },
		{ TokenType::Identifier, ReconstructDefault },
		{ TokenType::Comment, ReconstructComment },
		{ TokenType::NumericLiteral, ReconstructDefault },
		{ TokenType::StringLiteral, ReconstructString },
		{ TokenType::Symbol, ReconstructDefault },
		{ TokenType::EndOfFile, ReconstructEOF },
	};

	std::string ReconstructSource(const std::vector<Token>& tokens)
	{
		std::string result = "";
		SourceLocation location;
		for (const Token& token : tokens)
		{
			STORMDB_ASSERT((token.Location.Column >= location.Column && token.Location.Line == location.Line) || token.Location.Line > location.Line, "Invalid token location");
			AddWhitespace(location, token.Location, result);
			RECONSTRUCTOR_MAP[token.Type](token, result, location);
		}
		return result;
	}

}
