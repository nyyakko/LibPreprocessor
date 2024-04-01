#pragma once

#include <memory>
#include <list>

#define NODE_TYPE(TYPE)                                                   \
constexpr virtual Type type() override { return TYPE; }                   \
constexpr virtual char const* type_as_string() override { return #TYPE; } \

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

    constexpr virtual Type type() = 0;
    constexpr virtual char const* type_as_string() = 0;

    std::list<std::unique_ptr<INode>> nodes {};
};

} // libpreprocessor
