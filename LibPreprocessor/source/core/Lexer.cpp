#include "core/Lexer.hpp"

#include <fmt/format.h>

#include <functional>
#include <algorithm>

namespace libpreprocessor {

using namespace liberror;

namespace {

bool is_newline(char value) { return value == '\n'; }
bool is_space(char value) { return value == ' '; }
bool is_percent(char value) { return value == '%'; }
bool is_left_angle_bracket(char value) { return value == '<'; }
bool is_right_angle_bracket(char value) { return value == '>'; }
bool is_right_square_bracket(char value) { return value == ']'; }
bool is_colon(char value) { return value == ':'; }

ErrorOr<void> drop_while(Lexer& lexer, std::function<bool(char)> const& predicate)
{
    while (!lexer.eof() && predicate(TRY(lexer.peek())))
    {
        TRY(lexer.take());
    }

    return {};
}

ErrorOr<void> expect_to_peek(Lexer& lexer, char expected)
{
    auto fnUnscaped = [] (char byte) -> std::string {
        switch (byte)
        {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\r': return "\\r";
        case '\0': return "\\0";
        }
        return fmt::format("{}", byte);
    };

    if (lexer.eof())
        return make_error(PREFIX_ERROR": Expected \"{}\", but found \"EOF\" instead.", expected);
    if (TRY(lexer.peek()) != expected)
        return make_error(PREFIX_ERROR": Expected \"{}\", but found \"{}\" instead.", expected, fnUnscaped(TRY(lexer.peek())));

    return {};
}

ErrorOr<Token> tokenize_content(Lexer& lexer)
{
    ErrorOr<Token> token {};

    do
    {
        token.value().data += TRY(lexer.take());
    } while (!lexer.eof() && TRY(lexer.peek()) != '%' && !(is_newline(token.value().data.front()) || is_newline(TRY(lexer.peek()))));

    if (!lexer.eof() && is_newline(TRY(lexer.peek()))) { TRY(lexer.take()); }

    token.value().type  = Token::Type::CONTENT;
    token.value().begin = lexer.cursor() - token.value().data.size();
    token.value().end   = lexer.cursor();
    return token;
}

ErrorOr<Token> tokenize_keyword(Lexer& lexer)
{
    ErrorOr<Token> token {};

    while (!lexer.eof() && TRY(lexer.peek()) != ':' && !(is_space(TRY(lexer.peek())) || is_newline(TRY(lexer.peek()))))
    {
        token.value().data += TRY(lexer.take());
    }

    token.value().type  = Token::Type::STATEMENT;
    token.value().begin = lexer.cursor() - token.value().data.size();
    token.value().end   = lexer.cursor();

    if (!lexer.eof() && (is_newline(TRY(lexer.peek())) || is_space(TRY(lexer.peek()))))
    {
        TRY(lexer.take());
    }

    return token;
}

ErrorOr<std::string> build_literal(Lexer& lexer)
{
    std::string value {};

    while (!lexer.eof() && !is_right_angle_bracket(TRY(lexer.peek())) && !is_right_square_bracket(TRY(lexer.peek())))
    {
        if (is_left_angle_bracket(TRY(lexer.peek())))
        {
            value += TRY(lexer.take());
            value += TRY(build_literal(lexer));
        }

        value += TRY(lexer.take());
    }

    return value;
}

ErrorOr<Token> tokenize_literal(Lexer& lexer)
{
    ErrorOr<Token> token {};

    token.value().data  = TRY(build_literal(lexer));
    token.value().begin = lexer.cursor() - token.value().data.size();
    token.value().end   = lexer.cursor();
    token.value().type  = [&] {
        if ((token.value().data.starts_with("<") && token.value().data.ends_with(">") && token.value().data.contains(':')) &&
            !(token.value().data.starts_with("<<") || token.value().data.ends_with(">>")))
            return Token::Type::IDENTIFIER;
        return Token::Type::LITERAL;
    }();

    return token;
}

ErrorOr<void> tokenize_expression(Lexer& lexer, std::vector<Token>& tokens, size_t depth = 0)
{
    while (!lexer.eof() && !(is_colon(TRY(lexer.peek())) || is_newline(TRY(lexer.peek()))))
    {
        TRY(drop_while(lexer, is_space));

        switch (TRY(lexer.peek()))
        {
        case '[': {
            Token token {};
            token.type  = Token::Type::LEFT_SQUARE_BRACKET;
            token.data += TRY(lexer.take());
            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
            TRY(tokenize_expression(lexer, tokens, depth + 1));
            break;
        }
        case ']': {
            if (depth == 0) return {};
            Token token {};
            token.type  = Token::Type::RIGHT_SQUARE_BRACKET;
            token.data += TRY(lexer.take());
            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
            return {};
        }
        case '<': {
            Token token {};
            token.type  = Token::Type::LEFT_ANGLE_BRACKET;
            token.data += TRY(lexer.take());
            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
            tokens.push_back(TRY(tokenize_literal(lexer)));
            TRY(expect_to_peek(lexer, '>'));
            break;
        }
        case '>': {
            Token token {};
            token.type  = Token::Type::RIGHT_ANGLE_BRACKET;
            token.data += TRY(lexer.take());
            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
            break;
        }
        default: {
            Token token {};
            token.type = Token::Type::IDENTIFIER;

            while (!lexer.eof() && !is_space(TRY(lexer.peek())))
            {
                token.data += TRY(lexer.take());
            }

            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
            break;
        }
        }
    }

    return {};
}

}

ErrorOr<std::vector<Token>> Lexer::tokenize()
{
    ErrorOr<std::vector<Token>> tokens {};

    while (!eof())
    {
        switch (TRY(peek()))
        {
        case '%': {
            TRY(drop_while(*this, is_space));
            Token token {};
            token.type  = Token::Type::PERCENT;
            token.data += TRY(take());
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            tokens.value().push_back(TRY(tokenize_keyword(*this)));
            break;
        }
        case '[': {
            TRY(drop_while(*this, is_space));
            Token token {};
            token.type  = Token::Type::LEFT_SQUARE_BRACKET;
            token.data += TRY(take());
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            TRY(tokenize_expression(*this, tokens.value()));
            TRY(expect_to_peek(*this, ']'));
            break;
        }
        case ']': {
            TRY(drop_while(*this, is_space));
            Token token {};
            token.type  = Token::Type::RIGHT_SQUARE_BRACKET;
            token.data += TRY(take());
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            TRY(drop_while(*this, is_newline));
            break;
        }
        case ':': {
            TRY(drop_while(*this, is_space));
            Token token {};
            token.type  = Token::Type::COLON;
            token.data += TRY(take());
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));

            TRY(take());

            auto const spacesCount = TRY([&] -> ErrorOr<size_t> {
                auto count = 0zu;
                while (!eof() && is_space(TRY(peek())))
                {
                    count += 1;
                    TRY(take());
                }
                return count;
            }());

            if (!eof() && !is_percent(TRY(peek())))
            {
                for ([[maybe_unused]]auto _ : std::views::iota(0zu, spacesCount)) TRY(untake());
            }

            break;
        }
        case ' ': {
            auto const spacesCount = TRY([&] -> ErrorOr<size_t> {
                auto count = 0zu;
                while (!eof() && !is_left_angle_bracket(TRY(peek())) && !std::isalpha(TRY(peek())))
                {
                    count += 1;
                    TRY(take());
                }
                return count;
            }());

            if (!eof() && is_left_angle_bracket(TRY(peek())) && is_left_angle_bracket(TRY(peek_next())))
            {
                TRY(take());
                TRY(take());
                TRY(expect_to_peek(*this, ' '));
                TRY(take());
            }
            else
            {
                for ([[maybe_unused]]auto _ : std::views::iota(0zu, spacesCount)) TRY(untake());
                while (!eof() && !is_percent(TRY(peek())))
                {
                    auto token = TRY(tokenize_content(*this));
                    if (std::ranges::all_of(token.data, is_space)) continue;
                    tokens.value().push_back(std::move(token));
                }
            }

            break;
        }
        default: {
            while (!eof() && !is_percent(TRY(peek())))
            {
                auto token = TRY(tokenize_content(*this));
                if (std::ranges::all_of(token.data, is_space)) continue;
                tokens.value().push_back(std::move(token));
            }
            break;
        }
        }
    }

    return tokens;
}

}

