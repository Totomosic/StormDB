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

	// Essentially order of operations
	int GetBindingPower(const Token& token)
	{
		if (token.Type == TokenType::Keyword)
		{
			if (token.Value == KEYWORD_AND)
				return 2;
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
				return 4;

			if (token.Value == SYMBOL_MULTIPLY || token.Value == SYMBOL_DIVIDE)
				return 5;
		}
		return 0;
	}

	std::string FormatTokenValue(const Token& token)
	{
		if (token.Type == TokenType::EndOfFile)
			return "EOF";
		if (token.Type == TokenType::Symbol)
			return "'" + token.Value + "'";
		return token.Value;
	}

	// Utility method to provide a location and some context for error messages
	std::string CreateErrorMessage(const std::vector<Token>& tokens, uint32_t cursor, const std::string& message)
	{
		Token token = cursor >= tokens.size() ? tokens[cursor - 1] : tokens[cursor];
		return "[" + FormatSourceLocation(token.Location) + "]: " + message + ", got: " + FormatTokenValue(token);
	}

	bool CheckToken(const std::vector<Token>& tokens, uint32_t cursor, const Token& token)
	{
		if (cursor >= tokens.size())
			return false;
		return tokens[cursor] == token;
	}

	// Checks that the token is matches the given token type and increments the cursor
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

	// Checks that the token is matches the given token and increments the cursor
	std::optional<Token> ParseToken(const std::vector<Token>& tokens, uint32_t& cursor, const Token& token)
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
		return Optional<SQLExpression>::from_error(CreateErrorMessage(tokens, cursor, "Expected literal"));
	}

	Optional<SQLExpression> ParseExpression(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters, int minBindingPower)
	{
		Optional<SQLExpression> expression = {};
		// Check for open parenthesis
		std::optional<Token> leftParen = ParseToken(tokens, cursor, TokenFromSymbol(SYMBOL_LEFT_PAREN));
		if (leftParen)
		{
			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			// Parse expression inside parentheses
			expression = ParseExpression(tokens, cursor, CombineVectors(delimiters, { rightParen }), minBindingPower);
			if (!expression)
			{
				return expression;
			}
			// Check we closed the bracket
			std::optional<Token> end = ParseToken(tokens, cursor, rightParen);
			if (!end)
			{
				return Optional<SQLExpression>::from_error(CreateErrorMessage(tokens, cursor, "Expected ')'"));
			}
		}
		else
		{
			// Expect a literal (literal value or identifier) to start the expression
			expression = ParseLiteralExpression(tokens, cursor);
			if (!expression)
			{
				return expression;
			}
		}

		// List of all types of binary operations
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
			// Have we reached the end of where our expression should lie? (eg. SELECT [expressions] FROM ...)
			if (tokens[cursor].Type == TokenType::EndOfFile)
				return expression;
			for (const Token& token : delimiters)
			{
				if (tokens[cursor] == token)
					return expression;
			}
			Token op;
			// Find the operation symbol (+, -, *, AND, etc.)
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
			// Find the operator precedence of our operator
			int bindingPower = GetBindingPower(op);
			// If we are below the threshold then this branch of the expression tree is complete, so return
			if (bindingPower < minBindingPower)
			{
				cursor = lastCursor;
				break;
			}
			// Parse the rest of the expression past the operator
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

	Optional<SQLColumnDefinition> ParseColumnDefinition(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters)
	{
		std::optional<Token> name = ParseToken(tokens, cursor, TokenType::Identifier);
		if (name)
		{
			if (CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_INT)) || CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_TEXT)) || CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_BOOL)))
			{
				SQLColumnDefinition definition;
				definition.Name = name.value();
				definition.Datatype = tokens[cursor];
				cursor++;
				return definition;
			}
			return Optional<SQLColumnDefinition>::from_error(CreateErrorMessage(tokens, cursor, "Expected column type definition"));
		}
		return Optional<SQLColumnDefinition>::from_error(CreateErrorMessage(tokens, cursor, "Expected column name identifier"));
	}

	template<typename T>
	Optional<std::vector<T>> ParseExpressions(const std::vector<Token>& tokens, uint32_t& cursor, const std::vector<Token>& delimiters, ExpressionFunction<T> fn)
	{
		std::vector<T> result;
		while (cursor < tokens.size())
		{
			// Check we haven't reached the end of the region the expressions should be
			const Token& current = tokens[cursor];
			if (current.Type == TokenType::EndOfFile)
				return result;
			for (const Token& delimiter : delimiters)
			{
				if (current == delimiter)
					return result;
			}
			// If we already have found at least 1 expression, then we expect a comma to separate expressions
			if (!result.empty())
			{
				if (!CheckToken(tokens, cursor, TokenFromSymbol(SYMBOL_COMMA)))
				{
					return Optional<std::vector<T>>::from_error(CreateErrorMessage(tokens, cursor, "Expected comma"));
				}
				cursor++;
			}
			// Try and parse an expression at the current location, delimited by commas
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
		// Expect a SELECT keyword
		if (CheckToken(tokens, c, TokenFromKeyword(KEYWORD_SELECT)))
		{
			c++;
			Token fromToken = TokenFromKeyword(KEYWORD_FROM);
			// Then a list of expressions representing which columns to select
			Optional<std::vector<SQLExpression>> expressions = ParseExpressions<SQLExpression>(tokens, c, { fromToken, TokenFromKeyword(KEYWORD_WHERE), TokenFromSymbol(SYMBOL_SEMICOLON) }, ParseExpression);
			if (!expressions)
			{
				// * is a valid column expression
				if (CheckToken(tokens, c, TokenFromSymbol(SYMBOL_ASTERISK)))
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
			// Check if there is a FROM token
			// Selects without a FROM clause must only select constant values (eg. SELECT 1 + 1)
			if (CheckToken(tokens, c, TokenFromKeyword(KEYWORD_FROM)))
			{
				c++;
				// Find table name
				std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
				if (!table)
				{
					return Optional<SelectStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table identifier"));
				}
				statement.Table = table.value();
			}

			// Check if there is a WHERE clause
			if (CheckToken(tokens, c, TokenFromKeyword(KEYWORD_WHERE)))
			{
				c++;
				// WHERE clause only valid if there is a FROM clause (valid table name)
				if (!TokenValid(statement.Table))
				{
					return Optional<SelectStatement>::from_error(CreateErrorMessage(tokens, c, "WHERE clause found with no FROM clause"));
				}
				// Parse the WHERE expression
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
		// Expect an INSERT keyword
		if (CheckToken(tokens, c, TokenFromKeyword(KEYWORD_INSERT)))
		{
			c++;
			// Expect an INSERT keyword (must be of the form: INSERT INTO)
			if (!CheckToken(tokens, c, TokenFromKeyword(KEYWORD_INTO)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected INTO keyword"));
			}
			c++;
			// Find the table name to insert into
			std::optional<Token> table = ParseToken(tokens, c, TokenType::Identifier);
			if (!table)
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table identitifer"));
			}
			statement.Table = table.value();
			// Expect a VALUES keyword (must be of the form: INSERT INTO [table] VALUES)
			if (!CheckToken(tokens, c, TokenFromKeyword(KEYWORD_VALUES)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected VALUES keyword"));
			}
			c++;
			// Expect an open parenthesis
			if (!CheckToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected '('"));
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			// Parse expressions reprensenting the values to insert into the table
			// eg. INSERT INTO [table] VALUES ([expressions])
			Optional<std::vector<SQLExpression>> expressions = ParseExpressions<SQLExpression>(tokens, c, { rightParen }, ParseExpression);
			if (!expressions)
			{
				return Optional<InsertStatement>::from_error(expressions.get_error());
			}
			// Cannot insert nothing
			if (expressions->empty())
			{
				return Optional<InsertStatement>::from_error(CreateErrorMessage(tokens, c, "Expected values"));
			}
			statement.Values = expressions.value();
			// Expect a closing parenthesis
			if (!CheckToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
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
		// Expect a CREATE keyword
		if (CheckToken(tokens, c, TokenFromKeyword(KEYWORD_CREATE)))
		{
			c++;
			// Expect a TABLE keyword (must be of the form: CREATE TABLE)
			if (!CheckToken(tokens, c, TokenFromKeyword(KEYWORD_TABLE)))
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected TABLE keyword"));
			}
			c++;
			// Find the name of the table to create
			std::optional<Token> tableName = ParseToken(tokens, c, TokenType::Identifier);
			if (!tableName)
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected table name identifier"));
			}
			// Expect an open parenthesis
			if (!CheckToken(tokens, c, TokenFromSymbol(SYMBOL_LEFT_PAREN)))
			{
				return Optional<CreateStatement>::from_error(CreateErrorMessage(tokens, c, "Expected '('"));
			}
			c++;

			Token rightParen = TokenFromSymbol(SYMBOL_RIGHT_PAREN);
			// Find the column definitions between the opening and closing parentheses
			Optional<std::vector<SQLColumnDefinition>> columnDefinitions = ParseExpressions<SQLColumnDefinition>(tokens, c, { rightParen }, ParseColumnDefinition);
			if (!columnDefinitions)
			{
				return Optional<CreateStatement>::from_error(columnDefinitions.get_error());
			}
			if (columnDefinitions->empty())
			{
				return Optional<CreateStatement>::from_error("Table requires at least 1 column");
			}
			// Expect closing parenthesis
			if (!CheckToken(tokens, c, TokenFromSymbol(SYMBOL_RIGHT_PAREN)))
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
		if (CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_SELECT)))
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
		else if (CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_INSERT)))
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
		else if (CheckToken(tokens, cursor, TokenFromKeyword(KEYWORD_CREATE)))
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

	// Remove comment tokens
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

	// Parse a list of SQL statements from a set of tokens
	ParseResult ParseSQL(const std::vector<Token>& tokens)
	{
		ParseResult result;
		uint32_t cursor = 0;
		while (cursor < tokens.size() && tokens[cursor].Type != TokenType::EndOfFile)
		{
			Optional<SQLStatement> statement = ParseStatement(tokens, cursor);
			if (!statement)
			{
				result.Error = statement.get_error();
				return result;
			}
			result.Statements.push_back(statement.value());

			// Support multiple redundant semi colons
			Token semicolon = TokenFromSymbol(SYMBOL_SEMICOLON);
			bool hasSemicolon = false;
			while (CheckToken(tokens, cursor, semicolon))
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
