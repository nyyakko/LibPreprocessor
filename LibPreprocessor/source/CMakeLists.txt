add_subdirectory(core)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(LibPreprocessor_SourceFiles ${LibPreprocessor_SourceFiles}
    "${DIR}/Preprocessor.cpp"

    PARENT_SCOPE
)
