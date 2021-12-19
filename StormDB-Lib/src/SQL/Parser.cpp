#include "Parser.h"
#include <optional>

namespace StormDB
{

	template<typename T>
	using ExpressionFunction = Optional<T>(*)(const std::vector<Token>&, uint32_t&, const std::vector<Token>&);

	template<typename T>
	std::vector<T> CombineVectors(const std::vector<T>& left, const std::vector<T>& right)
	{
		std::vector<T> result = left;
		result.insert(result.end(), right.begin(), right.end());
		return result;
	}

	int GetBindingPower(const Token& token)
	{
		if (token.Type == TokenType::Keyword)
		{
			if (token.Value == KEYWORD_AND)
				return 1;
			if (token.Value == KEYWORD_OR)
				return 1;
		}
		if (token.Type == TokenType::Symbol)
		{
			if (token.Value == SYMBOL_EQUALS)
				return 3;
			if (token.Value == SYMBOL_NOT_EQUALS || token.Value == SYMBOL_NOT_EQUALS_2)
				return 3;
			if (token.Value == SYMBOL_GT || token.Value == SYMBOL_GTE)
				return 3;
			if (token.Value == SYMBOL_LT || token.Value == SYMBOL_LTE)
				return 3;
			if (token.Value == SYMBOL_PLUS || token.Value == SYMBOL_MINUS)
				return 3;

			if (token.Value == SYMBOL_MULTIPLY || token.Value == SYMBOL_DIVIDE)
				return 5;
		}
	}

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

	std::optional<Token> ParseToken(const std::vector<Token>& tokens, uint32_t& cursor, Token token)
	{
		if (cursor >= tokens.size())
			return {};
		if (tokens[cursor] == token)
		{
			cursor++;
			return tokens[cursor - 1];
		}
		return {};
	}

	Optional<SQLExpression> ParseLiteralExpression(const std::vector<Token>& tokens, uint32_t& cursor)
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

	Optional<SQLExpression> ParseExpression(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters, int minBindingPower)
	{
		Optional<SQLExpression> expression = {};
		std::optional<Token> leftParen = ParseToken(tokens, cursor, TokenFromSymbol(SYMBOL_LEFT_PAREN));
		if (leftParen)
		{
			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			expression = ParseExpression(tokens, cursor, CombineVectors(delimiters, { rightParen }), minBindingPower);
			if (!expression)
			{
				return expression;
			}
			std::optional<Token> end = ParseToken(tokens, cursor, rightParen);
			if (!end)
			{
				return Optional<SQLExpression>::from_error(CreateErrorMessage(tokens, cursor, "Expected ')'"));
			}
		}
		else
		{
			expression = ParseLiteralExpression(tokens, cursor);
			if (!expression)
			{
				return expression;
			}
		}

		const Token binaryOperations[] = {
			TokenFromKeyword(KEYWORD_AND),
			TokenFromKeyword(KEYWORD_OR),
			TokenFromSymbol(SYMBOL_PLUS),
			TokenFromSymbol(SYMBOL_MINUS),
			TokenFromSymbol(SYMBOL_MULTIPLY),
			TokenFromSymbol(SYMBOL_DIVIDE),
			TokenFromSymbol(SYMBOL_EQUALS),
			TokenFromSymbol(SYMBOL_NOT_EQUALS),
			TokenFromSymbol(SYMBOL_NOT_EQUALS_2),
			TokenFromSymbol(SYMBOL_GT),
			TokenFromSymbol(SYMBOL_GTE),
			TokenFromSymbol(SYMBOL_LT),
			TokenFromSymbol(SYMBOL_LTE),
		};

		uint32_t lastCursor = cursor;

		while (cursor < tokens.size())
		{
			for (const Token& token : delimiters)
			{
				if (tokens[cursor] == token)
					return expression;
			}
			Token op;
			for (const Token& operation : binaryOperations)
			{
				std::optional<Token> t = ParseToken(tokens, cursor, operation);
				if (t)
				{
					op = t.value();
					break;
				}
			}
			if (!TokenValid(op))
			{
				return Optional<SQLExpression>::from_error(CreateErrorMessage(tokens, cursor, "Expected binary operation"));
			}
			int bindingPower = GetBindingPower(op);
			if (bindingPower < minBindingPower)
			{
				cursor = lastCursor;
				break;
			}
			Optional<SQLExpression> b = ParseExpression(tokens, cursor, delimiters, bindingPower);
			if (!b)
			{
				return b;
			}
			SQLExpression exp;
			exp.Binary = std::make_shared<BinaryExpression>();
			exp.Binary->Left = expression.value();
			exp.Binary->Right = b.value();
			exp.Binary->Operator = op;
			exp.Type = ExpressionType::Binary;
			expression = exp;
			lastCursor = cursor;
		}
		return expression;
	}

	Optional<SQLExpression> ParseExpression(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters)
	{
		return ParseExpression(tokens, cursor, delimiters, 0);
	}

	Optional<ColumnDefinition> ParseColumnDefinition(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters)
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
			return Optional<ColumnDefinition>::from_error(CreateErrorMessage(tokens, cursor, "Expected column type definition"));
		}
		return Optional<ColumnDefinition>::from_error(CreateErrorMessage(tokens, cursor, "Expected column name"));
	}

	template<typename T>
	Optional<std::vector<T>> ParseExpressions(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters, ExpressionFunction<T> fn)
	{
		std::vector<T> result;
		while (cursor < tokens.size())
		{
			const Token& current = tokens[cursor];
			for (const Token& delimiter : delimiters)
			{
				if (current == delimiter)
					return result;
			}
			if (!result.empty())
			{
				if (!ExpectToken(tokens, cursor, TokenFromSymbol(SYMBOL_COMMA)))
				{
					return Optional<std::vector<T>>::from_error(CreateErrorMessage(tokens, cursor, "Expected comma"));
				}
				cursor++;
			}

			Optional<T> expression = fn(tokens, cursor, CombineVectors(delimiters, { TokenFromSymbol(SYMBOL_COMMA) }));
			if (!expression)
			{
				return Optional<std::vector<T>>::from_error(expression.get_error());
			}
			result.push_back(expression.value());
		}
		return result;
	}

	Optional<SelectStatement> ParseSelectStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		SelectStatement statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_SELECT)))
		{
			c++;
			Token fromToken = TokenFromKeyword(KEYWORD_FROM);
			Optional<std::vector<SQLExpression>> expressions = ParseExpressions<SQLExpression>(tokens, c, { fromToken, TokenFromKeyword(KEYWORD_WHERE), TokenFromSymbol(SYMBOL_SEMICOLON) }, ParseExpression);
			if (!expressions)
			{
				if (ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_ASTERISK)))
				{
					SQLExpression expression;
					expression.Type = ExpressionType::Literal;
					expression.Literal = tokens[c];
					expressions = { expression };
					c++;
				}
				else
				{
					return Optional<SelectStatement>::from_error(expressions.get_error());
				}
			}
			if (expressions->empty())
			{
				return Optional<SelectStatement>::from_error(CreateErrorMessage(tokens, c, "Expected column identifiers"));
			}
			statement.Columns = expressions.value();
			if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_FROM)))
			{
				c++;
				std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
				if (!table)
				{
					return Optional<SelectStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table identifier"));
				}
				statement.Table = table.value();
			}

			if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_WHERE)))
			{
				c++;
				if (!TokenValid(statement.Table))
				{
					return Optional<SelectStatement>::from_error(CreateErrorMessage(tokens, c, "WHERE clause found with no FROM clause"));
				}
				Optional<SQLExpression> whereExpression = ParseExpression(tokens, c, { TokenFromSymbol(SYMBOL_SEMICOLON) });
				if (!whereExpression)
				{
					return Optional<SelectStatement>::from_error(whereExpression.get_error());
				}
				statement.Where = whereExpression.value();
			}
			
			cursor = c;
			return statement;
		}
		return {};
	}

	Optional<InsertStatement> ParseInsertStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		InsertStatement statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_INSERT)))
		{
			c++;
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_INTO)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected INTO keyword"));
			}
			c++;
			std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
			if (!table)
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table identitifer"));
			}
			statement.Table = table.value();
			
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_VALUES)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected VALUES keyword"));
			}
			c++;
			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected '('"));
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			Optional<std::vector<SQLExpression>> expressions = ParseExpressions<SQLExpression>(tokens, c, { rightParen }, ParseExpression);
			if (!expressions)
			{
				return Optional<InsertStatement>::from_error(expressions.get_error());
			}
			if (expressions->empty())
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected values"));
			}
			statement.Values = expressions.value();

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected ')'"));
			}
			c++;

			cursor = c;
			return statement;
		}
		return {};
	}

	Optional<CreateStatement> ParseCreateStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		uint32_t c = cursor;
		CreateStatement statement;
		if (ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_CREATE)))
		{
			c++;
			if (!ExpectToken(tokens, c, TokenFromKeyword(KEYWORD_TABLE)))
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected TABLE keyword"));
			}
			c++;

			std::optional<Token> tableName = ParseToken(tokens, c, TokenType::Identifier);
			if (!tableName)
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table name identifier"));
			}

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected '('"));
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			Optional<std::vector<ColumnDefinition>> columnDefinitions = ParseExpressions<ColumnDefinition>(tokens, c, { rightParen }, ParseColumnDefinition);
			if (!columnDefinitions)
			{
				return Optional<CreateStatement>::from_error(columnDefinitions.get_error());
			}
			if (columnDefinitions->empty())
			{
				return Optional<CreateStatement>::from_error("Table requires at least 1 column");
			}

			if (!ExpectToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected ')'"));
			}
			c++;

			cursor = c;
			return statement;
		}
		return {};
	}

	Optional<SQLStatement> ParseStatement(const std::vector<Token>& tokens, uint32_t& cursor)
	{
		SQLStatement statement;
		if (ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_SELECT)))
		{
			Optional<SelectStatement> select = ParseSelectStatement(tokens, cursor);
			if (select)
			{
				statement.Type = StatementType::Select;
				statement.Select = std::move(select.value());
				return statement;
			}
			return Optional<SQLStatement>::from_error(select.get_error());
		}
		else if (ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_INSERT)))
		{
			Optional<InsertStatement> insert = ParseInsertStatement(tokens, cursor);
			if (insert)
			{
				statement.Type = StatementType::Insert;
				statement.Insert = std::move(insert.value());
				return statement;
			}
			return Optional<SQLStatement>::from_error(insert.get_error());
		}
		else if (ExpectToken(tokens, cursor, TokenFromKeyword(KEYWORD_CREATE)))
		{
			Optional<CreateStatement> create = ParseCreateStatement(tokens, cursor);
			if (create)
			{
				statement.Type = StatementType::Create;
				statement.Create = std::move(create.value());
				return statement;
			}
			return Optional<SQLStatement>::from_error(create.get_error());
		}
		return Optional<SQLStatement>::from_error(CreateErrorMessage(tokens, cursor, "Expected statement"));
	}

	std::vector<Token> FilterComments(const std::vector<Token>& tokens)
	{
		std::vector<Token> result;
		result.reserve(tokens.size());
		for (const Token& token : tokens)
		{
			if (token.Type != TokenType::Comment)
				result.push_back(token);
		}
		return result;
	}

	ParseResult ParseSQL(const std::vector<Token>& tokens)
	{
		ParseResult result;
		uint32_t cursor = 0;
		while (cursor < tokens.size())
		{
			Optional<SQLStatement> statement = ParseStatement(tokens, cursor);
			if (!statement)
			{
				result.Error = statement.get_error();
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
