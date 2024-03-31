#pragma once

#include "IStatement.hpp"

namespace libpreprocessor {

struct UnconditionalStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::UNCONDITIONAL);
    virtual ~UnconditionalStatementNode() override = default;
    std::unique_ptr<INode> branch {};
};

} // libpreprocessor
