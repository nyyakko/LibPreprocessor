set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(LibPreprocessor_HeaderFiles ${LibPreprocessor_HeaderFiles}
    "${DIR}/UnconditionalStatementNode.hpp"
    "${DIR}/ConditionalStatementNode.hpp"
    "${DIR}/SelectionStatementNode.hpp"
    "${DIR}/SelectionMatchStatementNode.hpp"
    "${DIR}/PrintNode.hpp"
    "${DIR}/OperatorNode.hpp"
    "${DIR}/LiteralNode.hpp"
    "${DIR}/IStatement.hpp"
    "${DIR}/ExpressionNode.hpp"
    "${DIR}/ContentNode.hpp"

    PARENT_SCOPE
)
