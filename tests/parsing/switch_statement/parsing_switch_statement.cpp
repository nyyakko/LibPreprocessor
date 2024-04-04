#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(parsing_switch_statement, missing_match_expression)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH\n"
        "    %DEFAULT:\n"
        "        hello\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: An \"%SWITCH\" statement didn't had a expression to match.");
}

TEST(parsing_switch_statement, missing_case_match_expression)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: An \"%CASE\" statement didn't had a expression to match.");
}

TEST(parsing_switch_statement, missing_default_case_colon)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [<69>]:\n"
        "    %CASE [<69>]:\n"
        "    %END\n"
        "    %DEFAULT\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: An \"%DEFAULT\" statement didn't had a body.");
}

