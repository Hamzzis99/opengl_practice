#include "open_file.h"

char* open_file(const char* file_name) {  
    FILE* file;
    fopen_s(&file, file_name, "r");
    if (!file) {
        printf("Failed to load shader file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* shaderSource = (char*)malloc(length + 1);
    size_t read_size = fread(shaderSource, 1, length, file);
    shaderSource[read_size] = '\0';

    fclose(file);

    return shaderSource;
}