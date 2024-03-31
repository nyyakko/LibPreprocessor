add_subdirectory(core)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(LibPreprocessor_HeaderFiles ${LibPreprocessor_HeaderFiles}
    "${DIR}/Preprocessor.hpp"

    PARENT_SCOPE
)
