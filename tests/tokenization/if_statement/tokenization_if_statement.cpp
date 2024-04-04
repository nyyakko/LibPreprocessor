#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(tokenization_true_if_statement, missing_closing_angle_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE]:\n"
        "    hello!\n"
        "%END\n"sv;

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
}

TEST(tokenization_true_if_statement, missing_closing_angle_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE]:\n"
        "    hello!\n"
        "%END\n"sv;

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
}

TEST(tokenization_true_if_statement, missing_closing_square_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>:\n"
        "    hello!\n"
        "%END\n"sv;

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \"\\]\", but found \":\" instead\\.");
}

TEST(tokenization_true_if_statement, missing_closing_square_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE>:\n"
        "    hello!\n"
        "%END\n"sv;

    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \"\\]\", but found \":\" instead\\.");
}

