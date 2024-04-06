#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(parsing_print_statement, stray_colon_token)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%PRINT [<hello!>]:\n"
        "\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: A stray token of type \"Token::Type::COLON\" was reached.");
    }
    {
    auto static constexpr source =
        "%PRINT [<hello!>]:\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: A stray token of type \"Token::Type::COLON\" was reached.");
    }
}

