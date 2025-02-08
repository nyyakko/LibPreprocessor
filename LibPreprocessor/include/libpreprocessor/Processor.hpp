#pragma once

#include "Interpreter.hpp"

#include <liberror/Result.hpp>

#include <string_view>
#include <filesystem>

namespace libpreprocessor {

liberror::Result<std::string> process(std::string_view source, PreprocessorContext const& context);
liberror::Result<std::string> process(std::filesystem::path path, PreprocessorContext const& context);

} // namespace libpreprocessor

