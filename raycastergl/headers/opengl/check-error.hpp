#pragma once

#include <string>
#include <iostream>
#include <glad/glad.h>

void __checkGlError(const char* file, int line, const char* code);

#ifndef NDEBUG
#define checkGlError(code) code; __checkGlError(__FILE__, __LINE__, #code)
#else
#define checkGlError(code) code
#endif
