// obj_loader.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include "obj_loader.h"

bool Load_Object(const char* path, Model& model) {
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << path << " ������ �� �� �����ϴ�." << std::endl;
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

            // ���� �ﰢ������ �簢������ Ȯ���ϰ� �ﰢ������ ����
            if (faceIndices.size() == 3) {
                // �ﰢ�� ���� ��� �״�� �߰�
                model.indices.insert(model.indices.end(), faceIndices.begin(), faceIndices.end());
            }
            else if (faceIndices.size() == 4) {
                // �簢�� ���� ��� �� ���� �ﰢ������ ����
                // �ﰢ�� 1
                model.indices.push_back(faceIndices[0]);
                model.indices.push_back(faceIndices[1]);
                model.indices.push_back(faceIndices[2]);
                // �ﰢ�� 2
                model.indices.push_back(faceIndices[0]);
                model.indices.push_back(faceIndices[2]);
                model.indices.push_back(faceIndices[3]);
            }
            // �� �̻��� ���� �ʿ信 ���� ó�� (���⼭�� ����)
        }
    }

    // ���� ������ ����
    for (const auto& pos : tempVertices) {
        Vertex vertex;
        vertex.position = pos;
        model.vertices.push_back(vertex);
    }

    return true;
}
