#pragma once

#include "IStatement.hpp"

namespace libpreprocessor {

struct ConditionalStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::CONDITIONAL);

    virtual ~ConditionalStatementNode() override = default;

    std::unique_ptr<INode> condition {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branch {};
};

} // libpreprocessor
