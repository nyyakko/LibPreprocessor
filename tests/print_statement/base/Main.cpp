#include <gtest/gtest.h>

#include <libpreprocessor/Preprocessor.hpp>

#include <stdio.h>
#include <array>

static constexpr auto BUFFER_SIZE = 1024;

int redirect_stdout_to_buffer(std::array<char, BUFFER_SIZE>& buffer)
{
    fflush(stdout);
    auto previousState = dup(fileno(stdout));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    (void)freopen("NUL", "a", stdout);
#pragma clang diagnostic pop
    setvbuf(stdout, buffer.data(), _IOFBF, buffer.size());

    return previousState;
}

void restore_stdout(int state)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    (void)freopen("NUL", "a", stdout);
#pragma clang diagnostic pop
    dup2(state, fileno(stdout));
    setvbuf(stdout, NULL, _IONBF, BUFFER_SIZE);
}

TEST(print_statement, simple)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {};

    auto static constexpr source = "%PRINT [(hello!)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "hello!\n");
}

TEST(print_statement, single_string_interpolation)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(<ENV:TEST>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "TESTING\n");
}

TEST(print_statement, normal_with_single_string_interpolation)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(TEST: <ENV:TEST>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "TEST: TESTING\n");
}

TEST(print_statement, single_string_interpolation_ignore)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(<<ENV:TEST>>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "<ENV:TEST>\n");
}

TEST(print_statement, normal_with_single_string_interpolation_ignored)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(TEST: <<ENV:TEST>>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "TEST: <ENV:TEST>\n");
}

TEST(print_statement, multiple_interpolated_string)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(<ENV:TEST> is <ENV:TEST>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "TESTING is TESTING\n");
}

TEST(print_statement, multiple_interpolated_string_with_one_ignored)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(<<ENV:TEST>> is <ENV:TEST>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "<ENV:TEST> is TESTING\n");
}

TEST(print_statement, multiple_interpolated_string_with_all_ignored)
{
    using namespace std::literals;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:TEST", "TESTING" }
        }
    };

    auto static constexpr source = "%PRINT [(<<ENV:TEST>> is <<ENV:TEST>>)]\n"sv;

    std::array<char, BUFFER_SIZE> buffer {};
    auto const previousState = redirect_stdout_to_buffer(buffer);
    auto const result = libpreprocessor::preprocess(source, context);
    restore_stdout(previousState);

    EXPECT_EQ(!result.has_value(), false);

    EXPECT_STREQ(buffer.data(), "<ENV:TEST> is <ENV:TEST>\n");
}

