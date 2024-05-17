#pragma once

#include "Token.hpp"
#include "nodes/INode.hpp"

#include <liberror/ErrorOr.hpp>

#include <ranges>
#include <stack>
#include <vector>

namespace libpreprocessor {

class Parser
{
public:
    struct Context
    {
        enum class Who
        {
            BEGIN__,
            IF_STATEMENT,
            ELSE_STATEMENT,
            SWITCH_STATEMENT,
            CASE_STATEMENT,
            PRINT_STATEMENT,
            EXPRESSION,
            END__
        };

        constexpr char const* who_is_as_string() const noexcept;

        int64_t parent;
        int64_t child;
        Who whois;
    };

    explicit Parser(std::vector<Token> const& tokens)
    {
        for (auto&& token : tokens | std::views::reverse)
        {
            tokens_m.push(std::move(token));
        }
    }

    liberror::ErrorOr<std::unique_ptr<INode>> parse() { return this->parse({}); }
    liberror::ErrorOr<std::unique_ptr<INode>> parse(Context const& context);

    bool eof() const noexcept { return tokens_m.empty(); }
    Token const& peek() const { return tokens_m.top(); }
    Token take() { auto value = tokens_m.top(); tokens_m.pop(); return value; }
    std::stack<Token>& tokens() noexcept { return tokens_m; }

private:
    std::stack<Token> tokens_m {};
};

constexpr char const* Parser::Context::who_is_as_string() const noexcept
{
    switch (whois)
    {
    case Parser::Context::Who::IF_STATEMENT: return "Context::Who::IF_STATEMENT";
    case Parser::Context::Who::ELSE_STATEMENT: return "Context::Who::ELSE_STATEMENT";
    case Parser::Context::Who::SWITCH_STATEMENT: return "Context::Who::SWITCH_STATEMENT";
    case Parser::Context::Who::CASE_STATEMENT: return "Context::Who::CASE_STATEMENT";
    case Parser::Context::Who::EXPRESSION: return "Context::Who::EXPRESSION";
    case Parser::Context::Who::PRINT_STATEMENT: return "Context::Who::PRINT_STATEMENT";

    case Parser::Context::Who::BEGIN__:
    case Parser::Context::Who::END__: {
        break;
    }
    }

    return "UNKNOWN";
}

} // libpreprocessor

