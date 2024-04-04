#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

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

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \">\", but found \"]\" instead.");
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

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \">\", but found \"]\" instead.");
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

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \"]\", but found \":\" instead.");
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

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \"]\", but found \":\" instead.");
}

