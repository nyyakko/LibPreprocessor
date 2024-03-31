# pre-processor library.

# installation

you may copy the files under LibPreprocessor/include into your project, install it with [vcpkg](https://github.com/echites/vcpkg-registry) or install directly into your system with the following: 

* ``py install.py``

and then include it with cmake into your project

```cmake
cmake_minimum_required_version(VERSION 3.25)

project(CoolProject LANGUAGES CXX)

find_package(libpreprocessor CONFIG REQUIRED)
find_package(liberror CONFIG REQUIRED)
add_executable(CoolProject source.cpp)
target_link_libraries(CoolProject PRIVATE LibError::LibError LibPreprocessor::LibPreprocessor)
```

# examples
```c++
#include <liberror/ErrorOr.hpp>
#include <libpreprocessor/Preprocessor.hpp>

#include <print>

int main()
{
    libpreprocessor::PreprocessorContext context {};

#if 1
    auto static constexpr source =
        "%IF [NOT <FALSE>]:\n"
        "    %PRINT [<hello!>]\n"
        "%END\n"sv;
#else
    std::print("Enter your name: ");
    std::string name {};
    std::cin >> name;

    libpreprocessor::PreprocessorContext context {
        .localVariables = {},
        .environmentVariables = {
            { "ENV:USERNAME", name },
        }
    };

    auto static constexpr source =
        "%IF [NOT <FALSE>]:\n"
        "    %PRINT [<hello: <ENV:USERNAME>!>]\n"
        "%END\n"sv;
#endif

    auto const result = MUST(libpreprocessor::preprocess(source, context));

    std::println("{}", result);
}
```

```c++
#include <liberror/ErrorOr.hpp>
#include <libpreprocessor/Preprocessor.hpp>

#include <print>

int main()
{
    libpreprocessor::PreprocessorContext context {};

    std::print("Do you like potatoes? [yes/no] ");
    std::string name {};
    std::cin >> name;

    libpreprocessor::PreprocessorContext context {
        .localVariables = {},
        .environmentVariables = {
            { "ENV:LIKEPOTATOES", name },
        }
    };

    auto static constexpr source =
        "i wonder what will happen to this line...\n"
        "%IF [<ENV:LIKEPOTATOES> EQUALS <yes>]:\n"
        "    awesome!\n"
        "%ELSE:\n"
        "    what a shame!\n"
        "%END\n"
        "this line too..."sv;

    auto const result = MUST(libpreprocessor::preprocess(source, context));

    std::println("{}", result);
}
```

i recommend you to simply explore the code and see what you can do with it. seriously. do it.

