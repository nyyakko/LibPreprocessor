#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(parsing_if_statement, missing_condition)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: An \"%IF\" statement didn't had a condition.");
}

TEST(parsing_if_statement, missing_condition_followed_by_content)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: \"%IF\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"INode::Type::CONTENT\".");
}

TEST(parsing_if_statement, missing_condition_followed_by_another_statement)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF\n"
        "    %IF [<TRUE>]:\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: \"%IF\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"INode::Type::STATEMENT\".");
}

TEST(parsing_if_statement, missing_colon)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE> AND <FALSE>]\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), true);
    EXPECT_STREQ(result.error().message().data(), "[LibPreprocessor::Runtime/error]: \"Context::Who::IF_STATEMENT\" expects a terminating \":\", instead got \"    hello!\".");
}

