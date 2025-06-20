cmake_minimum_required(VERSION 3.25)

project(LibPreprocessor LANGUAGES CXX)

include(cmake/get_cpm.cmake)
include(GNUInstallDirs)

CPMAddPackage("gh:fmtlib/fmt#10.2.1")
CPMAddPackage("gh:nyyakko/expected#master")
CPMAddPackage("gh:nyyakko/LibError#master")

include(cmake/static_analyzers.cmake)
include(cmake/enable_tests.cmake)

if (ENABLE_TESTING)
    enable_tests(${PROJECT_NAME})
endif()

set(LibPreprocessor_CompilerOptions ${LibPreprocessor_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibPreprocessor_LinkerOptions ${LibPreprocessor_LinkerOptions})

set(LibPreprocessor_ExternalLibraries LibError::LibError)

add_subdirectory(LibPreprocessor)
