// open_obj.cpp

#include "open_obj.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// OBJ 파일을 로드하는 함수 구현 (예시)
bool loadOBJ(const std::string& filepath, Model* model) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Unable to open file " << filepath << std::endl;
        return false;
    }

    // 임시 저장소
    std::vector<Vertex> temp_vertices;
    std::vector<Face> temp_faces;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            // 기본 색상 할당 (흰색)
            vertex.r = 1.0f;
            vertex.g = 1.0f;
            vertex.b = 1.0f;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "f") {
            Face face;
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            // OBJ 파일의 정점 인덱스는 1부터 시작하므로 0으로 조정
            face.v1 = std::stoi(v1) - 1;
            face.v2 = std::stoi(v2) - 1;
            face.v3 = std::stoi(v3) - 1;
            temp_faces.push_back(face);
        }
        // 기타 필요한 OBJ 요소 처리 가능
    }

    file.close();

    // 모델에 데이터 할당
    model->vertex_count = static_cast<int>(temp_vertices.size());
    model->vertices = new Vertex[model->vertex_count];
    for (int i = 0; i < model->vertex_count; ++i) {
        model->vertices[i] = temp_vertices[i];
    }

    model->face_count = static_cast<int>(temp_faces.size());
    model->faces = new Face[model->face_count];
    for (int i = 0; i < model->face_count; ++i) {
        model->faces[i] = temp_faces[i];
    }

    return true;
}

// 모델의 정점 정보를 콘솔에 출력하는 디버깅 함수 구현
void printModelVertices(const Model& model) {
    std::cout << "Model Vertex Count: " << model.vertex_count << std::endl;
    for (int i = 0; i < model.vertex_count; ++i) {
        const Vertex& v = model.vertices[i];
        std::cout << "Vertex " << i + 1 << ": ("
            << v.x << ", " << v.y << ", " << v.z << ") - Color: ("
            << v.r << ", " << v.g << ", " << v.b << ")"
            << std::endl;
    }

    std::cout << "Model Face Count: " << model.face_count << std::endl;
    for (int i = 0; i < model.face_count; ++i) {
        const Face& f = model.faces[i];
        std::cout << "Face " << i + 1 << ": ("
            << f.v1 + 1 << ", " << f.v2 + 1 << ", " << f.v3 + 1 << ")"
            << std::endl;
    }
}
