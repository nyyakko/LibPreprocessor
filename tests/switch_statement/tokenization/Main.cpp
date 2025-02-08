#include <gtest/gtest.h>

#include <libpreprocessor/Processor.hpp>

TEST(tokenization_switch_statement, missing_body_end)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<TRUE>]:\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): An \"%SWITCH\" statement missing its \"%END\" was reached.");
}

TEST(tokenization_switch_statement_no_match, missing_angle_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<0>]:\n"
        "    %CASE [<1]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (2, 14): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
}

TEST(tokenization_switch_statement_no_match, missing_angle_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<0]:\n"
        "    %CASE [<1>]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
}

TEST(tokenization_switch_statement_no_match, missing_square_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<0>]:\n"
        "    %CASE [<1>:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (2, 15): Expected \"]\", but found \"Token::Type::COLON\" instead.");
}

TEST(tokenization_switch_statement_no_match, missing_square_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<0>:\n"
        "    %CASE [<1>]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 13): Expected \"]\", but found \"Token::Type::COLON\" instead.");
}

TEST(tokenization_switch_statement, missing_match_expression_without_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%SWITCH\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): An \"%SWITCH\" statement didn't had a expression to match.");
    }
    {
    auto static constexpr source =
        "%SWITCH\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): An \"%SWITCH\" statement didn't had a expression to match.");
    }
}

TEST(tokenization_switch_statement, missing_match_expression_with_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%SWITCH\n"
        "    %DEFAULT:\n"
        "        hello\n"
        "    %END\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): An \"%SWITCH\" statement didn't had a expression to match.");
    }
    {
    auto static constexpr source =
        "%SWITCH\n"
        "    %DEFAULT:\n"
        "        hello\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 12): An \"%SWITCH\" statement didn't had a expression to match.");
    }
}

TEST(tokenization_switch_statement, missing_case_match_expression)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "    %END\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (2, 12): An \"%CASE\" statement didn't had a expression to match.");
    }
    {
    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (2, 12): An \"%CASE\" statement didn't had a expression to match.");
    }
}

TEST(tokenization_switch_statement, missing_default_case_colon)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE [<69>]:\n"
        "    %END\n"
        "    %DEFAULT\n"
        "    %END\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (4, 18): An \"%DEFAULT\" statement didn't had a body.");
    }
    {
    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE [<69>]:\n"
        "    %END\n"
        "    %DEFAULT\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (4, 18): An \"%DEFAULT\" statement didn't had a body.");
    }
}

