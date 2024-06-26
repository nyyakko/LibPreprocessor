cmake_minimum_required(VERSION 3.25)

project(LibPreprocessor LANGUAGES CXX)

include(cmake/static_analyzers.cmake)
include(cmake/get_cpm.cmake)
include(cmake/enable_tests.cmake)
include(GNUInstallDirs)

if (ENABLE_TESTING)
    enable_tests(${PROJECT_NAME})
endif()

set(LibPreprocessor_CompilerOptions ${LibPreprocessor_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibPreprocessor_LinkerOptions ${LibPreprocessor_LinkerOptions})

CPMAddPackage("gh:fmtlib/fmt#10.2.1")
CPMAddPackage(
    NAME LibError
    GITHUB_REPOSITORY nyyakko/LibError
    GIT_TAG master
    VERSION 27c056f
    OPTIONS "ENABLE_CLANGTIDY OFF" "ENABLE_CPPCHECK OFF"
)

set(LibPreprocessor_ExternalLibraries LibError::LibError)

add_subdirectory(LibPreprocessor)
