#include "core/Traversal.hpp"

#include "core/nodes/INode.hpp"
#include "core/nodes/IStatement.hpp"
#include "core/nodes/ConditionalStatementNode.hpp"
#include "core/nodes/ContentNode.hpp"
#include "core/nodes/ExpressionNode.hpp"
#include "core/nodes/LiteralNode.hpp"
#include "core/nodes/OperatorNode.hpp"
#include "core/nodes/PrintNode.hpp"
#include "core/nodes/BodyNode.hpp"
#include "core/nodes/SelectionMatchStatementNode.hpp"
#include "core/nodes/SelectionStatementNode.hpp"
#include "core/nodes/UnconditionalStatementNode.hpp"

#include <functional>
#include <algorithm>
#include <sstream>

namespace libpreprocessor {

using namespace liberror;

    namespace detail {

        namespace {

        std::string unquoted(std::string const& string) { return string.starts_with("\"") ? string.substr(1, string.size() - 2) : string; }

        ErrorOr<size_t> decay_to_integer_literal(std::string_view literal)
        {
            if (literal.empty())
            {
                return make_error("Cannot decay an empty literal.");
            }

            std::stringstream stream {};
            stream << literal;
            size_t value {};
            stream >> value;

            if (stream.fail())
            {
                return make_error("Couldn't decay literal \"{}\" to a integer.", literal);
            }

            return value;
        }

        ErrorOr<bool> decay_to_boolean_literal(std::string_view literal)
        {
            if (literal.empty())
            {
                return make_error("Cannot decay an empty literal.");
            }

            if (literal == "TRUE") return true;
            if (literal == "FALSE") return false;

            auto const result = decay_to_integer_literal(literal);

            if (result.has_error())
            {
                return make_error("Couldn't decay literal \"{}\" to a boolean.", literal);
            }

            return result;
        }

        ErrorOr<std::string> interpolate_string(std::string_view string, PreprocessorContext const& context)
        {
            if (string.empty()) return make_error("Tried to interpolate an empty string.");

            std::string result {};

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

                    result += std::format("{}", fnParseIdentifier().second);
                    index -= 1;
                }
                else
                {
                    result += string.at(index);
                }
            }

            return result;
        }

        ErrorOr<std::string> evaluate_expression(std::unique_ptr<INode> const& head, PreprocessorContext const& context)
        {
            if (!head) { return make_error("Head node was nullptr."); }

            if (head->type() != INode::Type::EXPRESSION)
            {
                return make_error("Head node is expected to be of type \"INode::Type::EXPRESSION\", instead it was \"{}\".", head->type_as_string());
            }

            auto* headExpression = static_cast<ExpressionNode*>(head.get());

            switch (headExpression->value->type())
            {
            case INode::Type::OPERATOR: {
                auto* operatorNode = static_cast<OperatorNode*>(headExpression->value.get());

                if (!operatorNode->lhs)
                {
                    return make_error("For any operator, it must have atleast one value for it to work on.");
                }

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
                    auto const lhs = TRY([&] -> liberror::ErrorOr<std::string> {
                        auto value = unquoted(static_cast<LiteralNode*>(operatorNode->lhs.get())->value);
                        return TRY(interpolate_string(value, context));
                    }());

                    if (operatorNode->name == "NOT")
                    {
                        return TRY(decay_to_boolean_literal(lhs)) ? "FALSE" : "TRUE";
                    }

                    break;
                }
                case OperatorNode::Arity::BINARY: {
                    if (!operatorNode->rhs)
                    {
                        return make_error("Operator \"{}\" is a binary operator and expects both an left-hand and an right-hand side, but only the former was given.", operatorNode->name);
                    }

                    auto const lhs = TRY([&] -> liberror::ErrorOr<std::string> {
                        auto value = unquoted(static_cast<LiteralNode*>(operatorNode->lhs.get())->value);
                        return TRY(interpolate_string(value, context));
                    }());

                    auto const rhs = TRY([&] -> liberror::ErrorOr<std::string> {
                        auto value = unquoted(static_cast<LiteralNode*>(operatorNode->rhs.get())->value);
                        return TRY(interpolate_string(value, context));
                    }());

                    if (operatorNode->name == "CONTAINS") { return std::ranges::contains_subrange(lhs, rhs) ? "TRUE" : "FALSE"; }
                    if (operatorNode->name == "EQUALS") { return lhs == rhs ? "TRUE" : "FALSE"; }
                    if (operatorNode->name == "AND") { return lhs == "TRUE" && rhs == "TRUE" ? "TRUE" : "FALSE"; }
                    if (operatorNode->name == "OR") { return lhs == "TRUE" || rhs == "TRUE" ? "TRUE" : "FALSE"; }

                    break;
                }

                case OperatorNode::Arity::BEGIN__:
                case OperatorNode::Arity::END__:
                default: {
                    return make_error("Operator \"{}\" had an invalid arity.", operatorNode->name);
                }
                }
                break;
            }
            case INode::Type::LITERAL: {
                auto const* literalNode  = static_cast<LiteralNode*>(headExpression->value.get());
                auto const unquotedValue = unquoted(literalNode->value);

                if (context.localVariables.contains(unquotedValue)) return context.localVariables.at(unquotedValue);
                if (context.environmentVariables.contains(unquotedValue)) return context.environmentVariables.at(unquotedValue);

                return interpolate_string(unquotedValue, context);
            }

            case INode::Type::START__:
            case INode::Type::EXPRESSION:
            case INode::Type::STATEMENT:
            case INode::Type::CONTENT:
            case INode::Type::CONDITION:
            case INode::Type::BODY:
            case INode::Type::END__: {
                return make_error("Unexpected node of type \"{}\" was reached.", headExpression->type_as_string());
            }
            }

            return "FALSE";
        }

        }

    static ErrorOr<void> traverse(std::unique_ptr<INode> const& head, std::stringstream& stream, PreprocessorContext const& context)
    {
        if (!head) { return make_error("Head node was nullptr."); }

        switch (head->type())
        {
        case INode::Type::BODY: {
            auto* bodyNode = static_cast<BodyNode*>(head.get());

            for (auto const& subnode : bodyNode->nodes)
            {
                TRY(traverse(subnode, stream, context));
            }

            break;
        }
        case INode::Type::STATEMENT: {
            auto* statement = static_cast<IStatementNode*>(head.get());

            switch (statement->statement_type())
            {
            case IStatementNode::Type::CONDITIONAL: {
                auto* node = static_cast<ConditionalStatementNode*>(statement);

                if (TRY(evaluate_expression(node->condition, context)) == "TRUE")
                {
                    TRY(traverse(node->branch.first, stream, context));
                }
                else if (node->branch.second)
                {
                    TRY(traverse(node->branch.second, stream, context));
                }

                break;
            }
            case IStatementNode::Type::UNCONDITIONAL: {
                auto* node = static_cast<UnconditionalStatementNode*>(statement);
                TRY(traverse(node->branch, stream, context));
                break;
            }
            case IStatementNode::Type::MATCH: {
                auto* node = static_cast<SelectionStatementNode*>(statement);

                if (node->match->type() != INode::Type::EXPRESSION)
                {
                    return make_error("%SWITCH statement expects an \"INode::Type::EXPRESSION\" as match, but instead got \"{}\".", node->match->type_as_string());
                }

                auto match = TRY(evaluate_expression(node->match, context));
                bool hasHandledNormalCase = false;

                for (auto const& subnode : node->branches.first->nodes)
                {
                    auto* selectionMatchStatementNode = static_cast<SelectionMatchStatementNode*>(subnode.get());

                    if (unquoted(static_cast<LiteralNode*>(selectionMatchStatementNode->match.get())->value) == match)
                    {
                        TRY(traverse(selectionMatchStatementNode->branch, stream, context));
                        hasHandledNormalCase = true;
                        break;
                    }
                }

                if (!hasHandledNormalCase && node->branches.second)
                {
                    TRY(traverse(static_cast<SelectionMatchStatementNode*>(node->branches.second.get())->branch, stream, context));
                }

                break;
            }
            case IStatementNode::Type::PRINT: {
                auto* node = static_cast<PrintStatementNode*>(statement);

                if (node->content->type() != INode::Type::EXPRESSION)
                {
                    return make_error("%PRINT statement expects an \"INode::Type::EXPRESSION\" as argument, but instead got \"{}\".", node->content->type_as_string());
                }

                std::println("{}", TRY(evaluate_expression(node->content, context)));

                break;
            }

            case IStatementNode::Type::MATCH_CASE:

            case IStatementNode::Type::START__:
            case IStatementNode::Type::END__:
            default: {
                return make_error("Unexpected statement \"{}\" reached.", statement->type_as_string());
            }
            }

            break;
        }
        case INode::Type::CONTENT: {
            ContentNode* node = static_cast<ContentNode*>(head.get());
            stream << node->content;

            if (!((node->content.front() == node->content.back()) && node->content.front() == '\n'))
            {
                stream << '\n';
            }

            break;
        }

        case INode::Type::EXPRESSION:
        case INode::Type::CONDITION:
        case INode::Type::OPERATOR:
        case INode::Type::LITERAL:

        case INode::Type::START__:
        case INode::Type::END__:
        default: {
            return make_error("Unexpected node of type \"{}\" was reached.", head->type_as_string());
        }
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
