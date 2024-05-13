#pragma once

#include "nodes/INode.hpp"

#include <liberror/ErrorOr.hpp>

#include <string>
#include <unordered_map>

namespace libpreprocessor {

struct PreprocessorContext
{
    std::unordered_map<std::string, std::string> environmentVariables {};
};

liberror::ErrorOr<std::string> interpret(std::unique_ptr<INode> const& head, PreprocessorContext const& context);

} // libpreprocessor

