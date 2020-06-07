#include <opengl/shader.hpp>
#include <iostream>
#include <glad/glad.h>
#include <utils/files.hpp>
#include <opengl/check-error.hpp>

constexpr int typeToGl(Shader::Type type) {
    switch(type) {
        case Shader::Type::Vertex: return GL_VERTEX_SHADER;
        case Shader::Type::Fragment: return GL_FRAGMENT_SHADER;
        case Shader::Type::Compute: return GL_COMPUTE_SHADER;
        default: return GL_VERTEX_SHADER;
    }
}

Shader::Shader(Shader::Type type) {
    checkGlError(shader = glCreateShader(typeToGl(type)));
}

Shader::~Shader() {
    if(shader) {
        glDeleteShader(shader);
        shader = 0;
    }
}

bool Shader::load(const fs::path& path) {
    this->path = path;
    std::cout << "> Loading shader " << path << std::endl;
    auto content = readFile("res/shaders" / path);
    if(content == std::nullopt) {
        std::cerr << "Could not read shader " << path << std::endl;
        return false;
    }

    const auto src = content->c_str();
    checkGlError(glShaderSource(shader, 1, &src, nullptr));

    return true;
}

bool Shader::compile() const {
    std::cout << "> Compiling shader " << path << std::endl;
    checkGlError(glCompileShader(shader));

    int success;
    char infoLog[1024];
    checkGlError(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if(!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Could not compile shader " << path << ":" << std::endl << infoLog << std::endl;
        return false;
    }

    return true;
}
