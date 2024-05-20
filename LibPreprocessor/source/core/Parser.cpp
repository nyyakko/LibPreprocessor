#include "core/Parser.hpp"

#include "core/nodes/Nodes.hpp"

#include <algorithm>
#include <list>

namespace libpreprocessor {

using namespace liberror;

#define ERROR(fmt, ...) make_error(PREFIX_ERROR ": " fmt __VA_OPT__(,) __VA_ARGS__)

namespace internal {

static ErrorOr<void> context_identify(Parser::Context const& context, Token const& token);
static ErrorOr<void> context_requires_trailing_colon(Parser::Context const& context, Token const& token);

}

static constexpr std::array unaryOperators_g  { "NOT" };
static constexpr std::array binaryOperators_g { "AND", "OR", "EQUALS", "CONTAINS" };

namespace {

ErrorOr<std::unique_ptr<INode>> parse_operator_node(Parser& parser, Parser::Context const& context, Token const& token, std::unique_ptr<INode> node)
{
    auto operatorNode   = std::make_unique<OperatorNode>();
    operatorNode->name  = token.data;
    operatorNode->arity = [&] {
        if (std::ranges::find(unaryOperators_g, operatorNode->name) != unaryOperators_g.end()) return OperatorNode::Arity::UNARY;
        if (std::ranges::find(binaryOperators_g, operatorNode->name) != binaryOperators_g.end()) return OperatorNode::Arity::BINARY;
        [[unlikely]];
        assert(false && "UNREACHABLE");
        return OperatorNode::Arity{};
    }();

    auto fnAsExpression = [] (std::unique_ptr<INode> node) -> std::unique_ptr<INode> {
        if (is_expression(node)) return node;
        auto expression   = std::make_unique<ExpressionNode>();
        expression->value = std::move(node);
        return expression;
    };

    if (node == nullptr)
    {
        operatorNode->lhs = fnAsExpression(TRY(parser.parse({ context.parent, context.child, context.whois })));
    }
    else switch (node->type())
    {
    case INode::Type::LITERAL:    operatorNode->lhs = fnAsExpression(std::move(node)); break;
    case INode::Type::EXPRESSION: operatorNode->lhs = std::move(node); break;

    default: {
        return ERROR("{}: Unexpected token of type \"{}\" was processed.", token.location_as_string(), node->type_as_string());
    }
    }

    if (operatorNode->arity == OperatorNode::Arity::BINARY)
        operatorNode->rhs = fnAsExpression(TRY(parser.parse({ context.parent, context.child + 1, context.whois })));

    return operatorNode;
}

ErrorOr<std::unique_ptr<INode>> parse_if_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    ErrorOr<std::unique_ptr<INode>> statementNode = nullptr;

    if (token.data == "IF")
    {
        auto conditionalStatement       = std::make_unique<ConditionalStatementNode>();
        conditionalStatement->condition = TRY(parser.parse({ context.parent, context.child, Parser::Context::Who::IF_STATEMENT }));

        if (conditionalStatement->condition == nullptr)
            return ERROR("{}: An \"%IF\" statement didn't had a condition.", token.location_as_string());
        if (!is_expression(conditionalStatement->condition))
            return ERROR("{}: \"%IF\" statement expects an \"INode::Type::EXPRESSION\", instead got \"{}\".", token.location_as_string(), conditionalStatement->condition->type_as_string());

        conditionalStatement->branch.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::IF_STATEMENT }));
        conditionalStatement->branch.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::IF_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return ERROR("{}: An \"%IF\" statement missing its \"%END\" was reached.", token.location_as_string());

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

        if (selectionStatement->match != nullptr && !is_expression(selectionStatement->match))
            return ERROR("{}: \"%SWITCH\" statement expects \"INode::Type::EXPRESSION\", instead got \"{}\".", token.location_as_string(), selectionStatement->match->type_as_string());
        if (selectionStatement->match == nullptr)
            return ERROR("{}: An \"%SWITCH\" statement didn't had a expression to match.", token.location_as_string());

        selectionStatement->branches.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::SWITCH_STATEMENT }));
        selectionStatement->branches.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::SWITCH_STATEMENT }));

        if (!(selectionStatement->branches.first || selectionStatement->branches.second))
            return ERROR("{}: An \"%SWITCH\" statement must have atleast a %DEFAULT case.", token.location_as_string());
        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return ERROR("{}: An \"%SWITCH\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionStatement);
    }
    else if (token.data == "CASE")
    {
        auto selectionMatchStatement    = std::make_unique<SelectionMatchStatementNode>();
        selectionMatchStatement->match  = TRY(parser.parse({ context.parent, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (selectionMatchStatement->match == nullptr)
            return ERROR("{}: An \"%CASE\" statement didn't had a expression to match.", token.location_as_string());
        if (!is_expression(selectionMatchStatement->match))
            return ERROR("{}: \"%CASE\" expects an \"INode::Type::EXPRESSION\", instead got \"{}\".", token.location_as_string(), selectionMatchStatement->match->type_as_string());

        selectionMatchStatement->branch = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return ERROR("{}: An \"%CASE\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionMatchStatement);
    }
    else if (token.data == "DEFAULT")
    {
        auto selectionMatchStatementNode    = std::make_unique<SelectionMatchStatementNode>();
        selectionMatchStatementNode->branch = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));
        selectionMatchStatementNode->match  = [] {
            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = "DEFAULT";
            return literalNode;
        }();

        if (selectionMatchStatementNode->branch == nullptr)
            return ERROR("{}: An \"%DEFAULT\" statement didn't had a body.", token.location_as_string());
        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
            return ERROR("{}: An \"%DEFAULT\" statement missing its \"%END\" was reached.", token.location_as_string());

        parser.take();

        statementNode = std::move(selectionMatchStatementNode);
    }

    return statementNode;
}

ErrorOr<std::unique_ptr<INode>> parse_print_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    auto printStatement = std::make_unique<PrintStatementNode>();
    printStatement->content = TRY(parser.parse({ context.parent, context.child, Parser::Context::Who::PRINT_STATEMENT }));

    if (printStatement->content == nullptr)
        return ERROR("{}: \"%PRINT\" statement didn't had a \"INode::Type::EXPRESSION\".", token.location_as_string());
    if (!is_expression(printStatement->content))
        return ERROR("{}: \"%PRINT\" expects an \"INode::Type::EXPRESSION\", instead got \"{}\".", token.location_as_string(), printStatement->content->type_as_string());

    return printStatement;
}

ErrorOr<std::unique_ptr<INode>> parse_statement_node(Parser& parser, Parser::Context const& context, Token const& token)
{
    if (token.data == "IF" || token.data == "ELSE") return parse_if_statement_node(parser, context, token);
    if (token.data == "SWITCH" || token.data == "CASE" || token.data == "DEFAULT") return parse_switch_statement_node(parser, context, token);
    if (token.data == "PRINT") return parse_print_statement_node(parser, context, token);

    return ERROR("{}: An unexpected keyword \"{}\" was reached.", token.location_as_string(), token.type_as_string());
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
                return ERROR("{}: Expected \"Token::Type::KEYWORD\" after \"%\", but found \"{}\" instead.", peek().location_as_string(), token.type_as_string());

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
                return ERROR("{}: Expected \"]\", but found \"EOF\" instead.", token.location_as_string());
            if (!is_operator(peek()) && !is_right_square_bracket(peek()))
                return ERROR("{}: Expected \"]\", but found \"{}\" instead.", peek().location_as_string(), peek().type_as_string());

            break;
        }
        case Token::Type::RIGHT_SQUARE_BRACKET: {
            TRY(internal::context_identify(context, token));

            if (!eof() && !is_operator(peek()))
            {
                TRY(internal::context_requires_trailing_colon(context, peek()));
                return root;
            }

            break;
        }
        case Token::Type::LEFT_ANGLE_BRACKET: {
            TRY(internal::context_identify(context, token));

            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = take().data;
            root               = std::move(literalNode);

            if (eof())
                return ERROR("{}: Expected \">\", but found \"EOF\" instead.", token.location_as_string());
            if (!is_right_angle_bracket(peek()))
                return ERROR("{}: Expected \">\", but found \"{}\" instead.", peek().location_as_string(), peek().type_as_string());

            break;
        }
        case Token::Type::RIGHT_ANGLE_BRACKET: {
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
            return TRY(parse_operator_node(*this, context, token, std::move(*root)));
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
            return ERROR("{}: Unexpected token of kind \"{}\" was reached.", token.location_as_string(), token.type_as_string());
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
        return ERROR("{}: A stray token of type \"{}\" was reached.", token.location_as_string(), token.type_as_string());
    return {};
}

static ErrorOr<void> context_requires_trailing_colon(Parser::Context const& context, Token const& token)
{
    switch (context.whois)
    {
    case Parser::Context::Who::IF_STATEMENT:
    case Parser::Context::Who::ELSE_STATEMENT:
    case Parser::Context::Who::SWITCH_STATEMENT:
    case Parser::Context::Who::CASE_STATEMENT: {
        if (!is_colon(token))
            return ERROR("{}: Expected \":\", but found \"{}\" instead.", token.location_as_string(), token.type_as_string());
        break;
    }

    case Parser::Context::Who::PRINT_STATEMENT:
    case Parser::Context::Who::EXPRESSION: {
        break;
    }

    case Parser::Context::Who::BEGIN__:
    case Parser::Context::Who::END__: {
        return ERROR("Context::whois had an invalid value.");
    }
    }

    return {};
}

}

