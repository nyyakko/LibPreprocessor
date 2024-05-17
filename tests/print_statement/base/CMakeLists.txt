set(TEST_NAME print_statement)

project(${TEST_NAME} LANGUAGES CXX)

add_executable(${TEST_NAME} Main.cpp)

enable_clang_tidy(${TEST_NAME})
enable_cppcheck(${TEST_NAME})

target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)

target_link_libraries(${TEST_NAME} PRIVATE gtest_main gtest LibError::LibError LibPreprocessor::LibPreprocessor -Wno-gnu-statement-expression-from-macro-expansion)
target_compile_options(${TEST_NAME} PRIVATE ${LibPreprocessor_TestsCompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
target_link_options(${TEST_NAME} PRIVATE ${LibPreprocessor_TestsLinkerOptions} -Wno-gnu-statement-expression-from-macro-expansion)

gtest_discover_tests(${TEST_NAME})

