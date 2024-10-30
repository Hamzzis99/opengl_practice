// open_obj.h
#pragma once
#ifndef OPEN_OBJ_H
#define OPEN_OBJ_H

#include <string>

// Vertex ����ü ����
struct Vertex {
    float x, y, z;    // ��ġ
    float r, g, b;    // ����
};

// Face ����ü ����
struct Face {
    unsigned int v1, v2, v3; // ���� �ε���
};

// Model ����ü ����
struct Model {
    int vertex_count;
    Vertex* vertices;
    int face_count;
    Face* faces;
};

// OBJ ������ �ε��ϴ� �Լ�
bool loadOBJ(const std::string& filepath, Model* model);

// ���� ���� ������ �ֿܼ� ����ϴ� ����� �Լ�
void printModelVertices(const Model& model);

#endif // OPEN_OBJ_H
