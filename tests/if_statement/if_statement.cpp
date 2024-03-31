#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(true_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n");
}

TEST(false_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(true_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n");
}

TEST(false_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    how are you?\n");
}

TEST(true_nested_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n        hello!\n    hello!\n");
}

TEST(surrounded_true_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\nhello!\n");
}

TEST(surrounded_false_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\n");
}

TEST(surrounded_true_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\nhello!\n");
}

TEST(surrounded_false_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    how are you?\nhello!\n");
}

TEST(surrounded_true_nested_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\n        hello!\n    hello!\nhello!\n");
}

TEST(multiple_true_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"sv ;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n    hello!\n");
}

TEST(multiple_false_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(multiple_true_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"sv
        ;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n    hello!\n");
}

TEST(multiple_false_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    how are you?\n    how are you?\n");
}

TEST(multiple_true_nested_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "    hello!\n        hello!\n    hello!\n    hello!\n        hello!\n    hello!\n");
}

TEST(surrounded_multiple_true_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\nhello!\n    hello!\nhello!\n");
}

TEST(surrounded_multiple_false_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\nhello!\n");
}

TEST(surrounded_multiple_true_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\nhello!\n    hello!\nhello!\n");
}

TEST(surrounded_multiple_false_if_else_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"
        "%IF [<FALSE>]:\n"
        "    hello!\n"
        "%ELSE:\n"
        "    how are you?\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    how are you?\nhello!\n    how are you?\nhello!\n");
}

TEST(surrounded_multiple_true_nested_if_statement, single)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"
        "%IF [<TRUE>]:\n"
        "    hello!\n"
        "    %IF [<TRUE>]:\n"
        "        hello!\n"
        "    %END\n"
        "    hello!\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(result.has_error(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n    hello!\n        hello!\n    hello!\nhello!\n    hello!\n        hello!\n    hello!\nhello!\n");
}

