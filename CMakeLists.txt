cmake_minimum_required(VERSION 3.25)

project(LibPreprocessor LANGUAGES CXX)

include(cmake/get_cpm.cmake)
include(cmake/enable_tests.cmake)
include(GNUInstallDirs)

CPMAddPackage("gh:fmtlib/fmt#10.2.1")

CPMFindPackage(
    NAME expected
    GITHUB_REPOSITORY nyyakko/expected
    GIT_TAG master
)

CPMFindPackage(
    NAME LibError
    GITHUB_REPOSITORY nyyakko/LibError
    GIT_TAG master
    OPTIONS "ENABLE_CLANGTIDY OFF" "ENABLE_CPPCHECK OFF"
)

if (ENABLE_TESTING)
    enable_tests(${PROJECT_NAME})
endif()


include(cmake/static_analyzers.cmake)

set(LibPreprocessor_CompilerOptions ${LibPreprocessor_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibPreprocessor_LinkerOptions ${LibPreprocessor_LinkerOptions})

set(LibPreprocessor_ExternalLibraries LibError::LibError)

add_subdirectory(LibPreprocessor)
