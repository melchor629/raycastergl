#pragma once

#include "buffer-attribute.hpp"
#include <optional>

class BufferGeometry {
    uint32_t vertexArrayObject = 0;
    std::optional<BufferAttribute> indices = std::nullopt;
    std::vector<BufferAttribute> attributes;

    void build();

public:
    ~BufferGeometry();

    void setIndices(const BufferAttribute& indices);
    void addAttribute(const BufferAttribute& attribute);
    void draw();
};
