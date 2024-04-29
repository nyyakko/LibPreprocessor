cmake_minimum_required(VERSION 3.25)

project(LibPreprocessor LANGUAGES CXX)

set(EXPORT_DIR "${CMAKE_SOURCE_DIR}/build/cmake")

include(cmake/static_analyzers.cmake)
include(cmake/CPM.cmake)
include(cmake/remote_clone.cmake)
include(cmake/enable_tests.cmake)
include(GNUInstallDirs)

if (ENABLE_TESTING)
    enable_tests()
endif()

set(LibPreprocessor_CompilerOptions ${LibPreprocessor_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibPreprocessor_LinkerOptions ${LibPreprocessor_LinkerOptions})

CPMAddPackage("gh:nyyakko/expected@master#52f2dd4")

set(LibPreprocessor_ExternalLibraries LibError::LibError)

add_subdirectory(LibPreprocessor)
