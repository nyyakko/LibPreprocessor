#include "core/Parser.hpp"

#include "core/nodes/INode.hpp"
#include "core/nodes/ConditionalStatementNode.hpp"
#include "core/nodes/ContentNode.hpp"
#include "core/nodes/BodyNode.hpp"
#include "core/nodes/ExpressionNode.hpp"
#include "core/nodes/LiteralNode.hpp"
#include "core/nodes/OperatorNode.hpp"
#include "core/nodes/PrintNode.hpp"
#include "core/nodes/SelectionMatchStatementNode.hpp"
#include "core/nodes/SelectionStatementNode.hpp"
#include "core/nodes/UnconditionalStatementNode.hpp"

#include <algorithm>
#include <list>

namespace libpreprocessor {

using namespace liberror;

ErrorOr<std::unique_ptr<INode>> Parser::parse(int64_t parent, int64_t child)
{
    std::unique_ptr<INode> root = nullptr;

    while (!eof())
    {
        auto const token = take();

        switch (token.type)
        {
        case Token::Type::PERCENT: {
            if (!is_keyword(peek()))
            {
                return make_error("Expecting a node of type \"Token::Type::KEYWORD\" after \"%\", but instead got \"{}\".", token.type_as_string());
            }

            if (peek().data == "END") { tokens().push(token); return root; }
            if (child > parent && (peek().data == "ELSE" || peek().data == "DEFAULT")) { tokens().push(token); return root; }

            auto const innerToken = take();

            std::unique_ptr<INode> statementNode {};

            if (innerToken.data == "IF")
            {
                auto conditionalStatement           = std::make_unique<ConditionalStatementNode>();
                conditionalStatement->condition     = TRY(parse(parent, child));
                conditionalStatement->branch.first  = TRY(parse(child, child + 1));
                conditionalStatement->branch.second = TRY(parse(child, parent));

                take();

                if (eof() || peek().data != "END")
                {
                    return make_error("An \"%IF\" statement missing its \"%END\" was reached.");
                }

                take();

                statementNode = std::move(conditionalStatement);
            }
            else if (innerToken.data == "ELSE")
            {
                return TRY(parse(parent, child + 1));
            }
            else if (innerToken.data == "SWITCH")
            {
                auto selectionStatementNode             = std::make_unique<SelectionStatementNode>();
                selectionStatementNode->match           = TRY(parse(parent, child + 1));
                selectionStatementNode->branches.first  = TRY(parse(child, child + 1));
                selectionStatementNode->branches.second = TRY(parse(child, parent));

                take();

                if (eof() || peek().data != "END")
                {
                    return make_error("An \"%SWITCH\" statement missing its \"%END\" was reached.");
                }

                take();

                statementNode = std::move(selectionStatementNode);
            }
            else if (innerToken.data == "CASE")
            {
                auto selectionMatchStatementNode = std::make_unique<SelectionMatchStatementNode>();
                selectionMatchStatementNode->match = std::unique_ptr<LiteralNode>(
                     static_cast<LiteralNode*>(
                            static_cast<ExpressionNode*>(TRY(parse(parent, child + 1)).release())->value.release()
                    ));
                selectionMatchStatementNode->branch = TRY(parse(child, child + 1));

                take();

                if (eof() || peek().data != "END")
                {
                    return make_error("An \"%CASE\" statement missing its \"%END\" was reached.");
                }

                take();

                statementNode = std::move(selectionMatchStatementNode);
            }
            else if (innerToken.data == "DEFAULT")
            {
                auto selectionMatchStatementNode   = std::make_unique<SelectionMatchStatementNode>();
                selectionMatchStatementNode->match = [] {
                    auto literalNode   = std::make_unique<LiteralNode>();
                    literalNode->value = "DEFAULT";
                    return literalNode;
                }();
                selectionMatchStatementNode->branch = TRY(parse(child, child + 1));

                take();

                if (eof() || peek().data != "END")
                {
                    return make_error("An \"%DEFAULT\" statement missing its \"%END\" was reached.");
                }

                take();

                return selectionMatchStatementNode;
            }
            else if (innerToken.data == "PRINT")
            {
                auto printStatementNode = std::make_unique<PrintStatementNode>();
                printStatementNode->content = TRY(parse(parent, child + 1));
                statementNode = std::move(printStatementNode);
            }
            else
            {
                return make_error("Unexpected keyword \"{}\" was reached.", innerToken.data);
            }

            if (root == nullptr)
                root = std::move(statementNode);
            else
                root->nodes.push_back(std::move(statementNode));

            break;
        }
        case Token::Type::LEFT_SQUARE_BRACKET: {
            auto expressionNode   = std::make_unique<ExpressionNode>();
            expressionNode->value = TRY(parse(parent, child + 1));

            if (eof() || !is_right_square_bracket(peek()))
            {
                return make_error("\"[\" missing its \"]\"");
            }

            root = std::move(expressionNode);

            break;
        }
        case Token::Type::RIGHT_SQUARE_BRACKET: {
            if (!eof() && !is_identifier(peek())) return root;
            break;
        }
        case Token::Type::LEFT_ANGLE_BRACKET: {
            auto literalNode   = std::make_unique<LiteralNode>();
            literalNode->value = take().data;
            root               = std::move(literalNode);
            break;
        }
        case Token::Type::RIGHT_ANGLE_BRACKET: {
            if (is_right_square_bracket(peek())) return root;
            break;
        }
        case Token::Type::COLON: {
            root = std::make_unique<BodyNode>();
            break;
        }
        case Token::Type::IDENTIFIER: {
            // FIXME: for now, all identifiers *must* be an operator. maybe we should be fixin' this in the future?
            auto operatorNode   = std::make_unique<OperatorNode>();
            operatorNode->name  = token.data;
            operatorNode->arity = [&operatorNode] {
                if (operatorNode->name == "NOT")
                    return OperatorNode::Arity::UNARY;
                else if (std::ranges::contains(std::array { "AND", "OR", "EQUALS", "CONTAINS" }, operatorNode->name))
                    return OperatorNode::Arity::BINARY;
                [[unlikely]];
                return OperatorNode::Arity{};
            }();

            operatorNode->lhs = (root == nullptr) ? TRY(parse(parent, child + 1)) : std::move(root);

            if (operatorNode->arity == OperatorNode::Arity::BINARY)
            {
                operatorNode->rhs = TRY(parse(parent, child + 1));
            }

            return operatorNode;
        }

        case Token::Type::CONTENT: {
            auto contentNode     = std::make_unique<ContentNode>();
            contentNode->content = token.data;

            if (root == nullptr)
                root = std::move(contentNode);
            else
                root->nodes.push_back(std::move(contentNode));

            break;
        }

        case Token::Type::KEYWORD:

        case Token::Type::LITERAL:
        case Token::Type::BEGIN__:
        case Token::Type::END__:
        default: {
            return make_error("Unexpected token of kind \"{}\" was reached.", token.type_as_string());
        }
        }
    }

    return root;
}

}

