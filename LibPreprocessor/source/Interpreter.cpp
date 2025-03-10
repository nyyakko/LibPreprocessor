#include "Interpreter.hpp"

#include "nodes/Nodes.hpp"

#include <liberror/Try.hpp>
#include <fmt/format.h>

#include <sstream>

namespace libpreprocessor {

using namespace liberror;
using namespace std::literals;

#define ERROR(fmt, ...) make_error(PREFIX_ERROR ": " fmt __VA_OPT__(,) __VA_ARGS__)

namespace internal {

static Result<size_t> decay_to_integer(std::string_view literal);
static Result<bool> decay_to_boolean(std::string_view literal);
static Result<std::string> interpolate(std::string_view string, PreprocessorContext const& context);

} // namespace internal

namespace detail {

static Result<void> traverse(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context);

namespace {

Result<std::string> evaluate(std::unique_ptr<INode> const& head, PreprocessorContext const& context);

Result<std::string> evaluate_unary_operator(OperatorNode const* operatorNode, PreprocessorContext const& context)
{
    auto const lhs = TRY(evaluate(operatorNode->lhs, context));

    if (operatorNode->name == "NOT") return TRY(internal::decay_to_boolean(lhs)) ? "FALSE"s : "TRUE"s;

    return ERROR("Unknown unary operator \"{}\" was reached.", operatorNode->name);
}

Result<std::string> evaluate_binary_operator(OperatorNode const* operatorNode, PreprocessorContext const& context)
{
    if (operatorNode->rhs == nullptr)
    {
        return ERROR("Operator \"{}\" is a binary operator and expects both an left-hand and an right-hand side, but only the former was given.", operatorNode->name);
    }

    auto const lhs = TRY(evaluate(operatorNode->lhs, context));
    auto const rhs = TRY(evaluate(operatorNode->rhs, context));

    if (operatorNode->name == "CONTAINS") return lhs.contains(rhs)              ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "EQUALS")   return lhs == rhs                     ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "AND")      return lhs == "TRUE" && rhs == "TRUE" ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "OR")       return lhs == "TRUE" || rhs == "TRUE" ? "TRUE"s : "FALSE"s;

    return ERROR("Unknown binary operator \"{}\" was reached.", operatorNode->name);
}

Result<std::string> evaluate_operator(ExpressionNode const* expressionNode, PreprocessorContext const& context)
{
    auto* operatorNode = static_cast<OperatorNode*>(expressionNode->value.get());

    if (operatorNode == nullptr) return ERROR("operatorNode was nullptr.");
    if (operatorNode->lhs == nullptr) return ERROR("For any operator, it must have atleast one value for it to work on.");

    switch (operatorNode->arity)
    {
    case OperatorNode::Arity::UNARY:  return evaluate_unary_operator(operatorNode, context);
    case OperatorNode::Arity::BINARY: return evaluate_binary_operator(operatorNode, context);

    case OperatorNode::Arity::BEGIN__: break;
    case OperatorNode::Arity::END__: {
        break;
    }
    }

    return ERROR("Operator \"{}\" had an invalid arity.", operatorNode->name);
}

Result<std::string> evaluate_literal(ExpressionNode const* expressionNode, PreprocessorContext const& context)
{
    auto const* literalNode = static_cast<LiteralNode const*>(expressionNode->value.get());
    if (literalNode == nullptr) return ERROR("literalNode was nullptr.");
    return internal::interpolate(literalNode->value, context);
}

Result<std::string> evaluate(std::unique_ptr<INode> const& head, PreprocessorContext const& context)
{
    if (head == nullptr) return ERROR("Head node was nullptr.");

    if (!is_expression(head))
    {
        return ERROR("Head node is expected to be of type \"INode::Type::EXPRESSION\", instead it was \"{}\".", head->type_as_string());
    }

    auto const* expressionNode = static_cast<ExpressionNode const*>(head.get());

    switch (expressionNode->value->type())
    {
    case INode::Type::OPERATOR:   return evaluate_operator(expressionNode, context);
    case INode::Type::LITERAL:    return evaluate_literal(expressionNode, context);
    case INode::Type::EXPRESSION: return evaluate(expressionNode->value, context);

    case INode::Type::BEGIN__:
    case INode::Type::END__:
    default: {
        return ERROR("Unexpected node of type \"{}\" was reached.", expressionNode->type_as_string());
    }
    }

    return "FALSE"s;
}

Result<void> traverse_if_statement(IStatementNode const* statementNode, std::stringstream& stream, PreprocessorContext const& context)
{
    auto const* node = static_cast<IfStatementNode const*>(statementNode);

    if (node->condition == nullptr) return ERROR("\"%IF\" statement condition was nullptr.");

    if (TRY(evaluate(node->condition, context)) == "TRUE") return traverse(node->branch.first, stream, context);
    if (node->branch.second) return traverse(node->branch.second, stream, context);

    return {};
}

Result<void> traverse_switch_statement(IStatementNode const* statementNode, std::stringstream& stream, PreprocessorContext const& context)
{
    if (statementNode->statement_type() == IStatementNode::Type::SWITCH_CASE)
    {
        auto const* node = static_cast<SwitchCaseStatementNode const*>(statementNode);
        TRY(traverse(node->branch, stream, context));
    }
    else
    {
        auto const* node = static_cast<SwitchStatementNode const*>(statementNode);

        if (node->match == nullptr) return ERROR("\"%SWITCH\" statement match was nullptr.");

        auto const match = TRY(evaluate(node->match, context));
        bool hasHandledNormalCase = false;

        for (auto const& subnode : node->branches.first->nodes)
        {
            auto const* innerNode = static_cast<SwitchCaseStatementNode const*>(subnode.get());

            if (innerNode == nullptr) return ERROR("\"%CASE\" statement was nulllptr.");
            if (innerNode->match == nullptr) return ERROR("\"%CASE\" statement match was nullptr.");

            if (TRY(evaluate(innerNode->match, context)) == match)
            {
                TRY(traverse(subnode, stream, context));
                hasHandledNormalCase = true;
                break;
            }
        }

        if (!hasHandledNormalCase && node->branches.second)
        {
            auto const* innerNode = static_cast<SwitchCaseStatementNode const*>(node->branches.second.get());
            TRY(traverse(innerNode->branch, stream, context));
        }
    }

    return {};
}

Result<void> traverse_print_statement(IStatementNode const* statementNode, PreprocessorContext const& context)
{
    auto const* node = static_cast<PrintStatementNode const*>(statementNode);
    fmt::println("{}", TRY(evaluate(node->content, context)));
    return {};
}

Result<void> traverse_statement(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context)
{
    if (head == nullptr) return ERROR("Head node was nullptr.");

    if (!is_statement(head))
    {
        return ERROR("Head node is expected to be of type \"INode::Type::STATEMENT\", instead it was \"{}\".", head->type_as_string());
    }

    auto const* node = static_cast<IStatementNode const*>(head.get());

    switch (node->statement_type())
    {
    case IStatementNode::Type::IF: {
        TRY(traverse_if_statement(node, stream, context));
        break;
    }
    case IStatementNode::Type::SWITCH:
    case IStatementNode::Type::SWITCH_CASE: {
        TRY(traverse_switch_statement(node, stream, context));
        break;
    }
    case IStatementNode::Type::PRINT: {
        TRY(traverse_print_statement(node, context));
        break;
    }

    case IStatementNode::Type::BEGIN__:
    case IStatementNode::Type::END__:
    default: {
        return ERROR("Unexpected statement node of type \"{}\" was reached.", node->type_as_string());
    }
    }

    return {};
}

Result<void> traverse_content(std::unique_ptr<INode> const& head, std::stringstream& stream)
{
    if (head == nullptr) return ERROR("Head node was nullptr.");

    if (!is_content(head))
    {
        return ERROR("Head node is expected to be of type \"INode::Type::CONTENT\", instead it was \"{}\".", head->type_as_string());
    }

    auto const* contentNode = static_cast<ContentNode const*>(head.get());
    stream << contentNode->content;

    if (!((contentNode->content.front() == contentNode->content.back()) && contentNode->content.front() == '\n'))
    {
        stream << '\n';
    }

    return {};
}

}

static Result<void> traverse(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context)
{
    if (head == nullptr) return ERROR("Head node was nullptr.");

    switch (head->type())
    {
    case INode::Type::STATEMENT: {
        TRY(traverse_statement(head, stream, context));
        break;
    }
    case INode::Type::CONTENT: {
        TRY(traverse_content(head, stream));
        break;
    }
    case INode::Type::SCOPE: {
        break;
    }

    case INode::Type::EXPRESSION:
    case INode::Type::CONDITION:
    case INode::Type::OPERATOR:
    case INode::Type::LITERAL:

    case INode::Type::BEGIN__:
    case INode::Type::END__:
    default: {
        return ERROR("Unexpected node of type \"{}\" was reached.", head->type_as_string());
    }
    }

    for (auto const& subnode : head->nodes)
    {
        TRY(traverse(subnode, stream, context));
    }

    return {};
}

} // namespace detail

Result<std::string> interpret(std::unique_ptr<INode> const& head, PreprocessorContext const& context)
{
    std::stringstream sourceStream {};
    TRY(detail::traverse(head, sourceStream, context));
    return sourceStream.str();
}

} // namespace libpreprocessor

liberror::Result<size_t> libpreprocessor::internal::decay_to_integer(std::string_view literal)
{
    if (literal.empty()) return ERROR("Cannot decay an empty literal.");
    std::stringstream stream {};
    stream << literal;
    size_t value {};
    stream >> value;
    if (stream.fail()) return ERROR("Couldn't decay literal \"{}\" to a integer.", literal);
    return value;
}

liberror::Result<bool> libpreprocessor::internal::decay_to_boolean(std::string_view literal)
{
    if (literal.empty()) return ERROR("Cannot decay an empty literal.");
    if (literal == "TRUE") return true;
    if (literal == "FALSE") return false;

    auto const result = decay_to_integer(literal);
    if (!result.has_value())
        return ERROR("Couldn't decay literal \"{}\" to a boolean.", literal);

    return static_cast<bool>(result.value());
}

liberror::Result<std::string> libpreprocessor::internal::interpolate(std::string_view string, PreprocessorContext const& context)
{
    if (string.empty()) return ERROR("Tried to interpolate an empty string.");
    if (!string.contains("|")) return string.data();

    Result<std::string> result {};

    for (auto index = 0zu; index < string.size(); index += 1)
    {
        if (string.at(index) == '|')
        {
            auto const [_, value] = [] (this auto self, auto const& context, auto const& string, auto& index) -> std::pair<std::string, std::string> {
                std::string result {};

                index += 1;
                while (string.at(index) != '|')
                {
                    if (string.at(index) == '|') result  = self(context, string, index).first;
                    else                         result += string.at(index++);
                }
                index += 1;

                auto const originalValue = fmt::format("|{}|", result);
                auto const replacedValue =
                    context.environmentVariables.contains(result)
                        ? context.environmentVariables.at(result)
                        : result;

                return { originalValue, replacedValue };
            }(context, string, index);

            result->append(value);

            index -= 1;
        }
        else
        {
            result->push_back(string.at(index));
        }
    }

    return result;
}

