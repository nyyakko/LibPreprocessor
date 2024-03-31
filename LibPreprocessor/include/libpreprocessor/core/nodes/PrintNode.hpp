#pragma once

#include "IStatement.hpp"

namespace libpreprocessor {

struct PrintStatementNode : IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::PRINT);
    virtual ~PrintStatementNode() override = default;
    std::unique_ptr<INode> content {};
};

} // libpreprocessor
