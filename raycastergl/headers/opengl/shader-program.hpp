#pragma once

#include <initializer_list>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "shader.hpp"

class ShaderProgram {
private:
    uint32_t program = 0;
    std::string name;
    std::unordered_map<const char*, int> uniformCache;

    void checkProgramIsBound();
    int getUniformLocation(const char* name);

public:
    explicit ShaderProgram(const std::string& name);
    ~ShaderProgram();

    bool link(const std::initializer_list<Shader*>& shaders) const;

    void use();
    void setUniform(const char* name, uint32_t x);
    void setUniform(const char* name, int x, int y);
    void setUniform(const char* name, const glm::vec2& v);
    void setUniform(const char* name, const glm::vec4& v);

    void dispatchCompute(uint32_t x = 1, uint32_t y = 1, uint32_t z = 1);
};
