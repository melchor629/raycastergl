#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <glm/vec2.hpp>
#include "sprite.hpp"
#include <opengl/texture.hpp>

using namespace glm;
using namespace std;
namespace fs = std::filesystem;

struct Map {
    uint8_t* data;
    uvec2 size;
    std::variant<uint32_t, vec3> floor;
    std::variant<uint32_t, vec3> ceil;
    vec2 initialPos;
    vec2 initialDir;
    vec2 initialPlane;
    vector<Sprite> sprites;
    std::shared_ptr<Texture> texture;

    inline uint8_t& at(size_t x, size_t y) {
        return data[x * size.x + y];
    }

    void destroy();

    static optional<Map> load(const fs::path& path);
};
