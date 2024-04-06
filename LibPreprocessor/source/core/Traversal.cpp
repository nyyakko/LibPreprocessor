#include "core/Traversal.hpp"

#include "core/nodes/Nodes.hpp"

#include <functional>
#include <algorithm>
#include <sstream>
#include <array>

namespace libpreprocessor {

using namespace liberror;
using namespace std::literals;

namespace detail {

static ErrorOr<void> traverse(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context);

namespace {

std::string unquoted(std::string const& string)
{
    return string.starts_with("\"") && string.ends_with("\"")
                ? string.substr(1, string.size() - 2)
                : string;
}

ErrorOr<size_t> decay_to_integer_literal(std::string_view literal)
{
    if (literal.empty()) return make_error(PREFIX_ERROR": Cannot decay an empty literal.");
    std::stringstream stream {};
    stream << literal;
    size_t value {};
    stream >> value;
    if (stream.fail()) return make_error(PREFIX_ERROR": Couldn't decay literal \"{}\" to a integer.", literal);
    return value;
}

ErrorOr<bool> decay_to_boolean_literal(std::string_view literal)
{
    if (literal.empty()) return make_error(PREFIX_ERROR": Cannot decay an empty literal.");
    if (literal == "TRUE") return true;
    if (literal == "FALSE") return false;
    auto const result = decay_to_integer_literal(literal);
    if (result.has_error()) return make_error(PREFIX_ERROR": Couldn't decay literal \"{}\" to a boolean.", literal);
    return static_cast<bool>(result.value());
}

ErrorOr<std::string> interpolate_literal(std::string_view string, PreprocessorContext const& context)
{
    if (string.empty()) return make_error(PREFIX_ERROR": Tried to interpolate an empty string.");

    ErrorOr<std::string> result {};

    for (auto index = 0zu; index < string.size(); index += 1)
    {
        if (string.at(index) == '<')
        {
            std::function<std::pair<std::string, std::string>()> fnParseIdentifier {};

            fnParseIdentifier = [&] () -> std::pair<std::string, std::string> {
                std::string value {};

                index += 1;
                while (string.at(index) != '>')
                {
                    if (string.at(index) == '<') value = fnParseIdentifier().first;
                    else
                    {
                        value += string.at(index);
                        index += 1;
                    }
                }
                index += 1;

                auto const originalValue = std::format("<{}>", value);

                if (context.localVariables.contains(value)) return { originalValue, context.localVariables.at(value) };
                if (context.environmentVariables.contains(value)) return { originalValue, context.environmentVariables.at(value) };

                return { originalValue, value };
            };

            result.value() += std::format("{}", fnParseIdentifier().second);
            index -= 1;
        }
        else
        {
            result.value() += string.at(index);
        }
    }

    if (context.localVariables.contains(result.value())) return context.localVariables.at(result.value());
    if (context.environmentVariables.contains(result.value())) return context.environmentVariables.at(result.value());

    return result;
}

ErrorOr<std::string> evaluate_expression(std::unique_ptr<INode> const& head, PreprocessorContext const& context);

ErrorOr<std::string> evaluate_unary_operator_expression(OperatorNode const* operatorNode, PreprocessorContext const& context)
{
    auto const lhs = TRY([&] -> liberror::ErrorOr<std::string> {
        auto value = static_cast<LiteralNode*>(operatorNode->lhs.get())->value;
        if (value.starts_with("\"") && value.ends_with("\"")) return unquoted(value);
        return interpolate_literal(value, context);
    }());

    if (std::ranges::contains(std::array { "NOT" }, operatorNode->name))
    {
        return TRY(decay_to_boolean_literal(lhs)) ? "FALSE"s : "TRUE"s;
    }

    return make_error(PREFIX_ERROR": Unknown unary operator \"{}\" was reached.", operatorNode->name);
}

ErrorOr<std::string> evaluate_binary_operator_expression(OperatorNode const* operatorNode, PreprocessorContext const& context)
{
    if (!operatorNode->rhs)
    {
        return make_error(PREFIX_ERROR": Operator \"{}\" is a binary operator and expects both an left-hand and an right-hand side, but only the former was given.", operatorNode->name);
    }

    auto const lhs = TRY([&] -> liberror::ErrorOr<std::string> {
        auto value = static_cast<LiteralNode*>(operatorNode->lhs.get())->value;
        if (value.starts_with("\"") && value.ends_with("\"")) return unquoted(value);
        return interpolate_literal(value, context);
    }());

    auto const rhs = TRY([&] -> liberror::ErrorOr<std::string> {
        auto value = static_cast<LiteralNode*>(operatorNode->rhs.get())->value;
        if (value.starts_with("\"") && value.ends_with("\"")) return unquoted(value);
        return interpolate_literal(value, context);
    }());

    if (operatorNode->name == "CONTAINS") return std::ranges::contains_subrange(lhs, rhs) ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "EQUALS") return lhs == rhs ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "AND") return lhs == "TRUE" && rhs == "TRUE" ? "TRUE"s : "FALSE"s;
    if (operatorNode->name == "OR") return lhs == "TRUE" || rhs == "TRUE" ? "TRUE"s : "FALSE"s;

    return make_error(PREFIX_ERROR": Unknown binary operator \"{}\" was reached.", operatorNode->name);
}

ErrorOr<std::string> evaluate_operator_expression(ExpressionNode const* expressionNode, PreprocessorContext const& context)
{
    auto* operatorNode = static_cast<OperatorNode*>(expressionNode->value.get());

    if (operatorNode == nullptr) return make_error(PREFIX_ERROR": operatorNode was nullptr.");
    if (!operatorNode->lhs) return make_error(PREFIX_ERROR": For any operator, it must have atleast one value for it to work on.");

    if (operatorNode->lhs->type() == INode::Type::EXPRESSION)
    {
        auto lhs          = std::make_unique<LiteralNode>();
        lhs->value        = TRY(evaluate_expression(operatorNode->lhs, context));
        operatorNode->lhs = std::move(lhs);
    }

    if (operatorNode->rhs && operatorNode->rhs->type() == INode::Type::EXPRESSION)
    {
        auto rhs          = std::make_unique<LiteralNode>();
        rhs->value        = TRY(evaluate_expression(operatorNode->rhs, context));
        operatorNode->rhs = std::move(rhs);
    }

    switch (operatorNode->arity)
    {
    case OperatorNode::Arity::UNARY: {
        return evaluate_unary_operator_expression(operatorNode, context);
    }
    case OperatorNode::Arity::BINARY: {
        return evaluate_binary_operator_expression(operatorNode, context);
    }

    case OperatorNode::Arity::BEGIN__:
    case OperatorNode::Arity::END__: {
        break;
    }
    }

    return make_error(PREFIX_ERROR": Operator \"{}\" had an invalid arity.", operatorNode->name);
}

ErrorOr<std::string> evaluate_literal_expression(ExpressionNode const* expressionNode, PreprocessorContext const& context)
{
    auto const* literalNode = static_cast<LiteralNode const*>(expressionNode->value.get());
    if (literalNode == nullptr) return make_error(PREFIX_ERROR": literalNode was nullptr.");

    auto const value = literalNode->value;

    if (value.starts_with("\"") && value.ends_with("\"")) return unquoted(value);

    if (context.localVariables.contains(value)) return context.localVariables.at(value);
    if (context.environmentVariables.contains(value)) return context.environmentVariables.at(value);

    return interpolate_literal(value, context);
}

ErrorOr<std::string> evaluate_expression(std::unique_ptr<INode> const& head, PreprocessorContext const& context)
{
    if (!head) return make_error(PREFIX_ERROR": Head node was nullptr.");

    if (head->type() != INode::Type::EXPRESSION)
    {
        return make_error(PREFIX_ERROR": Head node is expected to be of type \"INode::Type::EXPRESSION\", instead it was \"{}\".", head->type_as_string());
    }

    auto const* expressionNode = static_cast<ExpressionNode const*>(head.get());

    switch (expressionNode->value->type())
    {
    case INode::Type::OPERATOR: {
        return evaluate_operator_expression(expressionNode, context);
    }
    case INode::Type::LITERAL: {
        return evaluate_literal_expression(expressionNode, context);
    }

    case INode::Type::EXPRESSION:
    case INode::Type::STATEMENT:
    case INode::Type::CONTENT:
    case INode::Type::CONDITION:
    case INode::Type::BODY:

    case INode::Type::START__:
    case INode::Type::END__: {
        return make_error(PREFIX_ERROR": Unexpected node of type \"{}\" was reached.", expressionNode->type_as_string());
    }
    }

    return "FALSE"s;
}

ErrorOr<void> traverse_if_statement(IStatementNode const* statementNode, std::stringstream& stream, PreprocessorContext const& context)
{
    auto const* node = static_cast<ConditionalStatementNode const*>(statementNode);

    if (node->condition == nullptr) return make_error(PREFIX_ERROR": \"%IF\" statement condition was nullptr.");
    if (node->condition->type() != INode::Type::EXPRESSION) return make_error(PREFIX_ERROR": \"%IF\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", node->condition->type_as_string());

    if (TRY(evaluate_expression(node->condition, context)) == "TRUE")
        return traverse(node->branch.first, stream, context);

    if (node->branch.second)
        return traverse(node->branch.second, stream, context);

    return {};
}

ErrorOr<void> traverse_switch_statement(IStatementNode const* statementNode, std::stringstream& stream, PreprocessorContext const& context)
{
    if (statementNode->statement_type() == IStatementNode::Type::MATCH_CASE)
    {
        auto const* node = static_cast<SelectionMatchStatementNode const*>(statementNode);
        TRY(traverse(node->branch, stream, context));
    }
    else
    {
        auto const* node = static_cast<SelectionStatementNode const*>(statementNode);

        if (node->match == nullptr) return make_error(PREFIX_ERROR": \"%SWITCH\" statement match was nullptr.");
        if (node->match->type() != INode::Type::EXPRESSION) return make_error(PREFIX_ERROR": %SWITCH statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", node->match->type_as_string());

        auto const match = TRY(evaluate_expression(node->match, context));
        bool hasHandledNormalCase = false;

        for (auto const& subnode : node->branches.first->nodes)
        {
            auto const* innerNode = static_cast<SelectionMatchStatementNode const*>(subnode.get());

            if (innerNode == nullptr)
                return make_error(PREFIX_ERROR": \"%CASE\" statement was nulllptr.");
            if (innerNode->match == nullptr)
                return make_error(PREFIX_ERROR": \"%CASE\" statement match was nullptr.");
            if (innerNode->match->type() != INode::Type::EXPRESSION)
                return make_error(PREFIX_ERROR": \"%CASE\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", innerNode->match->type_as_string());

            if (unquoted(TRY(evaluate_expression(innerNode->match, context))) == match)
            {
                TRY(traverse(subnode, stream, context));
                hasHandledNormalCase = true;
                break;
            }
        }

        if (!hasHandledNormalCase && node->branches.second)
        {
            auto const* innerNode = static_cast<SelectionMatchStatementNode const*>(node->branches.second.get());
            TRY(traverse(innerNode->branch, stream, context));
        }
    }

    return {};
}

ErrorOr<void> traverse_print_statement(IStatementNode const* statementNode, PreprocessorContext const& context)
{
    auto const* node = static_cast<PrintStatementNode const*>(statementNode);

    if (node->content == nullptr) return make_error(PREFIX_ERROR": \"%PRINT\" statement didn't had an \"INode::Type::EXPRESSION\".");
    if (node->content->type() != INode::Type::EXPRESSION) return make_error(PREFIX_ERROR": %PRINT statement expects an \"INode::Type::EXPRESSION\" as argument, but instead got \"{}\".", node->content->type_as_string());

    std::println("{}", TRY(evaluate_expression(node->content, context)));

    return {};
}

ErrorOr<void> traverse_statement(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context)
{
    auto const* node = static_cast<IStatementNode const*>(head.get());

    switch (node->statement_type())
    {
    case IStatementNode::Type::CONDITIONAL: {
        TRY(traverse_if_statement(node, stream, context));
        break;
    }
    case IStatementNode::Type::MATCH:
    case IStatementNode::Type::MATCH_CASE: {
        TRY(traverse_switch_statement(node, stream, context));
        break;
    }
    case IStatementNode::Type::PRINT: {
        TRY(traverse_print_statement(node, context));
        break;
    }

    case IStatementNode::Type::START__:
    case IStatementNode::Type::END__:
    default: {
        return make_error(PREFIX_ERROR": Unexpected statement node of type \"{}\" was reached.", node->type_as_string());
    }
    }

    return {};
}

ErrorOr<void> traverse_content(std::unique_ptr<INode> const& head, std::stringstream& stream)
{
    auto const* contentNode = static_cast<ContentNode const*>(head.get());
    stream << contentNode->content;

    if (!((contentNode->content.front() == contentNode->content.back()) && contentNode->content.front() == '\n'))
    {
        stream << '\n';
    }

    return {};
}

}

static ErrorOr<void> traverse(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context)
{
    if (!head) return make_error(PREFIX_ERROR": Head node was nullptr.");

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
    case INode::Type::BODY: {
        break;
    }

    case INode::Type::EXPRESSION:
    case INode::Type::CONDITION:
    case INode::Type::OPERATOR:
    case INode::Type::LITERAL:

    case INode::Type::START__:
    case INode::Type::END__:
    default: {
        return make_error(PREFIX_ERROR": Unexpected node of type \"{}\" was reached.", head->type_as_string());
    }
    }

    for (auto const& subnode : head->nodes)
    {
        TRY(traverse(subnode, stream, context));
    }

    return {};
}

}

ErrorOr<std::string> traverse(std::unique_ptr<INode> const& head, PreprocessorContext const& context)
{
    std::stringstream sourceStream {};
    TRY(detail::traverse(head, sourceStream, context));
    return sourceStream.str();
}

}

