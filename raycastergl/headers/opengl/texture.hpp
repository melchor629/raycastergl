#pragma once

#include <optional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using namespace glm;

class Texture {
public:
    enum Type {
        _2D,
        Array2D,
    };

    enum Filter {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear,
    };

    enum Wrap {
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat,
        Repeat,
        MirrorClampToEdge,
    };

    enum InternalFormat {
        RGBA32F,
        R8UI,
    };

    enum ExternalFormat {
        RedInteger,
        RGB,
    };

    enum DataType {
        UnsignedByte,
    };

private:
    uint32_t texture = 0;
    int type = 0;
    int levels = 1;
    InternalFormat internalFormat;

    void checkTextureIsBound();

public:
    Texture(Type type);
    ~Texture();

    Texture(const Texture& o) = default;

    Texture(Texture&& o) {
        texture = o.texture;
        type = o.type;
        levels = o.levels;
        internalFormat = o.internalFormat;

        o.texture = 0;
    }

    void setWrap(Wrap s, Wrap t = Repeat, Wrap r = Repeat);
    void setMinFilter(Filter filter);
    void setMagFilter(Filter filter);

    void fillImage2D(int level, InternalFormat iformat, ivec2 size, int border, ExternalFormat eformat, DataType type, const void* data);
    void reserveStorage3D(InternalFormat format, ivec3 size, size_t levels = 1);
    void fillSubImage3D(int level, ivec3 offset, ivec3 size, ExternalFormat format, DataType type, const void* data);

    void bind();
    void bindImage(uint32_t index, uint32_t level = 0, bool write = false, std::optional<int> layer = std::nullopt);
};
