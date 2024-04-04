#include "core/Lexer.hpp"

#include <functional>
#include <algorithm>

namespace libpreprocessor {

using namespace liberror;

    namespace {

    bool is_newline(char value) { return value == '\n'; }
    bool is_space(char value) { return value == ' '; }

    void drop_while(Lexer& lexer, std::function<bool(char)> const& predicate)
    {
        while (!lexer.eof() && predicate(lexer.peek()))
        {
            lexer.take();
        }
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
            return std::format("{}", byte);
        };

        if (lexer.peek() != expected)
        {
            return make_error(PREFIX_ERROR": Expected \"{}\", but found \"{}\" instead.", expected, fnUnscaped(lexer.peek()));
        }

        return {};
    }

    void tokenize_content(Lexer& lexer, std::vector<Token>& tokens)
    {
        while (!lexer.eof() && lexer.peek() != '%')
        {
            Token token {};

            do
            {
                token.data += lexer.take();
            } while (!lexer.eof() && lexer.peek() != '%' && !(is_newline(token.data.front()) || is_newline(lexer.peek())));

            if (is_newline(lexer.peek())) { lexer.take(); }
            if (std::ranges::all_of(token.data, is_space)) { return; }

            token.type  = Token::Type::CONTENT;
            token.begin = lexer.cursor() - token.data.size();
            token.end   = lexer.cursor();
            tokens.push_back(std::move(token));
        }
    }

    void tokenize_keyword(Lexer& lexer, std::vector<Token>& tokens)
    {
        Token token {};

        while (!lexer.eof() && lexer.peek() != ':' && !(is_space(lexer.peek()) || is_newline(lexer.peek())))
        {
            token.data += lexer.take();
        }

        token.type  = Token::Type::KEYWORD;
        token.begin = lexer.cursor() - token.data.size();
        token.end   = lexer.cursor();
        tokens.push_back(std::move(token));

        if (is_newline(lexer.peek()) || is_space(lexer.peek()))
        {
            lexer.take();
        }
    }

    ErrorOr<void> tokenize_literal(Lexer& lexer, std::vector<Token>& tokens)
    {
        std::function<std::string()> fnBuildLiteral {};

        fnBuildLiteral = [&] () -> std::string {
            std::string value {};

            while (!lexer.eof() && lexer.peek() != '>' && lexer.peek() != ']')
            {
                if (lexer.peek() == '<')
                {
                    value += lexer.take();
                    value += fnBuildLiteral();
                }

                value += lexer.take();
            }

            return value;
        };

        Token token {};

        token.data  = fnBuildLiteral();
        token.begin = lexer.cursor() - token.data.size();
        token.end   = lexer.cursor();
        token.type  = [&] {
            if ((token.data.starts_with("<") && token.data.ends_with(">") && token.data.contains(':')) &&
                !(token.data.starts_with("<<") || token.data.ends_with(">>")))
                return Token::Type::IDENTIFIER;
            return Token::Type::LITERAL;
        }();

        tokens.push_back(std::move(token));

        return {};
    }

    ErrorOr<void> tokenize_expression(Lexer& lexer, std::vector<Token>& tokens, size_t depth = 0)
    {
        while (!lexer.eof())
        {
            drop_while(lexer, is_space);

            switch (lexer.peek())
            {
            case '[': {
                Token token {};
                token.type  = Token::Type::LEFT_SQUARE_BRACKET;
                token.data += lexer.take();
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
                token.data += lexer.take();
                token.begin = lexer.cursor() - token.data.size();
                token.end   = lexer.cursor();
                tokens.push_back(std::move(token));
                return {};
            }
            case '<': {
                Token token {};
                token.type  = Token::Type::LEFT_ANGLE_BRACKET;
                token.data += lexer.take();
                token.begin = lexer.cursor() - token.data.size();
                token.end   = lexer.cursor();
                tokens.push_back(std::move(token));
                TRY(tokenize_literal(lexer, tokens));
                MUST(expect_to_peek(lexer, '>'));
                break;
            }
            case '>': {
                Token token {};
                token.type  = Token::Type::RIGHT_ANGLE_BRACKET;
                token.data += lexer.take();
                token.begin = lexer.cursor() - token.data.size();
                token.end   = lexer.cursor();
                tokens.push_back(std::move(token));
                break;
            }
            default: {
                Token token {};
                token.type = Token::Type::IDENTIFIER;

                while (!lexer.eof() && !is_space(lexer.peek()))
                {
                    token.data += lexer.take();
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
        switch (peek())
        {
        case '%': {
            drop_while(*this, is_space);
            Token token {};
            token.type  = Token::Type::PERCENT;
            token.data += take();
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            tokenize_keyword(*this, tokens.value());
            break;
        }
        case '[': {
            drop_while(*this, is_space);
            Token token {};
            token.type  = Token::Type::LEFT_SQUARE_BRACKET;
            token.data += take();
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            TRY(tokenize_expression(*this, tokens.value()));
            MUST(expect_to_peek(*this, ']'));
            break;
        }
        case ']': {
            drop_while(*this, is_space);
            Token token {};
            token.type  = Token::Type::RIGHT_SQUARE_BRACKET;
            token.data += take();
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));
            drop_while(*this, is_newline);
            break;
        }
        case ':': {
            drop_while(*this, is_space);
            Token token {};
            token.type  = Token::Type::COLON;
            token.data += take();
            token.begin = cursor_m - token.data.size();
            token.end   = cursor_m;
            tokens.value().push_back(std::move(token));

            take();

            auto const spacesCount = [&] {
                auto count = 0zu;
                while (std::isspace(peek())) { count += 1; take(); }
                return count;
            }();

            if (peek() != '%')
            {
                for ([[maybe_unused]]auto _ : std::views::iota(0zu, spacesCount)) TRY(untake());
            }

            break;
        }
        case ' ': {
            auto const spacesCount = [&] {
                auto count = 0zu;
                while (peek() != '<' && !std::isalpha(peek())) { count += 1; take(); }
                return count;
            }();

            if (peek() == '<' && TRY(peek_next()) == '<')
            {
                take(); take();
                MUST(expect_to_peek(*this, ' '));
                take();
            }
            else
            {
                for ([[maybe_unused]]auto _ : std::views::iota(0zu, spacesCount)) TRY(untake());
                tokenize_content(*this, tokens.value());
            }

            break;
        }
        default: {
            tokenize_content(*this, tokens.value());
            break;
        }
        }
    }

    return tokens;
}

}
