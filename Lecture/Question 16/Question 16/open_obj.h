#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ����ü ����
typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    unsigned int v1, v2, v3;
} Face;

typedef struct {
    Vertex* vertices;
    size_t vertex_count;
    Face* faces;
    size_t face_count;
} Model;

// �Լ� ����
void read_newline(char* str);
void open_obj(const char* filename, Model* model);
