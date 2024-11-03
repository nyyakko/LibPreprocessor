#include "Preprocessor.hpp"

#include "core/Lexer.hpp"
#include "core/Parser.hpp"

namespace libpreprocessor {

using namespace liberror;

Maybe<std::string> preprocess(std::string_view source, PreprocessorContext const& context)
{
    Lexer lexer { source };
    Parser parser { lexer.tokenize() };
    return interpret(TRY(parser.parse()), context);
}

Maybe<std::string> preprocess(std::filesystem::path path, PreprocessorContext const& context)
{
    Lexer lexer { path };
    Parser parser { lexer.tokenize() };
    return interpret(TRY(parser.parse()), context);
}

}
