set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(LibPreprocessor_SourceFiles ${LibPreprocessor_SourceFiles}
    "${DIR}/Lexer.cpp"
    "${DIR}/Parser.cpp"
    "${DIR}/Interpreter.cpp"

    PARENT_SCOPE
)
