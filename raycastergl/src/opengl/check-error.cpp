#include <string>
#include <iostream>
#include <glad/glad.h>

void __checkGlError(const char* file, int line, const char* code) {
    GLenum error = GL_NO_ERROR;
    bool hadError = false;
    while((error = glGetError()) != GL_NO_ERROR) {
        if(!hadError) {
            std::cout << file << ":" << line << " OpenGL errors on " << code << ": ";
        }
        hadError = true;
        switch(error) {
            case GL_INVALID_ENUM: std::cout << "INVALID_ENUM "; break;
            case GL_INVALID_VALUE: std::cout << "INVALID_VALUE "; break;
            case GL_INVALID_OPERATION: std::cout << "INVALID_OPERATION "; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << "INVALID_FRAMEBUFFER_OPERATION "; break;
            case GL_OUT_OF_MEMORY: std::cout << "OUT_OF_MEMORY "; break;
            case GL_STACK_UNDERFLOW: std::cout << "STACK_UNDERFLOW "; break;
            case GL_STACK_OVERFLOW: std::cout << "STACK_OVERFLOW "; break;
            default: std::cout << "'OpenGL unknown error " << error << "' ";
        }
    }

    if(hadError) {
        std::cout << std::endl;
    }
}
