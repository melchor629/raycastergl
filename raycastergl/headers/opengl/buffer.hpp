#pragma once

#include <stdint.h>
#include <functional>
#include <cstring>

class Buffer {
public:
    enum Type {
        ArrayBuffer,
        ElementArrayBuffer,
        ShaderStorageBuffer,
    };

    enum Usage {
        StreamDraw,
        StreamRead,
        StreamCopy,
        StaticDraw,
        StaticRead,
        StaticCopy,
        DynamicDraw,
        DynamicRead,
        DynamicCopy,
    };

private:
    size_t bufferSize;
    void* data = nullptr;
    Type type;
    Usage usage = StreamCopy;

    friend class BufferGeometry;

    void build();

protected:
    uint32_t buffer = 0;

public:
    Buffer(Type type, Usage usage = StreamCopy): type(type), usage(usage) {}

    Buffer(Type type, size_t size, Usage usage = StreamCopy): Buffer(type, usage) {
        bufferSize = size;
    }

    template<typename DataType, size_t size>
    Buffer(Type type, const DataType data[size], Usage usage = StreamCopy): Buffer(type, size * sizeof(DataType), usage) {
        this->data = malloc(bufferSize);
        memcpy(this->data, data, bufferSize);
    }

    template<typename DataType>
    Buffer(Type type, const DataType* data, size_t count, Usage usage = StaticCopy): Buffer(type, count * sizeof(DataType), usage) {
        this->data = malloc(bufferSize);
        memcpy(this->data, data, bufferSize);
    }

    Buffer(const Buffer& o) {
        bufferSize = o.bufferSize;
        data = nullptr;
        type = o.type;
        usage = o.usage;
        buffer = 0;

        if(o.data) {
            data = malloc(o.bufferSize);
            memcpy(data, o.data, o.bufferSize);
        }
    }

    ~Buffer();

    void bind();
    void bindBase(uint32_t index);
    void setData(const void* data, size_t size);
    void mapWritableBuffer(const std::function<void(void*)>& func);
    void mapBuffer(const std::function<void(const void* const)>& func);

    void _writeContentsToFile(const char* fileName);

    template<typename DataType, size_t size>
    void setData(const DataType data[size]) {
        setData(data, size * sizeof(DataType));
    }

    template<typename DataType>
    void setData(const DataType* data, size_t count) {
        setData((void*) data, count * sizeof(DataType));
    }
};
