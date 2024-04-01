#pragma once

#include "Token.hpp"
#include "nodes/INode.hpp"

#include <liberror/ErrorOr.hpp>
#include <liberror/types/TraceError.hpp>

#include <vector>
#include <stack>

namespace libpreprocessor {

class Parser
{
public:
    explicit Parser(std::vector<Token> const& tokens)
        : tokens_m { tokens | std::views::reverse | std::ranges::to<std::stack>() }
    {}

    liberror::ErrorOr<std::unique_ptr<INode>> parse() { return this->parse(0, 0); }
    liberror::ErrorOr<std::unique_ptr<INode>> parse(int64_t parent, int64_t child);

    bool eof() const noexcept { return tokens_m.empty(); }
    Token const& peek() { return tokens_m.top(); }
    Token take() { auto value = tokens_m.top(); tokens_m.pop(); return value; }
    std::stack<Token>& tokens() noexcept { return tokens_m; }

private:
    std::stack<Token> tokens_m {};
};

} // libpreprocessor

