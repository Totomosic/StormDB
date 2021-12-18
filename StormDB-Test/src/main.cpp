#include "catch_amalgamated.hpp"
#include "StormDB.h"

using namespace StormDB;

TEST_CASE("Lexer", "Lexer")
{
	Init();

	std::string source = "SELECT * FROM poles;";
	LexResult result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 5);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_SELECT);

	REQUIRE(result.Tokens[1].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[1].Value == SYMBOL_ASTERISK);

	REQUIRE(result.Tokens[2].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[2].Value == KEYWORD_FROM);

	REQUIRE(result.Tokens[3].Type == TokenType::Identifier);
	REQUIRE(result.Tokens[3].Value == "poles");

	REQUIRE(result.Tokens[4].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[4].Value == SYMBOL_SEMICOLON);

	source = "INSERT into POLES VALUES (1, -2.45, 'Hello''s');";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 12);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_INSERT);

	REQUIRE(result.Tokens[1].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[1].Value == KEYWORD_INTO);

	REQUIRE(result.Tokens[2].Type == TokenType::Identifier);
	REQUIRE(result.Tokens[2].Value == "poles");

	REQUIRE(result.Tokens[3].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[3].Value == KEYWORD_VALUES);

	REQUIRE(result.Tokens[4].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[4].Value == SYMBOL_LEFT_PAREN);

	REQUIRE(result.Tokens[5].Type == TokenType::NumericLiteral);
	REQUIRE(result.Tokens[5].Value == "1");

	REQUIRE(result.Tokens[6].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[6].Value == SYMBOL_COMMA);

	REQUIRE(result.Tokens[7].Type == TokenType::NumericLiteral);
	REQUIRE(result.Tokens[7].Value == "-2.45");

	REQUIRE(result.Tokens[8].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[8].Value == SYMBOL_COMMA);

	REQUIRE(result.Tokens[9].Type == TokenType::StringLiteral);
	REQUIRE(result.Tokens[9].Value == "Hello's");

	REQUIRE(result.Tokens[10].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[10].Value == SYMBOL_RIGHT_PAREN);

	REQUIRE(result.Tokens[11].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[11].Value == SYMBOL_SEMICOLON);

	source = "insert";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_INSERT);

	source = "table";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_TABLE);

	source = "poles";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Identifier);
	REQUIRE(result.Tokens[0].Value == "poles");

	source = "-100.431";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1);

	REQUIRE(result.Tokens[0].Type == TokenType::NumericLiteral);
	REQUIRE(result.Tokens[0].Value == "-100.431");

	source = ";";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[0].Value == SYMBOL_SEMICOLON);
}
