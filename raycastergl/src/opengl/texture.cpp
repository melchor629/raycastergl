#include <opengl/texture.hpp>
#include <glad/glad.h>
#include <opengl/check-error.hpp>

constexpr int wrapToGlWrap(Texture::Wrap wrap) {
    switch(wrap) {
        case Texture::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case Texture::ClampToBorder: return GL_CLAMP_TO_BORDER;
        case Texture::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case Texture::Repeat: return GL_REPEAT;
        default: return GL_REPEAT;
    }
}

constexpr int filterToGlFilter(Texture::Filter filter) {
    switch(filter) {
        case Texture::Nearest: return GL_NEAREST;
        case Texture::Linear: return GL_LINEAR;
        case Texture::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case Texture::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case Texture::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case Texture::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_NEAREST;
    }
}

constexpr int formatToGlFormat(Texture::InternalFormat format) {
    switch(format) {
        case Texture::R8UI: return GL_R8UI;
        case Texture::RGBA32F: return GL_RGBA32F;
        default: return GL_RGB;
    }
}

constexpr int formatToGlFormat(Texture::ExternalFormat format) {
    switch(format) {
        case Texture::RedInteger: return GL_RED_INTEGER;
        case Texture::RGB: return GL_RGB;
        default: return GL_RGB;
    }
}

constexpr int dataTypeToGlType(Texture::DataType dataType) {
    switch(dataType) {
        case Texture::UnsignedByte: return GL_UNSIGNED_BYTE;
        default: return GL_UNSIGNED_BYTE;
    }
}

Texture::Texture(Type type) {
    switch(type) {
        case Type::_2D: this->type = GL_TEXTURE_2D; break;
        case Type::Array2D: this->type = GL_TEXTURE_2D_ARRAY; break;
    }
}

Texture::~Texture() {
    if(texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}

void Texture::checkTextureIsBound() {
#ifndef NDEBUG
    int p, toCheck = GL_TEXTURE_BINDING_2D;
    switch(type) {
        case GL_TEXTURE_2D_ARRAY: toCheck = GL_TEXTURE_BINDING_2D_ARRAY; break;
        case GL_TEXTURE_2D: toCheck = GL_TEXTURE_BINDING_2D; break;
    }

    checkGlError(glGetIntegerv(toCheck, &p));
    assert(uint32_t(p) == texture /* Ensure the texture is bound */);
#endif
}

void Texture::setWrap(Wrap s, Wrap t, Wrap) {
    checkTextureIsBound();
    checkGlError(glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapToGlWrap(s)));
    checkGlError(glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapToGlWrap(t)));
}

void Texture::setMinFilter(Filter filter) {
    checkTextureIsBound();
    checkGlError(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filterToGlFilter(filter)));
}

void Texture::setMagFilter(Filter filter) {
    checkTextureIsBound();
    checkGlError(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filterToGlFilter(filter)));
}

void Texture::fillImage2D(int level, InternalFormat iformat, ivec2 size, int border, ExternalFormat eformat, DataType dataType, const void* data) {
    internalFormat = iformat;

    checkTextureIsBound();
    checkGlError(glTexImage2D(
        type,
        level,
        formatToGlFormat(iformat),
        size.x,
        size.y,
        border,
        formatToGlFormat(eformat),
        dataTypeToGlType(dataType),
        data
    ));
}

void Texture::reserveStorage3D(InternalFormat format, ivec3 size, size_t levels) {
    internalFormat = format;

    checkTextureIsBound();
    checkGlError(glTexStorage3D(this->type, levels, formatToGlFormat(format), size.x, size.y, size.z));
}

void Texture::fillSubImage3D(int level, ivec3 offset, ivec3 size, ExternalFormat format, DataType dataType, const void* data) {
    checkTextureIsBound();
    checkGlError(glTexSubImage3D(
        type,
        level,
        offset.x,
        offset.y,
        offset.z,
        size.x,
        size.y,
        size.z,
        formatToGlFormat(format),
        dataTypeToGlType(dataType),
        data
    ));
}

void Texture::bind() {
    if(!texture) {
        checkGlError(glGenTextures(1, &texture));
    }

    checkGlError(glBindTexture(type, texture));
}

void Texture::bindImage(uint32_t index, uint32_t level, bool write, std::optional<int> layer) {
    if(!texture) {
        checkGlError(glGenTextures(1, &texture));
    }

    checkGlError(glIsTexture(texture));
    checkGlError(glBindImageTexture(
        index,
        texture,
        level,
        (bool) layer,
        layer.value_or(0),
        write ? GL_READ_WRITE : GL_READ_ONLY,
        formatToGlFormat(internalFormat)
    ));
}

