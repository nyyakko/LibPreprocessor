#pragma once

#include "core/Interpreter.hpp"

#include <liberror/Maybe.hpp>

#include <string_view>
#include <filesystem>

namespace libpreprocessor {

liberror::Maybe<std::string> preprocess(std::string_view source, PreprocessorContext const& context);
liberror::Maybe<std::string> preprocess(std::filesystem::path path, PreprocessorContext const& context);

} // libpreprocessor

