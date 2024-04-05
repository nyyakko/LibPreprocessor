#pragma once

#include "INode.hpp"
#include "IStatementNode.hpp"

#include <string>

namespace libpreprocessor {

struct ExpressionNode : INode
{
    NODE_TYPE(INode::Type::EXPRESSION);
    virtual ~ExpressionNode() override = default;
    std::unique_ptr<INode> value {};

private:
    using INode::nodes;
};

struct OperatorNode : INode
{
    NODE_TYPE(INode::Type::OPERATOR);

    enum class Arity
    {
        BEGIN__,
        UNARY,
        BINARY,
        END__
    };

    virtual ~OperatorNode() override = default;

    std::string name {};
    Arity arity {};
    std::unique_ptr<INode> lhs {};
    std::unique_ptr<INode> rhs {};

private:
    using INode::nodes;
};

struct LiteralNode : INode
{
    NODE_TYPE(INode::Type::LITERAL);
    virtual ~LiteralNode() override = default;
    std::string value {};

private:
    using INode::nodes;
};

struct ContentNode : INode
{
    NODE_TYPE(INode::Type::CONTENT);
    virtual ~ContentNode() override = default;
    std::string content {};

private:
    using INode::nodes;
};

struct BodyNode : INode
{
    NODE_TYPE(INode::Type::BODY);
    virtual ~BodyNode() override = default;
    std::list<std::unique_ptr<INode>> nodes {};
};

struct ConditionalStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::CONDITIONAL);

    virtual ~ConditionalStatementNode() override = default;

    std::unique_ptr<INode> condition {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branch {};

private:
    using INode::nodes;
};

struct SelectionStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::MATCH);

    virtual ~SelectionStatementNode() override = default;

    std::unique_ptr<INode> match {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branches {};

private:
    using INode::nodes;
};

struct SelectionMatchStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::MATCH_CASE);

    virtual ~SelectionMatchStatementNode() override = default;

    std::unique_ptr<INode> match {};
    std::unique_ptr<INode> branch {};

private:
    using INode::nodes;
};

struct PrintStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::PRINT);
    virtual ~PrintStatementNode() override = default;
    std::unique_ptr<INode> content {};

private:
    using INode::nodes;
};

} // libpreprocessor

