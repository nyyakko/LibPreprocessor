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

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
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

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
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

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \"\\]\", but found \"\\\\n\" instead\\.");
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

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \"\\]\", but found \"\\\\n\" instead\\.");
}

