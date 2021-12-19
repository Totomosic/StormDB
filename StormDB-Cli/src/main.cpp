#include "StormDB.h"

int main()
{
    StormDB::Init();

    char buffer[8192];
    while (true)
    {
        std::cin.getline(buffer, sizeof(buffer));
        StormDB::LexResult result = StormDB::LexSQL<true>(buffer);

        if (!result.Errors.empty())
        {
            std::cout << "Errors" << std::endl;
            for (const StormDB::LexError& error : result.Errors)
                std::cout << StormDB::FormatLexError(error) << std::endl;
        }
        else
        {
            std::cout << "Tokens" << std::endl;
            for (const StormDB::Token& token : result.Tokens)
                std::cout << StormDB::FormatToken(token) << std::endl;
            std::cout << "Reconstructed" << std::endl;
            std::cout << StormDB::ReconstructSource(result.Tokens) << std::endl;

            StormDB::ParseResult parsed = StormDB::ParseSQL(FilterComments(result.Tokens));
            if (!parsed.Error.empty())
            {
                std::cout << parsed.Error << std::endl;
            }
            else
            {
                std::cout << "Success" << std::endl;
            }
        }
    }

    return 0;
}
