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
        SCOPE,
        END__
    };

    virtual ~INode() = default;

    constexpr virtual Type type() const = 0;
    constexpr virtual char const* type_as_string() const = 0;

    std::list<std::unique_ptr<INode>> nodes {};
};

constexpr bool is_statement(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::STATEMENT; }
constexpr bool is_expression(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::EXPRESSION; }
constexpr bool is_content(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::CONTENT; }
constexpr bool is_condition(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::CONDITION; }
constexpr bool is_operator(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::OPERATOR; }
constexpr bool is_literal(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::LITERAL; }
constexpr bool is_scope(std::unique_ptr<INode> const& node) { return node->type() == INode::Type::SCOPE; }

} // libpreprocessor
