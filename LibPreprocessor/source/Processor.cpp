#include "Processor.hpp"

#include "Lexer.hpp"
#include "Parser.hpp"

namespace libpreprocessor {

using namespace liberror;

Result<std::string> process(std::string_view source, PreprocessorContext const& context)
{
    Lexer lexer { source };
    Parser parser { lexer.tokenize() };
    return interpret(TRY(parser.parse()), context);
}

Result<std::string> process(std::filesystem::path path, PreprocessorContext const& context)
{
    Lexer lexer { path };
    Parser parser { lexer.tokenize() };
    return interpret(TRY(parser.parse()), context);
}

} // namespace libpreprocessor
