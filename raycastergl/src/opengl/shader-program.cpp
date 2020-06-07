#include <opengl/shader-program.hpp>
#include <algorithm>
#include <iostream>
#include <glad/glad.h>
#include <opengl/check-error.hpp>

ShaderProgram::ShaderProgram(const std::string& name): name(name) {
    program = glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
    if(program) {
        glDeleteProgram(program);
        program = 0;
    }
}

bool ShaderProgram::link(const std::initializer_list<Shader*>& shaders) const {
    std::cout << "> Linking shaders into program " << name << std::endl;
    std::for_each(shaders.begin(), shaders.end(), [this] (auto s) { checkGlError(glAttachShader(program, s->shader)); });
    checkGlError(glLinkProgram(program));
    std::for_each(shaders.begin(), shaders.end(), [this] (auto s) { checkGlError(glDetachShader(program, s->shader)); });

    int success;
    char infoLog[512];
    checkGlError(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if(!success) {
        checkGlError(glGetProgramInfoLog(program, 512, nullptr, infoLog));
        std::cerr << "Could not link shader program " << name << " :" << std::endl;
        std::cerr << infoLog << std::endl;
        return false;
    }

    return true;
}

void ShaderProgram::checkProgramIsBound() {
#ifndef NDEBUG
    int p;
    checkGlError(glGetIntegerv(GL_CURRENT_PROGRAM, &p));
    assert(uint32_t(p) == program /* Ensure the shader program is bound */);
#endif
}

int ShaderProgram::getUniformLocation(const char* name) {
    auto it = uniformCache.find(name);
    if(it != uniformCache.end()) {
        return it->second;
    }

    return checkGlError(uniformCache[name] = glGetUniformLocation(program, name));
}

void ShaderProgram::use() {
    checkGlError(glUseProgram(program));
}

void ShaderProgram::setUniform(const char* name, uint32_t x) {
    checkProgramIsBound();
    checkGlError(glUniform1ui(getUniformLocation(name), x));
}

void ShaderProgram::setUniform(const char* name, int x, int y) {
    checkProgramIsBound();
    checkGlError(glUniform2i(getUniformLocation(name), x, y));
}

void ShaderProgram::setUniform(const char* name, const glm::vec2& v) {
    checkProgramIsBound();
    checkGlError(glUniform2f(getUniformLocation(name), v.x, v.y));
}

void ShaderProgram::dispatchCompute(uint32_t x, uint32_t y, uint32_t z) {
    checkProgramIsBound();
    checkGlError(glDispatchCompute(x, y, z));
}

void ShaderProgram::setUniform(const char* name, const glm::vec4& v) {
    checkProgramIsBound();
    checkGlError(glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w));
}
