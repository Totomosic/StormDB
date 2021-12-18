#pragma once
#include "Lexer.h"

namespace StormDB
{

	std::string ReconstructSource(const std::vector<Token>& tokens);

}
