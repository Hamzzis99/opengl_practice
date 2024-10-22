// obj_loader.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include "obj_loader.h"

bool Load_Object(const char* path, Model& model) {
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << path << " 파일을 열 수 없습니다." << std::endl;
        return false;
    }

    std::string line;
    std::vector<glm::vec3> tempVertices;

    while (std::getline(inFile, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            tempVertices.push_back(vertex);
        }
        else if (prefix == "f") {
            std::vector<unsigned int> faceIndices;
            std::string vertexStr;
            while (ss >> vertexStr) {
                size_t pos = vertexStr.find('/');
                if (pos != std::string::npos) {
                    vertexStr = vertexStr.substr(0, pos);
                }
                unsigned int index = std::stoi(vertexStr) - 1;
                faceIndices.push_back(index);
            }

            // 면이 삼각형인지 사각형인지 확인하고 삼각형으로 분할
            if (faceIndices.size() == 3) {
                // 삼각형 면인 경우 그대로 추가
                model.indices.insert(model.indices.end(), faceIndices.begin(), faceIndices.end());
            }
            else if (faceIndices.size() == 4) {
                // 사각형 면인 경우 두 개의 삼각형으로 분할
                // 삼각형 1
                model.indices.push_back(faceIndices[0]);
                model.indices.push_back(faceIndices[1]);
                model.indices.push_back(faceIndices[2]);
                // 삼각형 2
                model.indices.push_back(faceIndices[0]);
                model.indices.push_back(faceIndices[2]);
                model.indices.push_back(faceIndices[3]);
            }
            // 그 이상의 면은 필요에 따라 처리 (여기서는 제외)
        }
    }

    // 정점 데이터 구성
    for (const auto& pos : tempVertices) {
        Vertex vertex;
        vertex.position = pos;
        model.vertices.push_back(vertex);
    }

    return true;
}
