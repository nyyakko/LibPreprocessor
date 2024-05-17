#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

TEST(multiple_switch_statement_no_match, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(multiple_switch_statement_no_match, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(surrounded_multiple_switch_statement_no_match, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\nhello!\n");
}

TEST(surrounded_multiple_switch_statement_no_match, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\nhello!\n");
}

TEST(switch_statement_no_match, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(switch_statement_no_match, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "");
}

TEST(surrounded_switch_statement_no_match, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\n");
}

TEST(surrounded_switch_statement_no_match, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(0)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\nhello!\n");
}

TEST(switch_statement, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        hello!\n");
}

TEST(switch_statement, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n");
}

TEST(switch_statement, single_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n");
}

TEST(switch_statement, multiple_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n");
}

TEST(multiple_switch_statement, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        hello!\n        hello!\n");
}

TEST(multiple_switch_statement, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n        how are you?\n");
}

TEST(multiple_switch_statement, single_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n        how are you?\n");
}

TEST(multiple_switch_statement, multiple_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "        how are you?\n        how are you?\n");
}

TEST(surrounded_switch_statement, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        hello!\nhello!\n");
}

TEST(surrounded_switch_statement, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n");
}

TEST(surrounded_switch_statement, single_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n");
}

TEST(surrounded_switch_statement, multiple_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n");
}

TEST(surrounded_multiple_switch_statement, single_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(1)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        hello!\nhello!\n        hello!\nhello!\n");
}

TEST(surrounded_multiple_switch_statement, multiple_case_no_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n        how are you?\nhello!\n");
}

TEST(surrounded_multiple_switch_statement, single_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        how are you?\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n        how are you?\nhello!\n");
}

TEST(surrounded_multiple_switch_statement, multiple_case_default)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source =
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"
        "%SWITCH [(2)]:\n"
        "    %CASE [(1)]:\n"
        "        hello!\n"
        "    %END\n"
        "    %CASE [(2)]:\n"
        "        how are you?\n"
        "    %END\n"
        "    %DEFAULT:\n"
        "        im fine, thank you!\n"
        "    %END\n"
        "%END\n"
        "hello!\n"sv;

    auto const result = libpreprocessor::preprocess(source, context);
    EXPECT_EQ(!result.has_value(), false);
    EXPECT_STREQ(result.value().data(), "hello!\n        how are you?\nhello!\n        how are you?\nhello!\n");
}

