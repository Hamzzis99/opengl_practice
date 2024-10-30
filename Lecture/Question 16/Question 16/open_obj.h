// open_obj.h
#pragma once
#ifndef OPEN_OBJ_H
#define OPEN_OBJ_H

#include <string>

// Vertex 구조체 예시
struct Vertex {
    float x, y, z;    // 위치
    float r, g, b;    // 색상
};

// Face 구조체 예시
struct Face {
    unsigned int v1, v2, v3; // 정점 인덱스
};

// Model 구조체 예시
struct Model {
    int vertex_count;
    Vertex* vertices;
    int face_count;
    Face* faces;
};

// OBJ 파일을 로드하는 함수
bool loadOBJ(const std::string& filepath, Model* model);

// 모델의 정점 정보를 콘솔에 출력하는 디버깅 함수
void printModelVertices(const Model& model);

#endif // OPEN_OBJ_H
