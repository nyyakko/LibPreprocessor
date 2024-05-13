#pragma once

#include "core/Interpreter.hpp"

#include <liberror/ErrorOr.hpp>

#include <string_view>
#include <filesystem>

namespace libpreprocessor {

liberror::ErrorOr<std::string> preprocess(std::string_view source, PreprocessorContext const& context);
liberror::ErrorOr<std::string> preprocess(std::filesystem::path path, PreprocessorContext const& context);

} // libpreprocessor

