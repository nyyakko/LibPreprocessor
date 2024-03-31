cmake_minimum_required(VERSION 3.25)

project(LibPreprocessor LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(EXPORT_DIR "${CMAKE_SOURCE_DIR}/build/cmake")

include(static_analyzers)
include(enable_vcpkg)
include(remote_clone)
include(enable_tests)
include(GNUInstallDirs)

option(ENABLE_CLANGTIDY "" TRUE)
option(ENABLE_CPPCHECK "" TRUE)
option(ENABLE_TESTING "" TRUE)

if (ENABLE_TESTING)
    enable_tests()
endif()

enable_vcpkg()

set(LibPreprocessor_CompilerOptions ${LibPreprocessor_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibPreprocessor_LinkerOptions ${LibPreprocessor_LinkerOptions})

find_package(liberror CONFIG REQUIRED)
set(LibPreprocessor_ExternalLibraries LibError::LibError)

add_subdirectory(libpreprocessor)
