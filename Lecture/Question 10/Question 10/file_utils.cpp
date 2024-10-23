#define _CRT_SECURE_NO_WARNINGS
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

// ���̴� ������ �о���� �Լ�
char* readShaderSource(const char* shaderFile) {
    std::ifstream file(shaderFile, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "���̴� ���� ���� ����: " << shaderFile << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    char* source = new char[content.size() + 1];
    std::strcpy(source, content.c_str());

    return source;
}

// ���̴��� �������ϴ� �Լ�
GLuint compileShader(const char* filePath, GLenum shaderType) {
    char* shaderSource = readShaderSource(filePath);
    if (!shaderSource) {
        std::cerr << "���̴� �ҽ� �ε� ����: " << filePath << std::endl;
        return 0;
    }

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    delete[] shaderSource;

    glCompileShader(shader);

    // ������ ���� Ȯ��
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        std::cerr << typeStr << " ���̴� ������ ����:\n" << infoLog << std::endl;
        return 0;
    }

    std::cout << "���̴� ������ ����: " << filePath << std::endl;
    return shader;
}

// ���̴� ���α׷��� ��ũ�ϴ� �Լ�
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // ��ũ ���� Ȯ��
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "���̴� ���α׷� ��ũ ����:\n" << infoLog << std::endl;
        return 0;
    }

    std::cout << "���̴� ���α׷� ��ũ ����." << std::endl;
    return program;
}
