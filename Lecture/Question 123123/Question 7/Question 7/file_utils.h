#pragma once
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <GL/glew.h>

// 셰이더 파일을 읽어오는 함수
char* readShaderSource(const char* shaderFile);

// 셰이더를 컴파일하는 함수
GLuint compileShader(const char* filePath, GLenum shaderType);

// 셰이더 프로그램을 링크하는 함수
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

#endif // FILE_UTILS_H
