#include "Parser.h"
#include <optional>

namespace StormDB
{

	template<typename T>
	using ExpressionFunction = std::optional<T>(*)(const std::vector<Token>&, uint32_t&);

	template<typename T>
	struct STORMDB_API ExpressionResult
	{
	public:
		std::vector<T> Expressions;
		std::string Error = "";
	};

	bool ExpectToken(const std::vector<Token>& tokens, uint32_t cursor, const Token& token)
	{
		if (cursor >= tokens.size())
			return false;
		return tokens[cursor] == token;
	}

	std::string CreateErrorMessage(const std::vector<Token>& tokens, uint32_t cursor, const std::string& message)
	{
		Token token = cursor >= tokens.size() ? tokens[cursor - 1] : tokens[cursor];
		return "[" + FormatSourceLocation(token.Location) + "]: " + message + ", got: " + token.Value;
	}

	std::optional<Token> ParseToken(const std::vector<Token>& tokens, uint32_t& cursor, TokenType type)
	{
		if (cursor >= tokens.size())
			return {};
		if (tokens[cursor].Type == type)
		{
			cursor++;
			return tokens[cursor - 1];
		}
		return {};
	}

	std::optional<SQLExpression> ParseExpression(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		TokenType tokenTypes[] = { TokenType::Identifier, TokenType::NumericLiteral, TokenType::StringLiteral };
		for (TokenType type : tokenTypes)
		{
			std::optional<Token> token = ParseToken(tokens, cursor, type);
			if (token)
			{
				SQLExpression expression;
				expression.Type = ExpressionType::Literal;
				expression.Literal = token.value();
				return expression;
			}
		}
		return {};
	}

	std::optional<ColumnDefinition> ParseColumnDefinition(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		std::optional<Token> name = ParseToken(tokens, cursor, TokenType::Identifier);
		if (name)
		{
			if (ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_INT)) || ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_TEXT)) || ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_BOOL)))
			{
				ColumnDefinition definition;
				definition.Name = name.value();
				definition.Datatype = tokens[cursor];
				cursor++;
				return definition;
			}
		}
		return {};
	}

	template<typename T>
	ExpressionResult<T> ParseExpressions(const std::vector<Token>& tokens, uint32_t& cursor, const Token* const delimiters, size_t delimiterCount, ExpressionFunction<T> fn)
	{
		ExpressionResult<T> result;
		while (cursor < tokens.size())
		{
			const Token& current = tokens[cursor];
			for (size_t i = 0; i < delimiterCount; i++)
			{
				if (current == delimiters[i])
					return result;
			}
			if (!result.Expressions.empty())
			{
				if (!ExpectToken(tokens, cursor, TokenFromSymbol(SYMBOL_COMMA)))
				{
					result.Error = CreateErrorMessage(tokens, cursor, "Expected comma");
					return result;
				}
				cursor++;
			}

			std::optional<T> expression = fn(tokens, cursor);
			if (!expression)
			{
				result.Error = CreateErrorMessage(tokens, cursor, "Expected expression");
				return result;
			}
			result.Expressions.push_back(expression.value());
		}
		return result;
	}

	std::optional<StatementResult<SelectStatement>> ParseSelectStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		StatementResult<SelectStatement> statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_SELECT)))
		{
			c++;
			Token fromToken = TokenFromKeyword(KEYWORD_FROM);
			ExpressionResult<SQLExpression> expressions = ParseExpressions<SQLExpression>(tokens, c, &fromToken, 1, ParseExpression);
			if (!expressions.Error.empty())
			{
				if (ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_ASTERISK)))
				{
					SQLExpression expression;
					expression.Type = ExpressionType::Literal;
					expression.Literal = tokens[c];
					expressions.Expressions.push_back(expression);
					c++;
				}
				else
				{
					statement.Error = expressions.Error;
					return statement;
				}
			}
			if (expressions.Expressions.empty())
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected column identifiers");
				return statement;
			}
			statement.Statement.Columns = expressions.Expressions;
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_FROM)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected FROM-Clause");
				return statement;
			}
			c++;

			std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
			if (!table)
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected table identifier");
				return statement;
			}
			statement.Statement.Table = table.value();

			cursor = c;
			return statement;
		}
		return {};
	}

	std::optional<StatementResult<InsertStatement>> ParseInsertStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		StatementResult<InsertStatement> statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_INSERT)))
		{
			c++;
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_INTO)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected INTO keyword");
				return statement;
			}
			c++;
			std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
			if (!table)
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected table identitifer");
				return statement;
			}
			statement.Statement.Table = table.value();
			
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_VALUES)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected VALUES keyword");
				return statement;
			}
			c++;
			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected '('");
				return statement;
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			ExpressionResult<SQLExpression> expressions = ParseExpressions<SQLExpression>(tokens, c, &rightParen, 1, ParseExpression);
			if (!expressions.Error.empty())
			{
				statement.Error = expressions.Error;
				return statement;
			}
			if (expressions.Expressions.empty())
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected values");
				return statement;
			}
			statement.Statement.Values = expressions.Expressions;

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected ')'");
				return statement;
			}
			c++;

			cursor = c;
			return statement;
		}
		return {};
	}

	std::optional<StatementResult<CreateStatement>> ParseCreateStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		StatementResult<CreateStatement> statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_CREATE)))
		{
			c++;
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_TABLE)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected TABLE keyword");
				return statement;
			}
			c++;

			std::optional<Token> tableName = ParseToken(tokens, c, TokenType::Identifier);
			if (!tableName)
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected table name identifier");
				return statement;
			}

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected '('");
				return statement;
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			ExpressionResult<ColumnDefinition> columnDefinitions = ParseExpressions<ColumnDefinition>(tokens, c, &rightParen, 1, ParseColumnDefinition);
			if (!columnDefinitions.Error.empty())
			{
				statement.Error = columnDefinitions.Error;
				return statement;
			}
			if (columnDefinitions.Expressions.empty())
			{
				statement.Error = "Table requires at least 1 column";
				return statement;
			}

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
			{
				statement.Error = CreateErrorMessage(tokens, c, "Expected ')'");
				return statement;
			}
			c++;

			cursor = c;
			return statement;
		}
		return {};
	}

	std::optional<SQLStatement> ParseStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		SQLStatement statement;
		std::optional<StatementResult<SelectStatement>> select = ParseSelectStatement(tokens, cursor);
		if (select.has_value())
		{
			statement.Type = StatementType::Select;
			statement.Select = select.value().Statement;
			statement.Error = select->Error;
			return statement;
		}
		std::optional<StatementResult<InsertStatement>> insert = ParseInsertStatement(tokens, cursor);
		if (insert.has_value())
		{
			statement.Type = StatementType::Insert;
			statement.Insert = insert.value().Statement;
			statement.Error = insert->Error;
			return statement;
		}
		std::optional<StatementResult<CreateStatement>> create = ParseCreateStatement(tokens, cursor);
		if (create.has_value())
		{
			statement.Type = StatementType::Create;
			statement.Create = create.value().Statement;
			statement.Error = create->Error;
			return statement;
		}
		return {};
	}

	ParseResult ParseSQL(const std::vector<Token>& tokens)
	{
		ParseResult result;
		uint32_t cursor = 0;
		while (cursor < tokens.size())
		{
			std::optional<SQLStatement> statement = ParseStatement(tokens, cursor);
			if (statement.has_value() && !statement->Error.empty())
			{
				result.Error = statement->Error;
				return result;
			}
			if (!statement.has_value())
			{
				result.Error = CreateErrorMessage(tokens, cursor, "Expected statement");
				return result;
			}
			result.Statements.push_back(statement.value());

			Token semicolon = TokenFromSymbol(SYMBOL_SEMICOLON);
			bool hasSemicolon = false;
			while (ExpectToken(tokens, cursor, semicolon))
			{
				cursor++;
				hasSemicolon = true;
			}
			if (!hasSemicolon)
			{
				result.Error = CreateErrorMessage(tokens, cursor, "Expected semi-colon");
				return result;
			}
		}
		return result;
	}

}
