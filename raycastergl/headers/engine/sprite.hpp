#pragma once

#include <glm/vec2.hpp>

struct Sprite {
    float x;
    float y;
    uint32_t texture;
    int32_t uDiv;
    int32_t vDiv;
    float vMove;

    inline operator glm::vec2() {
        return glm::vec2(x, y);
    }
};
