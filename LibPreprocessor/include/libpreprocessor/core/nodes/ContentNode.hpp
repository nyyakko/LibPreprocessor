#pragma once

#include "INode.hpp"

#include <string>

namespace libpreprocessor {

struct ContentNode : INode
{
    NODE_TYPE(INode::Type::CONTENT);
    virtual ~ContentNode() override = default;
    std::string content {};
};

} // libpreprocessor
