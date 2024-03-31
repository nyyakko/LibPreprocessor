#pragma once

#include "IStatement.hpp"

namespace libpreprocessor {

struct SelectionStatementNode: IStatementNode
{
    STATEMENT_TYPE(IStatementNode::Type::MATCH);

    virtual ~SelectionStatementNode() override = default;

    std::unique_ptr<INode> match {};
    std::pair<
        std::unique_ptr<INode>, std::unique_ptr<INode>
    > branches {};
};

} // libpreprocessor
