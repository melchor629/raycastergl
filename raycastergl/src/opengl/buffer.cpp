#include <opengl/buffer.hpp>
#include <fstream>
#include <glad/glad.h>
#include <opengl/check-error.hpp>

constexpr int usageToGlUsage(Buffer::Usage usage) {
    switch(usage) {
        case Buffer::StreamDraw: return GL_STREAM_DRAW;
        case Buffer::StreamRead: return GL_STREAM_READ;
        case Buffer::StreamCopy: return GL_STREAM_COPY;
        case Buffer::StaticDraw: return GL_STATIC_DRAW;
        case Buffer::StaticRead: return GL_STATIC_READ;
        case Buffer::StaticCopy: return GL_STATIC_COPY;
        case Buffer::DynamicDraw: return GL_DYNAMIC_DRAW;
        case Buffer::DynamicRead: return GL_DYNAMIC_READ;
        case Buffer::DynamicCopy: return GL_DYNAMIC_COPY;
        default: return GL_STREAM_COPY;
    }
}

constexpr int typeToGlType(Buffer::Type type) {
    switch(type) {
        case Buffer::ArrayBuffer: return GL_ARRAY_BUFFER;
        case Buffer::ElementArrayBuffer: return GL_ELEMENT_ARRAY_BUFFER;
        case Buffer::ShaderStorageBuffer: return GL_SHADER_STORAGE_BUFFER;
        default: return GL_ARRAY_BUFFER;
    }
}

Buffer::~Buffer() {
    if(buffer) {
        glDeleteBuffers(1, &buffer);
        buffer = 0;
    }

    if(data) {
        free(data);
        data = nullptr;
    }
}

void Buffer::build() {
    auto type = typeToGlType(this->type);
    auto usage = usageToGlUsage(this->usage);
    checkGlError(glGenBuffers(1, &buffer));
    checkGlError(glBindBuffer(type, buffer));
    checkGlError(glBufferData(type, bufferSize, data, usage));
}

void Buffer::bind() {
    if(!buffer) {
        build();
    }

    checkGlError(glBindBuffer(typeToGlType(type), buffer));
}

void Buffer::bindBase(uint32_t index) {
    if(!buffer) {
        build();
    }

    checkGlError(glBindBufferBase(typeToGlType(type), index, buffer));
}

void Buffer::setData(const void* data, size_t size) {
    if(bufferSize < size || !this->data) {
        this->data = realloc(this->data, size);
    }

    auto type = typeToGlType(this->type);
    auto usage = usageToGlUsage(this->usage);
    memcpy(this->data, data, size);
    bufferSize = size;
    checkGlError(glBufferData(type, bufferSize, data, usage));
}

void Buffer::mapBuffer(const std::function<void(const void* const)>& func) {
    bind();
    auto type = typeToGlType(this->type);
    GLvoid* p = glMapBuffer(type, GL_READ_ONLY);
    func(p);
    glUnmapBuffer(type);
}

void Buffer::mapWritableBuffer(const std::function<void(void*)>& func) {
    bind();
    auto type = typeToGlType(this->type);
    GLvoid* p = glMapBuffer(type, GL_READ_WRITE);
    func(p);
    glUnmapBuffer(type);
}

void Buffer::_writeContentsToFile(const char* fileName) {
    mapBuffer([this, fileName] (const void* const ptr) {
        std::ofstream ff("yes.bin");
        ff.write((const char*) ptr, this->bufferSize);
        ff.close();
    });
}

