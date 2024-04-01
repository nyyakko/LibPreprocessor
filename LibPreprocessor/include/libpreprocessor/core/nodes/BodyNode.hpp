#pragma once

#include "INode.hpp"

namespace libpreprocessor {

struct BodyNode : INode
{
    NODE_TYPE(INode::Type::BODY);

    virtual ~BodyNode() override = default;
};

}

