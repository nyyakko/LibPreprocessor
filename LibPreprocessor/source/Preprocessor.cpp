#include "Preprocessor.hpp"

#include "core/Lexer.hpp"
#include "core/Parser.hpp"

#include <fstream>
#include <sstream>

namespace libpreprocessor {

using namespace liberror;

ErrorOr<std::string> preprocess(std::string_view source, PreprocessorContext const& context)
{
    Lexer lexer { source };
    Parser parser { TRY(lexer.tokenize()) };
    auto const ast = TRY(parser.parse());
    return TRY(traverse(ast, context));
}

ErrorOr<std::string> preprocess(std::filesystem::path path, PreprocessorContext const& context)
{
    std::ifstream inputStream { path };
    std::stringstream contentStream {};
    contentStream << inputStream.rdbuf();

    return TRY(preprocess(std::string_view(contentStream.str()), context));
}

}
