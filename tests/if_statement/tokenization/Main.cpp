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
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): An \"%IF\" statement missing its \"%END\" was reached.");
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
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 11): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
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
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 23): Expected \">\", but found \"Token::Type::RIGHT_SQUARE_BRACKET\" instead.");
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
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 12): Expected \"]\", but found \"Token::Type::COLON\" instead.");
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
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 24): Expected \"]\", but found \"Token::Type::COLON\" instead.");
}

TEST(tokenization_if_statement, missing_condition)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%IF\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): An \"%IF\" statement didn't had a condition.");
    }
    {
    auto static constexpr source =
        "%IF\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): An \"%IF\" statement didn't had a condition.");
    }
}

TEST(tokenization_if_statement, missing_condition_followed_by_content)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%IF\n"
        "    hello!\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): \"%IF\" statement expects an \"INode::Type::EXPRESSION\", instead got \"INode::Type::CONTENT\".");
    }
    {
    auto static constexpr source =
        "%IF\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): \"%IF\" statement expects an \"INode::Type::EXPRESSION\", instead got \"INode::Type::CONTENT\".");
    }
}

TEST(tokenization_if_statement, missing_condition_followed_by_another_statement)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%IF\n"
        "    %IF [<TRUE>]:\n"
        "    %END\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): \"%IF\" statement expects an \"INode::Type::EXPRESSION\", instead got \"INode::Type::STATEMENT\".");
    }
    {
    auto static constexpr source =
        "%IF\n"
        "    %IF [<TRUE>]:\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (0, 4): \"%IF\" statement expects an \"INode::Type::EXPRESSION\", instead got \"INode::Type::STATEMENT\".");
    }
}

TEST(tokenization_if_statement, missing_colon)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    {
    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE>]\n"
        "    hello!\n"
        "%END\n"
        "\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 19): Expected \":\", but found \"Token::Type::CONTENT\" instead.");
    }
    {
    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE>]\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: Local/Global Variable: (1, 19): Expected \":\", but found \"Token::Type::CONTENT\" instead.");
    }
}

