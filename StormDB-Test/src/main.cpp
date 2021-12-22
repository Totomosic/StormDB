#include "catch_amalgamated.hpp"
#include "StormDB.h"

using namespace StormDB;

TEST_CASE("Lexer", "[Lexer]")
{
	Init();

	// + 1 token counts represent the EOF token

	std::string source = "SELECT * FROM poles;";
	LexResult result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 5 + 1);

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
	REQUIRE(result.Tokens.size() == 12 + 1);

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
	REQUIRE(result.Tokens.size() == 1 + 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_INSERT);

	source = "table";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1 + 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Keyword);
	REQUIRE(result.Tokens[0].Value == KEYWORD_TABLE);

	source = "poles";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1 + 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Identifier);
	REQUIRE(result.Tokens[0].Value == "poles");

	source = "-100.431";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1 + 1);

	REQUIRE(result.Tokens[0].Type == TokenType::NumericLiteral);
	REQUIRE(result.Tokens[0].Value == "-100.431");

	source = ";";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 1 + 1);

	REQUIRE(result.Tokens[0].Type == TokenType::Symbol);
	REQUIRE(result.Tokens[0].Value == SYMBOL_SEMICOLON);

	source = "(x+y)/(z + z * 3) - 10.01";
	result = LexSQL(source);
	REQUIRE(result.Errors.empty());
	REQUIRE(result.Tokens.size() == 15 + 1);

	source = "5abc";
	result = LexSQL(source);
	REQUIRE(!result.Errors.empty());
}

TEST_CASE("Expressions", "[Parser]")
{
	Init();
	uint32_t cursor = 0;

	SECTION("Basic literal expression")
	{
		std::string source = "1";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Literal);
		REQUIRE(result->Literal.Type == TokenType::NumericLiteral);
		REQUIRE(result->Literal.Value == "1");
	}

	SECTION("Basic binary expression")
	{
		std::string source = "1 + 3";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Binary);
		REQUIRE(result->Binary != nullptr);
		REQUIRE(result->Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(result->Binary->Operator.Value == SYMBOL_PLUS);
		REQUIRE(result->Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(result->Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(result->Binary->Left.Literal.Value == "1");
		REQUIRE(result->Binary->Right.Literal.Value == "3");
	}

	SECTION("Compound binary expression")
	{
		std::string source = "2 * 4 - (3 + 5 / 4)";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Binary);
		REQUIRE(result->Binary != nullptr);
		REQUIRE(result->Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(result->Binary->Operator.Value == SYMBOL_MINUS);

		SQLExpression& left = result->Binary->Left;
		SQLExpression& right = result->Binary->Right;

		REQUIRE(left.Type == ExpressionType::Binary);
		REQUIRE(right.Type == ExpressionType::Binary);

		REQUIRE(left.Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(left.Binary->Operator.Value == SYMBOL_MULTIPLY);
		REQUIRE(left.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Left.Literal.Value == "2");
		REQUIRE(left.Binary->Right.Literal.Value == "4");

		REQUIRE(right.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(right.Binary->Right.Type == ExpressionType::Binary);
		REQUIRE(right.Binary->Left.Literal.Value == "3");

		REQUIRE(right.Binary->Right.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(right.Binary->Right.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(right.Binary->Right.Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(right.Binary->Right.Binary->Operator.Value == SYMBOL_DIVIDE);
		REQUIRE(right.Binary->Right.Binary->Left.Literal.Value == "5");
		REQUIRE(right.Binary->Right.Binary->Right.Literal.Value == "4");
	}

	SECTION("Boolean expression")
	{
		std::string source = "3 + 4 <= 5";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Binary);
		REQUIRE(result->Binary != nullptr);
		REQUIRE(result->Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(result->Binary->Operator.Value == SYMBOL_LTE);

		SQLExpression& left = result->Binary->Left;
		SQLExpression& right = result->Binary->Right;

		REQUIRE(left.Type == ExpressionType::Binary);
		REQUIRE(right.Type == ExpressionType::Literal);

		REQUIRE(left.Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(left.Binary->Operator.Value == SYMBOL_PLUS);
		REQUIRE(left.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Left.Literal.Value == "3");
		REQUIRE(left.Binary->Right.Literal.Value == "4");

		REQUIRE(right.Literal.Type == TokenType::NumericLiteral);
		REQUIRE(right.Literal.Value == "5");
	}

	SECTION("Compound boolean expression")
	{
		std::string source = "3 + 4 <= 5 and 5 = 6";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Binary);
		REQUIRE(result->Binary != nullptr);
		REQUIRE(result->Binary->Operator.Type == TokenType::Keyword);
		REQUIRE(result->Binary->Operator.Value == KEYWORD_AND);

		SQLExpression& left = result->Binary->Left;
		SQLExpression& right = result->Binary->Right;

		REQUIRE(left.Type == ExpressionType::Binary);
		REQUIRE(right.Type == ExpressionType::Binary);

		REQUIRE(left.Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(left.Binary->Operator.Value == SYMBOL_LTE);
		REQUIRE(left.Binary->Left.Type == ExpressionType::Binary);
		REQUIRE(left.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Left.Binary->Operator.Value == SYMBOL_PLUS);
		REQUIRE(left.Binary->Left.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Left.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(left.Binary->Right.Literal.Value == "5");

		REQUIRE(right.Binary->Operator.Value == SYMBOL_EQUALS);
		REQUIRE(right.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(right.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(right.Binary->Left.Literal.Value == "5");
		REQUIRE(right.Binary->Right.Literal.Value == "6");
	}

	SECTION("Multiple parentheses")
	{
		std::string source = "((((1 + 1))))";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(result.has_value());
		REQUIRE(result->Type == ExpressionType::Binary);
		REQUIRE(result->Binary != nullptr);
		REQUIRE(result->Binary->Operator.Type == TokenType::Symbol);
		REQUIRE(result->Binary->Operator.Value == SYMBOL_PLUS);

		REQUIRE(result->Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(result->Binary->Right.Type == ExpressionType::Literal);

		REQUIRE(result->Binary->Left.Literal.Value == "1");
		REQUIRE(result->Binary->Right.Literal.Value == "1");
	}

	SECTION("Missing operand")
	{
		std::string source = "3 +";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(!result.has_value());
		std::cout << result.get_error() << std::endl;
	}

	SECTION("Start with operator")
	{
		std::string source = "+ 5";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(!result.has_value());
		std::cout << result.get_error() << std::endl;
	}

	SECTION("Too many opening parentheses")
	{
		std::string source = "((3 + 4) / 5";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(!result.has_value());
		std::cout << result.get_error() << std::endl;
	}

	SECTION("Too many closing parentheses")
	{
		std::string source = "(3 + 4) / 5))";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(!result.has_value());
		std::cout << result.get_error() << std::endl;
	}

	SECTION("2 operators")
	{
		std::string source = "2 ++ 2";
		LexResult lexResult = LexSQL(source);
		Optional<SQLExpression> result = ParseExpression(lexResult.Tokens, cursor, {});

		REQUIRE(!result.has_value());
		std::cout << result.get_error() << std::endl;
	}
}

TEST_CASE("ParseSQL", "[Parser]")
{
	Init();

	SECTION("Basic SELECT")
	{
		std::string source = "SELECT id from users;";
		LexResult lexResult = LexSQL(source);
		ParseResult result = ParseSQL(lexResult.Tokens);

		REQUIRE(result.Error.empty());
		REQUIRE(result.Statements.size() == 1);
		REQUIRE(result.Statements[0].Type == StatementType::Select);
		SelectStatement& select = result.Statements[0].Select;
		REQUIRE(select.Columns.size() == 1);
		REQUIRE(select.Columns[0].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[0].Literal.Type == TokenType::Identifier);
		REQUIRE(select.Columns[0].Literal.Value == "id");
		
		REQUIRE(select.Table.Type == TokenType::Identifier);
		REQUIRE(select.Table.Value == "users");

		REQUIRE(select.Where.Type == ExpressionType::None);
	}

	SECTION("Select all")
	{
		std::string source = "SELECT * from users;";
		LexResult lexResult = LexSQL(source);
		ParseResult result = ParseSQL(lexResult.Tokens);

		REQUIRE(result.Error.empty());
		REQUIRE(result.Statements.size() == 1);
		REQUIRE(result.Statements[0].Type == StatementType::Select);
		SelectStatement& select = result.Statements[0].Select;
		REQUIRE(select.Columns.size() == 1);
		REQUIRE(select.Columns[0].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[0].Literal.Type == TokenType::Symbol);
		REQUIRE(select.Columns[0].Literal.Value == SYMBOL_ASTERISK);

		REQUIRE(select.Table.Type == TokenType::Identifier);
		REQUIRE(select.Table.Value == "users");

		REQUIRE(select.Where.Type == ExpressionType::None);
	}

	SECTION("Select WHERE condition")
	{
		std::string source = "SELECT id from users WHERE id = 5;";
		LexResult lexResult = LexSQL(source);
		ParseResult result = ParseSQL(lexResult.Tokens);

		REQUIRE(result.Error.empty());
		REQUIRE(result.Statements.size() == 1);
		REQUIRE(result.Statements[0].Type == StatementType::Select);
		SelectStatement& select = result.Statements[0].Select;
		REQUIRE(select.Columns.size() == 1);
		REQUIRE(select.Columns[0].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[0].Literal.Type == TokenType::Identifier);
		REQUIRE(select.Columns[0].Literal.Value == "id");

		REQUIRE(select.Table.Type == TokenType::Identifier);
		REQUIRE(select.Table.Value == "users");

		REQUIRE(select.Where.Type == ExpressionType::Binary);
		REQUIRE(select.Where.Binary->Left.Type == ExpressionType::Literal);
		REQUIRE(select.Where.Binary->Right.Type == ExpressionType::Literal);
		REQUIRE(select.Where.Binary->Left.Literal.Value == "id");
		REQUIRE(select.Where.Binary->Right.Literal.Value == "5");
		REQUIRE(select.Where.Binary->Left.Literal.Type == TokenType::Identifier);
		REQUIRE(select.Where.Binary->Right.Literal.Type == TokenType::NumericLiteral);
	}

	SECTION("Select no table")
	{
		std::string source = "SELECT 10, 20, 'test';";
		LexResult lexResult = LexSQL(source);
		ParseResult result = ParseSQL(lexResult.Tokens);

		REQUIRE(result.Error.empty());
		REQUIRE(result.Statements.size() == 1);
		REQUIRE(result.Statements[0].Type == StatementType::Select);
		SelectStatement& select = result.Statements[0].Select;
		REQUIRE(select.Columns.size() == 3);

		REQUIRE(select.Columns[0].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[0].Literal.Type == TokenType::NumericLiteral);
		REQUIRE(select.Columns[0].Literal.Value == "10");

		REQUIRE(select.Columns[1].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[1].Literal.Type == TokenType::NumericLiteral);
		REQUIRE(select.Columns[1].Literal.Value == "20");

		REQUIRE(select.Columns[2].Type == ExpressionType::Literal);
		REQUIRE(select.Columns[2].Literal.Type == TokenType::StringLiteral);
		REQUIRE(select.Columns[2].Literal.Value == "test");
	}
}
