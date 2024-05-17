#include "core/Parser.hpp"

#include "core/nodes/Nodes.hpp"

#include <algorithm>
#include <list>

namespace libpreprocessor {

using namespace liberror;

namespace internal {

static ErrorOr<void> context_identify(Parser::Context const& context, Token const& token);
static ErrorOr<void> context_requires_ending_colon(Parser::Context const& context, Token const& token);

}

namespace {

ErrorOr<std::unique_ptr<INode>> parse_operator_node(Parser& parser, Parser::Context const& context, Token const& token, std::unique_ptr<INode>& root)
{
    auto operatorNode   = std::make_unique<OperatorNode>();
    operatorNode->name  = token.data;
    operatorNode->arity = [&] {
        static constexpr auto unaryOperators  = { "NOT" };
        static constexpr auto binaryOperators = { "AND", "OR", "EQUALS", "CONTAINS" };
        if (std::ranges::find(unaryOperators, operatorNode->name) != unaryOperators.end()) return OperatorNode::Arity::UNARY;
        if (std::ranges::find(binaryOperators, operatorNode->name) != binaryOperators.end()) return OperatorNode::Arity::BINARY;
        [[unlikely]];
        assert(false && "UNREACHABLE");
        return OperatorNode::Arity{};
    }();

    if (root == nullptr)
    {
        operatorNode->lhs = TRY(parser.parse({ context.parent, context.child, context.whois }));
        if (operatorNode->arity == OperatorNode::Arity::BINARY)
            operatorNode->rhs = TRY(parser.parse({ context.parent, context.child + 1, context.whois }));
        return operatorNode;
    }

    if (!(root->type() == INode::Type::EXPRESSION || root->type() == INode::Type::LITERAL))
        return liberror::make_error(PREFIX_ERROR": {}: Unexpected token of type \"{}\" was processed.", token.location_as_string(), root->type_as_string());

    if (operatorNode->arity == OperatorNode::Arity::BINARY)
        operatorNode->rhs = TRY(parser.parse({ context.parent, context.child + 1, context.whois }));

    switch (root->type())
    {
    case INode::Type::LITERAL:    operatorNode->lhs = std::move(root); break;
    case INode::Type::EXPRESSION: operatorNode->lhs = std::move(static_cast<ExpressionNode*>(root.get())->value); break;
    default: {
        assert(false && "UNREACHABLE");
    }
    }

    return operatorNode;
}

ErrorOr<std::unique_ptr<INode>> parse_if_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    ErrorOr<std::unique_ptr<INode>> statementNode = nullptr;

    if (token.data == "IF")
    {
        auto conditionalStatement       = std::make_unique<ConditionalStatementNode>();
        conditionalStatement->condition = TRY(parser.parse({ context.parent, context.child, Parser::Context::Who::IF_STATEMENT }));

        if (conditionalStatement->condition != nullptr && conditionalStatement->condition->type() != INode::Type::EXPRESSION)
            return make_error(PREFIX_ERROR": {}: \"%IF\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", token.location_as_string(), conditionalStatement->condition->type_as_string());

        if (conditionalStatement->condition == nullptr)
            return make_error(PREFIX_ERROR": {}: An \"%IF\" statement didn't had a condition.", token.location_as_string());

        conditionalStatement->branch.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::IF_STATEMENT }));
        conditionalStatement->branch.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::IF_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return make_error(PREFIX_ERROR": {}: An \"%IF\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(conditionalStatement);
    }
    else if (token.data == "ELSE")
    {
        return parser.parse({ context.parent, context.child + 1, Parser::Context::Who::ELSE_STATEMENT });
    }

    return statementNode;
}

ErrorOr<std::unique_ptr<INode>> parse_switch_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    ErrorOr<std::unique_ptr<INode>> statementNode = nullptr;

    if (token.data == "SWITCH")
    {
        auto selectionStatement   = std::make_unique<SelectionStatementNode>();
        selectionStatement->match = TRY(parser.parse({ context.parent, context.child + 1, Parser::Context::Who::SWITCH_STATEMENT }));

        if (selectionStatement->match != nullptr && selectionStatement->match->type() != INode::Type::EXPRESSION)
            return make_error(PREFIX_ERROR": {}: \"%SWITCH\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", token.location_as_string(), selectionStatement->match->type_as_string());

        if (selectionStatement->match == nullptr)
            return make_error(PREFIX_ERROR": {}: An \"%SWITCH\" statement didn't had a expression to match.", token.location_as_string());

        selectionStatement->branches.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::SWITCH_STATEMENT }));
        selectionStatement->branches.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::SWITCH_STATEMENT }));

        if (!(selectionStatement->branches.first || selectionStatement->branches.second))
            return make_error(PREFIX_ERROR": {}: An \"%SWITCH\" statement must have atleast a %DEFAULT case.", token.location_as_string());

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return make_error(PREFIX_ERROR": {}: An \"%SWITCH\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionStatement);
    }
    else if (token.data == "CASE")
    {
        auto selectionMatchStatement    = std::make_unique<SelectionMatchStatementNode>();
        selectionMatchStatement->match  = TRY(parser.parse({ context.parent, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (selectionMatchStatement->match != nullptr && selectionMatchStatement->match->type() != INode::Type::EXPRESSION)
            return make_error(PREFIX_ERROR": {}: \"%CASE\" statement expects an \"INode::Type::EXPRESSION\" to evaluate, but instead got \"{}\".", token.location_as_string(), selectionMatchStatement->match->type_as_string());

        if (selectionMatchStatement->match == nullptr)
            return make_error(PREFIX_ERROR": {}: An \"%CASE\" statement didn't had a expression to match.", token.location_as_string());

        selectionMatchStatement->branch = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return make_error(PREFIX_ERROR": {}: An \"%CASE\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionMatchStatement);
    }
    else if (token.data == "DEFAULT")
    {
        auto selectionMatchStatementNode   = std::make_unique<SelectionMatchStatementNode>();
        selectionMatchStatementNode->match = [] {
            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = "DEFAULT";
            return literalNode;
        }();
        selectionMatchStatementNode->branch = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (selectionMatchStatementNode->branch == nullptr)
            return make_error(PREFIX_ERROR": {}: An \"%DEFAULT\" statement didn't had a body.", token.location_as_string());

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return make_error(PREFIX_ERROR": {}: An \"%DEFAULT\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionMatchStatementNode);
    }

    return statementNode;
}

ErrorOr<std::unique_ptr<INode>> parse_print_statement_node(Parser& parser, Parser::Context const& context)
{
    auto printStatementNode = std::make_unique<PrintStatementNode>();
    printStatementNode->content = TRY(parser.parse({ context.parent, context.child, Parser::Context::Who::PRINT_STATEMENT }));
    return printStatementNode;
}

ErrorOr<std::unique_ptr<INode>> parse_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    if (token.data == "IF" || token.data == "ELSE") return parse_if_statement_node(parser, context, token);
    if (token.data == "SWITCH" || token.data == "CASE" || token.data == "DEFAULT") return parse_switch_statement_node(parser, context, token);
    if (token.data == "PRINT") return parse_print_statement_node(parser, context);
    return make_error(PREFIX_ERROR": {}: An unexpected keyword \"{}\" was reached.", token.location_as_string(), token.type_as_string());
}

}

ErrorOr<std::unique_ptr<INode>> Parser::parse(Context const& context)
{
    ErrorOr<std::unique_ptr<INode>> root = nullptr;

    while (!eof())
    {
        auto const token = take();

        switch (token.type)
        {
        case Token::Type::PERCENT: {
            if (!is_keyword(peek()))
                return make_error(PREFIX_ERROR": {}: Expected \"Token::Type::KEYWORD\" after \"%\", but found \"{}\" instead.", peek().location_as_string(), token.type_as_string());

            auto const peekedEndToken = peek().data == "END";
            auto const peekedElseOrDefault = context.child > context.parent && (peek().data == "ELSE" || peek().data == "DEFAULT");

            if (peekedEndToken || peekedElseOrDefault)
            {
                tokens().push(token);
                return root;
            }

            auto statementNode = TRY(parse_statement_node(*this, context, take()));

            if (root.value() == nullptr) root = std::move(statementNode);
            else root.value()->nodes.push_back(std::move(statementNode));

            break;
        }
        case Token::Type::LEFT_SQUARE_BRACKET: {
            TRY(internal::context_identify(context, token));

            auto expressionNode   = std::make_unique<ExpressionNode>();
            expressionNode->value = TRY(parse({ context.parent, context.child, Context::Who::EXPRESSION }));
            root                  = std::move(expressionNode);

            if (eof())
                return make_error(PREFIX_ERROR": {}: Expected \"]\", but found \"EOF\" instead.", token.location_as_string());
            if (peek().type != Token::Type::OPERATOR && !is_right_square_bracket(peek()))
                return make_error(PREFIX_ERROR": {}: Expected \"]\", but found \"{}\" instead.", peek().location_as_string(), peek().type_as_string());

            break;
        }
        case Token::Type::RIGHT_SQUARE_BRACKET: {
            TRY(internal::context_identify(context, token));

            if (!eof() && !is_operator(peek()))
            {
                TRY(internal::context_requires_ending_colon(context, peek()));
                return root;
            }

            break;
        }
        case Token::Type::LEFT_ROUND_BRACKET: {
            TRY(internal::context_identify(context, token));

            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = take().data;
            root               = std::move(literalNode);

            if (eof())
                return make_error(PREFIX_ERROR": {}: Expected \")\", but found \"EOF\" instead.", token.location_as_string());
            if (!is_right_round_bracket(peek()))
                return make_error(PREFIX_ERROR": {}: Expected \")\", but found \"{}\" instead.", peek().location_as_string(), peek().type_as_string());

            break;
        }
        case Token::Type::RIGHT_ROUND_BRACKET: {
            TRY(internal::context_identify(context, token));
            if (!eof() && !is_operator(peek())) return root;
            break;
        }
        case Token::Type::COLON: {
            TRY(internal::context_identify(context, token));
            root = std::make_unique<BodyNode>();
            break;
        }
        case Token::Type::OPERATOR: {
            TRY(internal::context_identify(context, token));
            return TRY(parse_operator_node(*this, context, token, *root));
        }
        case Token::Type::CONTENT: {
            auto contentNode     = std::make_unique<ContentNode>();
            contentNode->content = token.data;

            if (root.value() == nullptr) root = std::move(contentNode);
            else root.value()->nodes.push_back(std::move(contentNode));

            break;
        }

        case Token::Type::KEYWORD:
        case Token::Type::IDENTIFIER:
        case Token::Type::LITERAL:

        case Token::Type::BEGIN__:
        case Token::Type::END__: {
            return make_error(PREFIX_ERROR": {}: Unexpected token of kind \"{}\" was reached.", token.location_as_string(), token.type_as_string());
        }
        }
    }

    return root;
}

}

namespace libpreprocessor::internal {

static ErrorOr<void> context_identify(Parser::Context const& context, Token const& token)
{
    if (context.whois == Parser::Context::Who::BEGIN__ ||
        context.whois == Parser::Context::Who::END__)
        return make_error(PREFIX_ERROR": {}: A stray token of type \"{}\" was reached.", token.location_as_string(), token.type_as_string());
    return {};
}

static ErrorOr<void> context_requires_ending_colon(Parser::Context const& context, Token const& token)
{
    switch (context.whois)
    {
    case Parser::Context::Who::IF_STATEMENT:
    case Parser::Context::Who::ELSE_STATEMENT:
    case Parser::Context::Who::SWITCH_STATEMENT:
    case Parser::Context::Who::CASE_STATEMENT: {
        if (!is_colon(token))
            return make_error(PREFIX_ERROR": {}: Expected \":\", but found \"{}\" instead.", token.location_as_string(), token.type_as_string());
        break;
    }

    case Parser::Context::Who::PRINT_STATEMENT:
    case Parser::Context::Who::EXPRESSION: {
        break;
    }

    case Parser::Context::Who::BEGIN__:
    case Parser::Context::Who::END__: {
        return make_error(PREFIX_ERROR": Context::whois had an invalid value.");
    }
    }

    return {};
}

}

