#pragma once

#include "INode.hpp"

#define STATEMENT_TYPE(TYPE)                                                              \
constexpr virtual Type statement_type() const override { return TYPE; }                   \
constexpr virtual char const* statement_type_as_string() const override { return #TYPE; } \

namespace libpreprocessor {

struct IStatementNode : INode
{
    NODE_TYPE(INode::Type::STATEMENT);

    enum class Type
    {
        START__,
        CONDITIONAL,
        MATCH,
        MATCH_CASE,
        PRINT,
        END__
    };

    virtual ~IStatementNode() = default;
    virtual constexpr Type statement_type() const = 0;
    virtual constexpr char const* statement_type_as_string() const = 0;

private:
    using INode::nodes;
};

} // libpreprocessor
