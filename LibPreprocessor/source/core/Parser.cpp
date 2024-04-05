#include "core/Parser.hpp"

#include "core/nodes/Nodes.hpp"

#include <algorithm>
#include <list>

namespace libpreprocessor {

using namespace liberror;

namespace {

ErrorOr<void> identify(Parser::Context const& context, Token const& token)
{
    switch (context.whois)
    {
    case Parser::Context::Who::IF_STATEMENT:
    case Parser::Context::Who::ELSE_STATEMENT:
    case Parser::Context::Who::SWITCH_STATEMENT:
    case Parser::Context::Who::CASE_STATEMENT:
    case Parser::Context::Who::EXPRESSION:
    case Parser::Context::Who::PRINT_STATEMENT: return {};

    case Parser::Context::Who::BEGIN__:
    case Parser::Context::Who::END__: {
        break;
    }
    }

    return make_error(PREFIX_ERROR": A stray token of type \"{}\" was reached.", token.type_as_string());
}

ErrorOr<std::unique_ptr<INode>> parse_if_statement_node(Parser& parser, Parser::Context const& context, Token const& innerToken)
{
    ErrorOr<std::unique_ptr<INode>> statementNode = nullptr;

    if (innerToken.data == "IF")
    {
        auto conditionalStatement       = std::make_unique<ConditionalStatementNode>();
        conditionalStatement->condition = TRY(parser.parse({ context.parent, context.child, Parser::Context::Who::IF_STATEMENT }));

        if (conditionalStatement->condition == nullptr)
        {
            return make_error(PREFIX_ERROR": An \"%IF\" statement didn't had a condition.");
        }

        conditionalStatement->branch.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::IF_STATEMENT }));
        conditionalStatement->branch.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::IF_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
        {
            return make_error(PREFIX_ERROR": An \"%IF\" statement missing its \"%END\" was reached.");
        }

        parser.take();

        statementNode = std::move(conditionalStatement);
    }
    else if (innerToken.data == "ELSE")
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
        auto selectionStatementNode   = std::make_unique<SelectionStatementNode>();
        selectionStatementNode->match = TRY(parser.parse({ context.parent, context.child + 1, Parser::Context::Who::SWITCH_STATEMENT }));

        if (selectionStatementNode->match == nullptr)
        {
            return make_error(PREFIX_ERROR": An \"%SWITCH\" statement didn't had a expression to match.");
        }

        selectionStatementNode->branches.first  = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::SWITCH_STATEMENT }));
        selectionStatementNode->branches.second = TRY(parser.parse({ context.child, context.parent, Parser::Context::Who::SWITCH_STATEMENT }));

        if (!(selectionStatementNode->branches.first || selectionStatementNode->branches.second))
        {
            return make_error(PREFIX_ERROR": An \"%SWITCH\" statement must have atleast a %DEFAULT case.");
        }

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
        {
            return make_error(PREFIX_ERROR": An \"%SWITCH\" statement missing its \"%END\" was reached.");
        }

        parser.take();

        statementNode = std::move(selectionStatementNode);
    }
    else if (token.data == "CASE")
    {
        auto selectionMatchStatementNode    = std::make_unique<SelectionMatchStatementNode>();
        selectionMatchStatementNode->match  = TRY(parser.parse({ context.parent, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (selectionMatchStatementNode->match == nullptr)
        {
            return make_error(PREFIX_ERROR": An \"%CASE\" statement didn't had a expression to match.");
        }

        selectionMatchStatementNode->branch = TRY(parser.parse({ context.child, context.child + 1, Parser::Context::Who::CASE_STATEMENT }));

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
        {
            return make_error(PREFIX_ERROR": An \"%CASE\" statement missing its \"%END\" was reached.");
        }

        parser.take();

        statementNode = std::move(selectionMatchStatementNode);
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
        {
            return make_error(PREFIX_ERROR": An \"%DEFAULT\" statement didn't had a body.");
        }

        if (parser.eof() || (parser.take(), parser.peek().data != "END"))
        {
            return make_error(PREFIX_ERROR": An \"%DEFAULT\" statement missing its \"%END\" was reached.");
        }

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
    return make_error(PREFIX_ERROR": An unexpected statement \"{}\" was reached.", token.data);
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
            if (!is_statement(peek()))
            {
                return make_error(PREFIX_ERROR": Expected a node of type \"Token::Type::STATEMENT\" after \"%\", but instead got \"{}\".", token.type_as_string());
            }

            if (peek().data == "END") { tokens().push(token); return root; }
            if (context.child > context.parent && (peek().data == "ELSE" || peek().data == "DEFAULT")) { tokens().push(token); return root; }

            auto statementNode = TRY(parse_statement_node(*this, context, take()));

            if (root.value() == nullptr)
                root = std::move(statementNode);
            else
                root.value()->nodes.push_back(std::move(statementNode));

            break;
        }
        case Token::Type::LEFT_SQUARE_BRACKET: {
            TRY(identify(context, token));
            auto expressionNode   = std::make_unique<ExpressionNode>();
            expressionNode->value = TRY(parse({ context.parent, context.child, Context::Who::EXPRESSION }));
            root = std::move(expressionNode);
            break;
        }
        case Token::Type::RIGHT_SQUARE_BRACKET: {
            TRY(identify(context, token));
            if (!eof() && !is_identifier(peek()))
            {
                switch (context.whois)
                {
                case Context::Who::IF_STATEMENT:
                case Context::Who::ELSE_STATEMENT:
                case Context::Who::SWITCH_STATEMENT:
                case Context::Who::CASE_STATEMENT: {
                    if (!is_colon(peek()))
                    {
                        return make_error(PREFIX_ERROR": \"{}\" expects a terminating \":\", instead got \"{}\".", context.who_is_as_string(), peek().data);
                    }
                    [[fallthrough]];
                }
                case Context::Who::PRINT_STATEMENT:
                case Context::Who::EXPRESSION: {
                    break;
                }

                case Context::Who::BEGIN__:
                case Context::Who::END__: {
                    return make_error(PREFIX_ERROR": Context.whois had an invalid value.");
                }
                }
                return root;
            }
            break;
        }
        case Token::Type::LEFT_ANGLE_BRACKET: {
            TRY(identify(context, token));
            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = take().data;
            root               = std::move(literalNode);
            break;
        }
        case Token::Type::RIGHT_ANGLE_BRACKET: {
            TRY(identify(context, token));
            if (is_right_square_bracket(peek())) return root;
            break;
        }
        case Token::Type::COLON: {
            TRY(identify(context, token));
            root = std::make_unique<BodyNode>();
            break;
        }
        case Token::Type::IDENTIFIER: {
            TRY(identify(context, token));
            // FIXME: for now, all identifiers *must* be an operator. maybe we should be fixin' this in the future?
            auto operatorNode   = std::make_unique<OperatorNode>();
            operatorNode->name  = token.data;
            operatorNode->arity = [&] {
                auto const isUnary = std::ranges::contains(std::array { "NOT" }, operatorNode->name);
                if (isUnary) return OperatorNode::Arity::UNARY;

                auto const isBinary = std::ranges::contains(std::array { "AND", "OR", "EQUALS", "CONTAINS" }, operatorNode->name);
                if (isBinary) return OperatorNode::Arity::BINARY;

                [[unlikely]];

                return OperatorNode::Arity{};
            }();

            operatorNode->lhs = (root.value() == nullptr) ? TRY(parse({ context.parent, context.child, context.whois })) : std::move(root.value());

            if (operatorNode->arity == OperatorNode::Arity::BINARY)
            {
                operatorNode->rhs = TRY(parse({ context.parent, context.child + 1, context.whois }));
            }

            return operatorNode;
        }
        case Token::Type::CONTENT: {
            auto contentNode     = std::make_unique<ContentNode>();
            contentNode->content = token.data;

            if (root.value() == nullptr)
                root = std::move(contentNode);
            else
                root.value()->nodes.push_back(std::move(contentNode));

            break;
        }

        case Token::Type::STATEMENT:
        case Token::Type::LITERAL:

        case Token::Type::BEGIN__:
        case Token::Type::END__:
        default: {
            return make_error(PREFIX_ERROR": Unexpected token of kind \"{}\" was reached.", token.type_as_string());
        }
        }
    }

    return root;
}

}

