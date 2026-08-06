#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ravel::proof {

struct ReflectionDeclaration {
    bool enabled = false;
    std::string mode;
};

// Deliberately tiny, closed reader for the bootstrap declaration.  The full
// Lua loader validates the complete schema when Lua is available.  This reader
// exists so math instrumentation can be activated without executing arbitrary
// Lua and without teaching the test executable any proof logic.
inline ReflectionDeclaration load_reflection_declaration(
    const std::filesystem::path& schema_path) {
    std::ifstream input(schema_path);
    if (!input) throw std::runtime_error("cannot read Lua schema: " + schema_path.string());
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string text = buffer.str();
    const auto reflection = text.find("reflection = {");
    if (reflection == std::string::npos) return {};
    const auto end = text.find("\n  },", reflection);
    if (end == std::string::npos) throw std::runtime_error("unterminated reflection declaration");
    const auto block = text.substr(reflection, end - reflection);
    ReflectionDeclaration declaration;
    declaration.enabled = block.find("enabled = true") != std::string::npos;
    if (block.find("mode = \"observe_and_prove\"") != std::string::npos)
        declaration.mode = "observe_and_prove";
    if (declaration.enabled && declaration.mode.empty())
        throw std::runtime_error("enabled reflection has unsupported or missing mode");
    return declaration;
}

} // namespace ravel::proof
