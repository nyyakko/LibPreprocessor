#pragma once

#include "INode.hpp"

namespace libpreprocessor {

struct ExpressionNode : INode
{
    NODE_TYPE(INode::Type::EXPRESSION);
    virtual ~ExpressionNode() override = default;
    std::unique_ptr<INode> value {};

private:
    using INode::nodes;
};

} // libpreprocessor
