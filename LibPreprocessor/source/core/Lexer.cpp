#include "core/Lexer.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <ranges>

namespace libpreprocessor {

using namespace liberror;

namespace internal {

static std::vector<std::string> split_string(std::string_view string, std::string_view separator);
static std::string read_file_contents(std::filesystem::path file);

}

static constexpr std::string_view unknown_file_location_g = "Local/Global Variable";

static constexpr std::array special_g  { '%', '[', '<', '>', ']', ':', ' ' };
static constexpr std::array keyword_g  { "IF", "END", "ELSE", "SWITCH", "CASE", "DEFAULT", "PRINT" };
static constexpr std::array operator_g { "AND", "OR", "NOT", "EQUALS", "CONTAINS" };

Lexer::Lexer(std::string_view source)
    : _source(internal::split_string(source, "\n"))
    , _cursor({})
    , _tokens({})
{
}

Lexer::Lexer(std::filesystem::path file)
    : _file(file)
    , _source(internal::split_string(internal::read_file_contents(file), "\n"))
    , _cursor({})
    , _tokens({})
{
}

std::vector<Token> Lexer::tokenize()
{
    for (; _cursor.first < _source.size(); _cursor.first += 1)
    {
        while (!eof())
        {
            auto const skipped = skip_spaces(*this);
            auto token    = *next_special()
                    .or_else([this] () { return next_keyword(); })
                    .or_else([this] () { return next_operator(); })
                    .or_else([this] () { return next_identifier(); })
                    .or_else([this] () { return next_literal(); })
                    .or_else([this, skipped] () {
                        _cursor.second -= skipped;
                        return next_content();
                    });

            token.location = {
                { _file.empty() ? unknown_file_location_g : _file.string() },
                { _cursor.first + 1, _cursor.second + token.data.size() - 1 }
            };

            _tokens.emplace_back(std::move(token));
        }

        _cursor.second = 0;
    }

    return _tokens;
}

std::optional<Token> Lexer::next_special()
{
    if (std::ranges::find(special_g, MUST(peek())) == special_g.end()) return std::nullopt;

    std::optional<Token> token { Token {} };

    token->data = fmt::format("{}", MUST(take()));

    switch (token->data.front())
    {
    case '%': token->type = Token::Type::PERCENT; break;
    case '[': token->type = Token::Type::LEFT_SQUARE_BRACKET; break;
    case '<': token->type = Token::Type::LEFT_ANGLE_BRACKET; break;
    case '>': token->type = Token::Type::RIGHT_ANGLE_BRACKET; break;
    case ']': token->type = Token::Type::RIGHT_SQUARE_BRACKET; break;
    case ':': token->type = Token::Type::COLON; break;
    }

    return token;
}

std::optional<Token> Lexer::next_keyword()
{
    std::optional<Token> token { Token {} };

    while (!eof() && std::ranges::find(special_g, MUST(peek())) == special_g.end())
    {
        token->data += MUST(take());
    }

    token->type = Token::Type::KEYWORD;

    if (std::ranges::find(keyword_g, token->data) == keyword_g.end())
    {
        _cursor.second -= token->data.size();
        return std::nullopt;
    }

    return token;
}

std::optional<Token> Lexer::next_identifier()
{
    if (_tokens.empty() || _tokens.back().type != Token::Type::LEFT_ANGLE_BRACKET) return std::nullopt;

    std::optional<Token> token { Token {} };

    while (!eof() && MUST(peek()) != '>' && MUST(peek()) != ']')
    {
        token->data += MUST(take());
    }

    token->type = Token::Type::IDENTIFIER;

    if (std::size(internal::split_string(token->data, ":")) != 2)
    {
        _cursor.second -= token->data.size();
        return std::nullopt;
    }

    return token;
}

std::optional<Token> Lexer::next_operator()
{
    std::optional<Token> token { Token {} };

    while (!eof() && MUST(peek()) != ' ')
    {
        token->data += MUST(take());
    }

    token->type = Token::Type::OPERATOR;

    if (std::ranges::find(operator_g, token->data) == operator_g.end())
    {
        _cursor.second -= token->data.size();
        return std::nullopt;
    }

    return token;
}

std::optional<Token> Lexer::next_literal()
{
    if (_tokens.empty() || _tokens.back().type != Token::Type::LEFT_ANGLE_BRACKET) return std::nullopt;

    std::optional<Token> token { Token {} };

    while (!eof() && MUST(peek()) != '>' && MUST(peek()) != ']')
    {
        token->data += MUST(take());
    }

    token->type = Token::Type::LITERAL;

    return token;
}

std::optional<Token> Lexer::next_content()
{
    std::optional<Token> token { Token {} };

    while (!eof())
    {
        token->data += MUST(take());
    }

    auto const justifyCount = std::ranges::count(token->data, '@');

    if (justifyCount)
    {
        // FIXME: find a better way to handle this
        token->data = (token->data | std::views::drop(justifyCount * 4 + justifyCount + 1)).data();
    }

    token->type = Token::Type::CONTENT;

    return token;
}

}

namespace libpreprocessor::internal {

static std::vector<std::string> split_string(std::string_view string, std::string_view separator)
{
    std::vector<std::string> result {};
    std::ranges::copy(
        string
            | std::views::split(separator)
            | std::views::filter([] (auto&& element) { return std::strlen(element.data()); })
            | std::views::transform([separator] (auto&& element) -> std::string {
                  auto const begin    = element.data();
                  auto const [end, _] = std::ranges::search(
                          element.data(), element.data() + std::strlen(element.data()), separator.begin(), separator.end());
                  std::string result {};
                  std::ranges::copy(begin, end, std::back_inserter(result));
                  return result.empty() ? "\n" : result;
              }),
        std::back_inserter(result)
    );
    return result;
}

static std::string read_file_contents(std::filesystem::path file)
{
    std::ifstream inputStream { file };
    std::stringstream contentStream {};
    contentStream << inputStream.rdbuf();
    return contentStream.str();
}

}

