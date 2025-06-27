# pre-processor library.

# installation

you may install it with [CPM](https://github.com/cpm-cmake/CPM.cmake) or install directly into your system with the following:

```bash
python install.py
```

and then include it with cmake into your project

```cmake
cmake_minimum_required_version(VERSION 3.25)

project(CoolProject LANGUAGES CXX)

find_package(LibPreprocessor CONFIG REQUIRED)
add_executable(CoolProject source.cpp)
target_link_libraries(CoolProject PRIVATE LibPreprocessor::LibPreprocessor)
```

# examples
```c++
#include <libpreprocessor/Processor.hpp>

#include <iostream>

int main()
{
    std::print("Enter your name: ");
    std::string name {};
    std::cin >> name;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:USERNAME", name },
        }
    };

    auto static constexpr source =
        "%IF [NOT <FALSE>]:\n"
        "    %PRINT [<hello: <ENV:USERNAME>!>]\n"
        "%END\n"sv;

    auto const result = libpreprocessor::process(source, context);

    if (!result.has_value())
    {
        std::cerr << result.error().message() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << result.value() << '\n';
}
```

```c++
#include <libpreprocessor/Processor.hpp>

#include <iostream>

int main()
{
    using namespace std::literals;

    std::print("Do you like potatoes? [yes/no] ");
    std::string name {};
    std::cin >> name;

    libpreprocessor::PreprocessorContext context {
        .environmentVariables = {
            { "ENV:LIKEPOTATOES", name },
        }
    };

    auto static constexpr source =
        "i wonder what will happen to this line...\n"
        "%IF [<ENV:LIKEPOTATOES> EQUALS <yes>]:\n"
        "    awesome!\n"
        "%ELSE:\n"
        "    what a shame...\n"
        "%END\n"
        "this line too..."sv;

    auto const result = libpreprocessor::process(source, context);

    if (!result.has_value())
    {
        std::cerr << result.error().message() << '\n';
        return EXIT_FAILURE;
    }

    std::cout << result.value() << '\n';
}
```

i recommend you to simply explore the code and see what you can do with it. seriously. do it.

