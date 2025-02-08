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

struct ScopeNode : INode
{
    NODE_TYPE(INode::Type::SCOPE);
    virtual ~ScopeNode() override = default;
};

struct IfStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::IF);

    virtual ~IfStatementNode() override = default;

    std::unique_ptr<INode> condition {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branch {};

private:
    using INode::nodes;
};

struct SwitchStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::SWITCH);

    virtual ~SwitchStatementNode() override = default;

    std::unique_ptr<INode> match {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branches {};

private:
    using INode::nodes;
};

struct SwitchCaseStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::SWITCH_CASE);

    virtual ~SwitchCaseStatementNode() override = default;

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

