#define _CRT_SECURE_NO_WARNINGS
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

// 셰이더 파일을 읽어오는 함수
char* readShaderSource(const char* shaderFile) {
    std::ifstream file(shaderFile, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "셰이더 파일 열기 실패: " << shaderFile << std::endl;
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

// 셰이더를 컴파일하는 함수
GLuint compileShader(const char* filePath, GLenum shaderType) {
    char* shaderSource = readShaderSource(filePath);
    if (!shaderSource) {
        std::cerr << "셰이더 소스 로드 실패: " << filePath << std::endl;
        return 0;
    }

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    delete[] shaderSource;

    glCompileShader(shader);

    // 컴파일 상태 확인
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        std::cerr << typeStr << " 셰이더 컴파일 실패:\n" << infoLog << std::endl;
        return 0;
    }

    std::cout << "셰이더 컴파일 성공: " << filePath << std::endl;
    return shader;
}

// 셰이더 프로그램을 링크하는 함수
GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // 링크 상태 확인
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "셰이더 프로그램 링크 실패:\n" << infoLog << std::endl;
        return 0;
    }

    std::cout << "셰이더 프로그램 링크 성공." << std::endl;
    return program;
}
