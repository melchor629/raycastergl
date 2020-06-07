#pragma once

#include <string>
#include <glm/vec2.hpp>

struct Arguments {
    int32_t vsync;
    std::string map;
    glm::ivec2 initialWindowSize;

    bool parseArguments(int argc, const char* const argv[]);
};