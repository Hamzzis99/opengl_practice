#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <fstream>
#include <iterator>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

const int WIN_X = 10, WIN_Y = 10;
const int WIN_W = 800, WIN_H = 800;

const glm::vec3 background_rgb = glm::vec3(0.0f, 0.0f, 0.0f);
const std::vector<glm::vec3> colors = {
    {1,1,1},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 1.0, 0.0},
    {1.0, 0.0, 1.0},
    {0.0, 1.0, 1.0},
};

bool isCulling = true;
bool is_reverse = true;

GLfloat mx = 0.0f;
GLfloat my = 0.0f;

int framebufferWidth, framebufferHeight;
GLuint shaderProgramID;
GLuint cube_VAO, cube_VBO, cube_EBO;
GLuint tet_VAO, tet_VBO, tet_EBO;
GLuint cube_VBO_n, tet_VBO_n;
GLuint cube_VBO_uv, tet_VBO_uv;

std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

glm::vec3 cameraPos = { -2.0f,2.0f,2.0f };
glm::vec3 camera_rotate = { 0.0f,0.0f,0.0f };
bool camera_rotate_y = false;
float camera_y_seta = 0.5f;
bool camera_rotate_x = false;
float camera_x_seta = 0.5f;

glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool is_cube = true;

unsigned int textureIDs[6];
unsigned int sky_tex;

char* File_To_Buf(const char* file)
{
    ifstream in(file, ios::binary);
    if (!in) {
        cerr << file << " 파일을 열 수 없습니다." << endl;
        exit(1);
    }

    in.seekg(0, ios::end);
    std::streamoff len = in.tellg();
    long length = static_cast<long>(len); // 명시적 캐스팅
    in.seekg(0, ios::beg);

    char* buf = new char[length + 1];
    int cnt = 0;
    // 파일 끝까지 읽되, length 이상 읽지 않도록 주의
    while (cnt < length && in >> noskipws >> buf[cnt]) {
        cnt++;
    }
    buf[cnt] = '\0'; // 실제 읽은 만큼 문자열 종료

    return buf;
}

bool Load_Object(const char* path) {
    vertexIndices.clear();
    uvIndices.clear();
    normalIndices.clear();
    vertices.clear();
    uvs.clear();
    normals.clear();

    ifstream in(path);
    if (!in) {
        cerr << path << " 파일을 찾을 수 없습니다." << endl;
        exit(1);
    }

    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") {
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (lineHeader == "vt") {
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (lineHeader == "vn") {
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (lineHeader == "f") {
            char a;
            unsigned int vertexIndex_[3], uvIndex_[3], normalIndex_[3];
            for (int i = 0; i < 3; i++) {
                in >> vertexIndex_[i] >> a >> uvIndex_[i] >> a >> normalIndex_[i];
                vertexIndices.push_back(vertexIndex_[i] - 1);
                uvIndices.push_back(uvIndex_[i] - 1);
                normalIndices.push_back(normalIndex_[i] - 1);
            }
        }
    }
    return true;
}

bool Make_Shader_Program() {
    const GLchar* vertexShaderSource = File_To_Buf("vertex.glsl");
    const GLchar* fragmentShaderSource = File_To_Buf("fragment.glsl");

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
        return false;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
        return false;
    }

    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
        return false;
    }

    glUseProgram(shaderProgramID);
    GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

    return true;
}

bool Load_Textures() {
    const char* textureFiles[6] = {
        "A.bmp",
        "B.bmp",
        "C.bmp",
        "D.bmp",
        "E.bmp",
        "F.bmp"
    };

    stbi_set_flip_vertically_on_load(true);

    glGenTextures(6, textureIDs);

    for (int i = 0; i < 6; i++) {
        int width, height, nrChannels;
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned char* data = stbi_load(textureFiles[i], &width, &height, &nrChannels, STBI_rgb);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            cerr << "Failed to load texture: " << textureFiles[i] << endl;
        }
        stbi_image_free(data);
    }

    glGenTextures(1, &sky_tex);
    int width, height, nrChannels;
    glBindTexture(GL_TEXTURE_2D, sky_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* sky_data = stbi_load("sky.png", &width, &height, &nrChannels, STBI_rgb);
    if (sky_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, sky_data);
    } else {
        cerr << "Failed to load texture: sky.png" << endl;
    }
    stbi_image_free(sky_data);

    return true;
}

void RebuildMeshData(
    const std::vector<glm::vec3>& in_vertices,
    const std::vector<glm::vec2>& in_uvs,
    const std::vector<glm::vec3>& in_normals,
    const std::vector<unsigned int>& in_vertexIndices,
    const std::vector<unsigned int>& in_uvIndices,
    const std::vector<unsigned int>& in_normalIndices,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals
) {
    out_vertices.resize(in_vertexIndices.size());
    out_uvs.resize(in_vertexIndices.size());
    out_normals.resize(in_vertexIndices.size());

    for (size_t i = 0; i < in_vertexIndices.size(); i++) {
        out_vertices[i] = in_vertices[in_vertexIndices[i]];
        out_uvs[i] = in_uvs[in_uvIndices[i]];
        out_normals[i] = in_normals[in_normalIndices[i]];
    }
}

bool Set_VAO() {
    // cube.obj
    Load_Object("cube.obj");
    if(!Load_Textures()) {
        cerr << "Error loading textures" << endl;
        return false;
    }

    std::vector<glm::vec3> cube_vertices_final;
    std::vector<glm::vec2> cube_uvs_final;
    std::vector<glm::vec3> cube_normals_final;
    RebuildMeshData(vertices, uvs, normals, vertexIndices, uvIndices, normalIndices,
        cube_vertices_final, cube_uvs_final, cube_normals_final);

    for (auto& uv : cube_uvs_final) {
        float inset = 0.05f;
        uv.x = uv.x * (1.0f - 2.0f * inset) + inset;
        uv.y = uv.y * (1.0f - 2.0f * inset) + inset;
    }

    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    glGenBuffers(1, &cube_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices_final.size() * sizeof(glm::vec3), &cube_vertices_final[0], GL_STATIC_DRAW);

    GLint cubePositionAttribute = glGetAttribLocation(shaderProgramID, "positionAttribute");
    glVertexAttribPointer(cubePositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(cubePositionAttribute);

    glGenBuffers(1, &cube_VBO_n);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO_n);
    glBufferData(GL_ARRAY_BUFFER, cube_normals_final.size() * sizeof(glm::vec3), &cube_normals_final[0], GL_STATIC_DRAW);

    GLint cubeNormalAttribute = glGetAttribLocation(shaderProgramID, "normalAttribute");
    glVertexAttribPointer(cubeNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(cubeNormalAttribute);

    glGenBuffers(1, &cube_VBO_uv);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO_uv);
    glBufferData(GL_ARRAY_BUFFER, cube_uvs_final.size() * sizeof(glm::vec2), &cube_uvs_final[0], GL_STATIC_DRAW);

    GLint cubeUvAttribute = glGetAttribLocation(shaderProgramID, "uvAttribute");
    glVertexAttribPointer(cubeUvAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(cubeUvAttribute);

    std::vector<unsigned int> cube_indices(cube_vertices_final.size());
    for (unsigned int i = 0; i < cube_indices.size(); i++) cube_indices[i] = i;

    glGenBuffers(1, &cube_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_indices.size() * sizeof(unsigned int), &cube_indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);


    // tetrahedron.obj
    Load_Object("tetrahedron.obj");
    std::vector<glm::vec3> tet_vertices_final;
    std::vector<glm::vec2> tet_uvs_final;
    std::vector<glm::vec3> tet_normals_final;
    RebuildMeshData(vertices, uvs, normals, vertexIndices, uvIndices, normalIndices,
        tet_vertices_final, tet_uvs_final, tet_normals_final);

    // UV 상하 반전
    for (auto& uv : tet_uvs_final) {
        uv.y = 1.0f - uv.y;
    }

    // UV 좌우 반전
    for (auto& uv : tet_uvs_final) {
        uv.x = 1.0f - uv.x;
    }

    // UV inset
    for (auto& uv : tet_uvs_final) {
        float inset = 0.05f;
        uv.x = uv.x * (1.0f - 2.0f * inset) + inset;
        uv.y = uv.y * (1.0f - 2.0f * inset) + inset;
    }

    glGenVertexArrays(1, &tet_VAO);
    glBindVertexArray(tet_VAO);

    glGenBuffers(1, &tet_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tet_VBO);
    glBufferData(GL_ARRAY_BUFFER, tet_vertices_final.size() * sizeof(glm::vec3), &tet_vertices_final[0], GL_STATIC_DRAW);

    GLint tetPositionAttribute = glGetAttribLocation(shaderProgramID, "positionAttribute");
    glVertexAttribPointer(tetPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tetPositionAttribute);

    glGenBuffers(1, &tet_VBO_n);
    glBindBuffer(GL_ARRAY_BUFFER, tet_VBO_n);
    glBufferData(GL_ARRAY_BUFFER, tet_normals_final.size() * sizeof(glm::vec3), &tet_normals_final[0], GL_STATIC_DRAW);

    GLint tetNormalAttribute = glGetAttribLocation(shaderProgramID, "normalAttribute");
    glVertexAttribPointer(tetNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tetNormalAttribute);

    glGenBuffers(1, &tet_VBO_uv);
    glBindBuffer(GL_ARRAY_BUFFER, tet_VBO_uv);
    glBufferData(GL_ARRAY_BUFFER, tet_uvs_final.size() * sizeof(glm::vec2), &tet_uvs_final[0], GL_STATIC_DRAW);

    GLint tetUvAttribute = glGetAttribLocation(shaderProgramID, "uvAttribute");
    glVertexAttribPointer(tetUvAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(tetUvAttribute);

    std::vector<unsigned int> tet_indices(tet_vertices_final.size());
    for (unsigned int i = 0; i < tet_indices.size(); i++) tet_indices[i] = i;

    glGenBuffers(1, &tet_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tet_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tet_indices.size() * sizeof(unsigned int), &tet_indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return true;
}

void Viewport1() {
    glUseProgram(shaderProgramID);
    unsigned viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "transform");
    unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "colorAttribute");

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(cube_VAO);

    glm::mat4 bgProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(bgProjection));

    glm::mat4 bgView = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(bgView));

    glm::mat4 bgTR = glm::mat4(1.0f);
    bgTR = glm::translate(bgTR, glm::vec3(0.0f, 0.0f, -2.0f));
    bgTR = glm::scale(bgTR, glm::vec3(2.0f));

    glUniform3f(colorLocation, 1, 1, 1);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(bgTR));
    glBindTexture(GL_TEXTURE_2D, sky_tex);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)(0));

    glEnable(GL_DEPTH_TEST);

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
    projection = glm::translate(projection, glm::vec3(0.0, 0.0, -5.0));

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 TR = glm::mat4(1.0f);
    TR = glm::rotate(TR, glm::radians(camera_rotate.y), glm::vec3(0.0, 1.0, 0.0));
    TR = glm::rotate(TR, glm::radians(camera_rotate.x), glm::vec3(1.0, 0.0, 0.0));

    if (is_cube) {
        glBindVertexArray(cube_VAO);

        for (int i = 0; i < 6; i++) {
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
        }
    }
    else {
        glBindVertexArray(tet_VAO);
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        glBindTexture(GL_TEXTURE_2D, textureIDs[4]); // E
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        glBindTexture(GL_TEXTURE_2D, textureIDs[3]); // D
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

        glBindTexture(GL_TEXTURE_2D, textureIDs[2]); // C
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(9 * sizeof(unsigned int)));

        glBindTexture(GL_TEXTURE_2D, textureIDs[1]); // B
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        glBindTexture(GL_TEXTURE_2D, textureIDs[0]); // A
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(15 * sizeof(unsigned int)));
    }
}

GLvoid drawScene()
{
    glClearColor(background_rgb.x, background_rgb.y, background_rgb.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    isCulling ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    Viewport1();

    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, WIN_W, WIN_H);
}

GLvoid TimerFunction1(int value)
{
    glutPostRedisplay();
    if (camera_rotate_y) {
        camera_rotate.y += camera_y_seta;
    }
    if (camera_rotate_x) {
        camera_rotate.x += camera_x_seta;
    }
    glutTimerFunc(10, TimerFunction1, 1);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'y':
        camera_rotate_y = !camera_rotate_y;
        camera_y_seta = 1;
        break;
    case 'Y':
        camera_rotate_y = !camera_rotate_y;
        camera_y_seta = -1;
        break;
    case 'x':
        camera_rotate_x = !camera_rotate_x;
        camera_x_seta = 1;
        break;
    case 'X':
        camera_rotate_x = !camera_rotate_x;
        camera_x_seta = -1;
        break;
    case 'n':
        is_cube = !is_cube;
        break;
    case 'q':
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(WIN_X, WIN_Y);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Example1");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cerr << "Unable to initialize GLEW" << endl;
        exit(EXIT_FAILURE);
    }
    else
        cout << "GLEW Initialized\n";

    if (!Make_Shader_Program()) {
        cerr << "Error: Shader Program 생성 실패" << endl;
        exit(EXIT_FAILURE);
    }

    if (!Set_VAO()) {
        cerr << "Error: VAO 생성 실패" << endl;
        exit(EXIT_FAILURE);
    }

    glutTimerFunc(10, TimerFunction1, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();

    return 0;
}
