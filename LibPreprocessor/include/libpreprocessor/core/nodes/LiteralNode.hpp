#pragma once

#include "INode.hpp"

#include <string>

namespace libpreprocessor {

struct LiteralNode : INode
{
    NODE_TYPE(INode::Type::LITERAL);
    virtual ~LiteralNode() override = default;
    std::string value {};
};

} // libpreprocessor
