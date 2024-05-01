#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(tokenization_if_statement, missing_body_end)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: An \"%IF\" statement missing its \"%END\" was reached.");
}

TEST(tokenization_if_statement, missing_closing_angle_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE]:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \">\", but found \"]\" instead.");
}

TEST(tokenization_if_statement, missing_closing_angle_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE]:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \">\", but found \"]\" instead.");
}

TEST(tokenization_if_statement, missing_closing_square_bracket_1)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \"]\", but found \":\" instead.");
}

TEST(tokenization_if_statement, missing_closing_square_bracket_2)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE>:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Expected \"]\", but found \":\" instead.");
}

