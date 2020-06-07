#include <opengl/buffer-geometry.hpp>
#include <glad/glad.h>
#include <opengl/check-error.hpp>

constexpr int attributeDataTypeToGlType(BufferAttribute::DataType dataType) {
    switch(dataType) {
        case BufferAttribute::DataType::Float: return GL_FLOAT;
        case BufferAttribute::DataType::UnsignedInt: return GL_UNSIGNED_INT;
        case BufferAttribute::DataType::Int: return GL_INT;
        default: return GL_FLOAT;
    }
}

BufferGeometry::~BufferGeometry() {
    if(vertexArrayObject) {
        glDeleteVertexArrays(1, &vertexArrayObject);
        vertexArrayObject = 0;
    }
}

void BufferGeometry::setIndices(const BufferAttribute& indices) {
    this->indices = indices;
}

void BufferGeometry::addAttribute(const BufferAttribute& attribute) {
    attributes.push_back(attribute);
}

void BufferGeometry::build() {
    checkGlError(glGenVertexArrays(1, &vertexArrayObject));
    checkGlError(glBindVertexArray(vertexArrayObject));

    if(indices != std::nullopt) {
        indices->type = BufferAttribute::ElementArrayBuffer;
        indices->bind();
    }

    for(auto it = attributes.begin(); it != attributes.end(); it += 1) {
        size_t pos = it - attributes.begin();
        int dataType = attributeDataTypeToGlType(it->dataType);
        it->bind();
        checkGlError(glVertexAttribPointer(pos, it->internalSize, dataType, it->normalized, 0, nullptr));
        checkGlError(glEnableVertexAttribArray(pos));
    }
}

void BufferGeometry::draw() {
    if(!vertexArrayObject) {
        build();
    } else {
        checkGlError(glBindVertexArray(vertexArrayObject));
    }

    if(indices != std::nullopt) {
        int dataType = attributeDataTypeToGlType(indices->dataType);
        checkGlError(glDrawElements(GL_TRIANGLES, indices->dataSize, dataType, nullptr));
    } else {
        // TODO should be using min here
        checkGlError(glDrawArrays(GL_TRIANGLES, 0, attributes[0].dataSize / attributes[0].internalSize));
    }
}

