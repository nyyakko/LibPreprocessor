#pragma once

#include "Token.hpp"
#include "nodes/Nodes.hpp"

#include <liberror/ErrorOr.hpp>

#include <optional>

namespace libpreprocessor {

static constexpr std::array special_g  { '%', '[', '<', '>', ']', ':', ' ' };
static constexpr std::array keyword_g  { "IF", "END", "ELSE", "SWITCH", "CASE", "DEFAULT", "PRINT" };
static constexpr std::array operator_g {
    std::pair { "AND"      , OperatorNode::Arity::BINARY },
    std::pair { "CONTAINS" , OperatorNode::Arity::BINARY },
    std::pair { "EQUALS"   , OperatorNode::Arity::BINARY },
    std::pair { "OR"       , OperatorNode::Arity::BINARY },
    std::pair { "NOT"      , OperatorNode::Arity::UNARY  },
};

class Lexer
{
public:
    explicit Lexer(std::string_view source);
    explicit Lexer(std::filesystem::path file);

    // FIXME: should find a way to make this an erroable function.
    std::vector<Token> tokenize();

private:

    friend size_t skip_spaces(Lexer&);

    bool eof() const
    {
        return _cursor.second >= _source.at(_cursor.first).size();
    }

    liberror::ErrorOr<char> peek() const
    {
        if (eof()) return liberror::make_error("End of file reached");
        return _source.at(_cursor.first).at(_cursor.second);
    }

    liberror::ErrorOr<char> take()
    {
        if (eof()) return liberror::make_error("End of file reached");
        return _source.at(_cursor.first).at(_cursor.second++);
    }

    std::optional<Token> next_special();
    std::optional<Token> next_keyword();
    std::optional<Token> next_operator();
    std::optional<Token> next_literal();
    std::optional<Token> next_identifier();
    std::optional<Token> next_content();

    std::filesystem::path _file;
    std::vector<std::string> _source;
    std::pair<size_t, size_t> _cursor;
    std::vector<Token> _tokens;
};

inline size_t skip_spaces(Lexer& lexer)
{
    size_t count = 0;
    for (; MUST(lexer.peek()) == ' '; count += 1)
        MUST(lexer.take());
    return count;
}

} // libpreprocessor

