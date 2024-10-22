#pragma once
// obj_loader.h
#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

bool Load_Object(const char* path, Model& model);

#endif
