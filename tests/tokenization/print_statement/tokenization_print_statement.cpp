#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(tokenization_print_statement, missing_closing_square_bracket)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source = "%PRINT [<hello!>\n"sv;
    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \"\\]\", but found \"\\\\n\" instead\\.");
}

TEST(tokenization_print_statement, missing_closing_angle_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .localVariables = {},
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [<<ENV:TEST>]\n"sv;
    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
}

TEST(tokenization_print_statement, missing_closing_square_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .localVariables = {},
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [<TEST: <ENV:TEST>]\n"sv;
    EXPECT_DEATH((void)libpreprocessor::preprocess(source, context), "Aborted execution because: \\[LibPreprocessor::Runtime/error\\]: Expected \">\", but found \"\\]\" instead\\.");
}

