#pragma once

#include <vector>
#include "buffer.hpp"

class BufferAttribute: Buffer {
public:
    enum DataType {
        Float,
        UnsignedInt,
        Int,
    };

private:
    DataType dataType;
    size_t dataSize;
    uint32_t internalSize = 3;
    bool normalized = false;

    friend class BufferGeometry;

    template<typename T>
    BufferAttribute(
        const std::vector<T>& data,
        uint32_t internalSize,
        DataType dataType,
        bool normalized
    ): Buffer(Buffer::ArrayBuffer, data.data(), data.size()), dataType(dataType), internalSize(internalSize), normalized(normalized) {
        dataSize = data.size();
    }

public:
    BufferAttribute(
        const std::initializer_list<float>& data,
        uint32_t internalSize,
        bool normalized = false
    ): BufferAttribute(std::vector<float>(data), internalSize, Float, normalized) {}

    BufferAttribute(
        const std::initializer_list<uint32_t>& data,
        uint32_t internalSize,
        bool normalized = false
    ): BufferAttribute(std::vector<uint32_t>(data), internalSize, UnsignedInt, normalized) {}

    BufferAttribute(const BufferAttribute& o): Buffer(o) {
        dataType = o.dataType;
        dataSize = o.dataSize;
        internalSize = o.internalSize;
        normalized = o.normalized;
    }
};
