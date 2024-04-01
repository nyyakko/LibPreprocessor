#pragma once

#include <memory>
#include <list>

#define NODE_TYPE(TYPE)                                                         \
constexpr virtual Type type() const override { return TYPE; }                   \
constexpr virtual char const* type_as_string() const override { return #TYPE; } \

namespace libpreprocessor {

struct INode
{
    enum class Type
    {
        START__,
        STATEMENT,
        EXPRESSION,
        CONTENT,
        CONDITION,
        OPERATOR,
        LITERAL,
        BODY,
        END__
    };

    virtual ~INode() = default;

    constexpr virtual Type type() const = 0;
    constexpr virtual char const* type_as_string() const = 0;

    std::list<std::unique_ptr<INode>> nodes {};
};

} // libpreprocessor
