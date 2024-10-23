#pragma once
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <GL/glew.h>

// ���̴� ������ �о���� �Լ�
char* readShaderSource(const char* shaderFile);

// ���̴��� �������ϴ� �Լ�
GLuint compileShader(const char* filePath, GLenum shaderType);

// ���̴� ���α׷��� ��ũ�ϴ� �Լ�
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

#endif // FILE_UTILS_H
