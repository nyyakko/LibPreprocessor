#include <gtest/gtest.h>

#include <libpreprocessor/Processor.hpp>

TEST(tokenization_print_statement, missing_closing_square_bracket)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source = "%PRINT [<hello!>\n"sv;
    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 8): Expected \"]\", but found \"EOF\" instead.");
}

TEST(tokenization_print_statement, missing_closing_angle_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [<ENV:TEST]\n"sv;
    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 18): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
}

TEST(tokenization_print_statement, missing_closing_angle_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [<TEST: ENV:TEST]\n"sv;
    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 24): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
}

TEST(tokenization_print_statement, stray_colon_token)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%PRINT [<hello!>]:\n"
        "\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 18): A stray token of type \"Token::Type::COLON\" was reached.");
    }
    {
    auto static constexpr source =
        "%PRINT [<hello!>]:\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 18): A stray token of type \"Token::Type::COLON\" was reached.");
    }
}

