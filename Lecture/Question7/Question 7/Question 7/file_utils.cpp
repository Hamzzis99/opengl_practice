#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

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
