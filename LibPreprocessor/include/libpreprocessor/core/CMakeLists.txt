add_subdirectory(nodes)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(LibPreprocessor_HeaderFiles ${LibPreprocessor_HeaderFiles}
    "${DIR}/Lexer.hpp"
    "${DIR}/Parser.hpp"
    "${DIR}/Token.hpp"
    "${DIR}/Traversal.hpp"

    PARENT_SCOPE
)
