#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "open_file.h"    // 변경: file_open.h → open_file.h
#include "open_obj.h"     // 변경: read_obj.h → open_obj.h
#include <random>
#include <vector>

// 미리 선언할거
#define vertex_shader_code "vertex.glsl"
#define fragment_shader_code "fragment.glsl"
std::random_device rd;
std::mt19937 g(rd());

//------------------------------------------------------
// 콜백함수
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);

GLvoid timer(int);

//------------------------------------------------------
// 셰이더 용 선언
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;
GLuint VAO[4], VBO[4], EBO[4], a_axis, b_axis;

void make_vertex_shader();
void make_fragment_shader();
GLuint make_shader();
GLvoid init_buffer();
//------------------------------------------------------
// 전역변수
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

Model cube_15;
Model cone_15;
Model sphere_15;
Model cylinder_15;

int spin_flag = -1; // 1=+x 2=-x 3=+y 4=-y

int what_spin = 3;

const float axis[] = {
    0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, -1.0f, 0.0, 1.0f, 1.0f, 1.0f,

    -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

    0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f,
};

float axis_dx{};
float axis_dy{};

float raxis_dx{};
float raxis_dy{};

float rotate_dy{};

bool is_c = false;

//------------------------------------------------------
// 필요한 함수 선언

void print_model_info(const Model& model) {
    std::cout << "Total Vertices: " << model.vertex_count << std::endl;
    for (size_t i = 0; i < model.vertex_count; ++i) {
        std::cout << "Vertex " << i + 1 << ": ("
            << model.vertices[i].x << ", "
            << model.vertices[i].y << ", "
            << model.vertices[i].z << ")" << std::endl;
    }

    std::cout << "Total Faces: " << model.face_count << std::endl;
    for (size_t i = 0; i < model.face_count; ++i) {
        std::cout << "Face " << i + 1 << ": ("
            << model.faces[i].v1 + 1 << ", "
            << model.faces[i].v2 + 1 << ", "
            << model.faces[i].v3 + 1 << ")" << std::endl;
    }
    std::cout << "\n\n\n";
}

//------------------------------------------------------
int main(int argc, char** argv) { // 변경: void main → int main
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example1");

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    make_vertex_shader();
    make_fragment_shader();
    shader_program = make_shader();

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);

    glutTimerFunc(60, timer, 0);
    read_obj_file("cube.obj", &cube_15);
    read_obj_file("cone.obj", &cone_15);
    read_obj_file("sphere.obj", &sphere_15);
    read_obj_file("cylinder.obj", &cylinder_15);
    glEnable(GL_DEPTH_TEST);  // 깊이 테스트 활성화

    init_buffer();

    glutMainLoop();

    // 메모리 해제 (추가 권장 사항)
    free(cube_15.vertices);
    free(cube_15.faces);
    free(cone_15.vertices);
    free(cone_15.faces);
    free(sphere_15.vertices);
    free(sphere_15.faces);
    free(cylinder_15.vertices);
    free(cylinder_15.faces);

    return 0;
}

GLUquadricObj* qobj;

GLvoid drawScene(GLvoid) {

    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glValidateProgram(shader_program);
    GLuint modloc = glGetUniformLocation(shader_program, "trans");

    glBindVertexArray(a_axis);

    glm::mat4 temp(1.0f);

    temp = glm::rotate(temp, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
    temp = glm::rotate(temp, glm::radians(-30.0f), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modloc, 1, GL_FALSE, glm::value_ptr(temp));

    glDrawArrays(GL_LINES, 0, 12);

    glm::mat4 tr_mat = glm::mat4(1.0f);
    tr_mat = glm::rotate(tr_mat, glm::radians(rotate_dy), glm::vec3(0.0, 1.0, 0.0));
    tr_mat = glm::translate(tr_mat, glm::vec3(-0.5f, 0.0f, 0.0f));
    tr_mat = glm::rotate(tr_mat, glm::radians(30.0f + axis_dx), glm::vec3(1.0, 0.0, 0.0));
    tr_mat = glm::rotate(tr_mat, glm::radians(-30.0f + axis_dy), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modloc, 1, GL_FALSE, glm::value_ptr(tr_mat));

    if (!is_c) {
        glBindVertexArray(VAO[0]);
        for (int i = 0; i < (cube_15.face_count * 3) - 1; ++i)
        {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }
    else {
        glBindVertexArray(VAO[2]);
        for (int i = 0; i < (sphere_15.face_count * 3) - 1; ++i)
        {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }

    glm::mat4 tr_mat2 = glm::mat4(1.0f);
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(rotate_dy), glm::vec3(0.0, 1.0, 0.0));
    tr_mat2 = glm::translate(tr_mat2, glm::vec3(0.5f, 0.0f, 0.0f));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(30.0f + raxis_dx), glm::vec3(1.0, 0.0, 0.0));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(-30.0f + raxis_dy), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modloc, 1, GL_FALSE, glm::value_ptr(tr_mat2));
    if (!is_c) {
        glBindVertexArray(VAO[1]);
        for (int i = 0; i < (cone_15.face_count * 3); ++i) {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }
    else {
        glBindVertexArray(VAO[3]);
        for (int i = 0; i < (cylinder_15.face_count * 3) - 1; ++i)
        {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }

    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
        spin_flag = 1;
        break;

    case 'X':
        spin_flag = 2;
        break;

    case 'y':
        spin_flag = 3;
        break;

    case 'Y':
        spin_flag = 4;
        break;

    case 's':
        spin_flag = -1;
        axis_dx = 0;
        axis_dy = 0;
        rotate_dy = 0;
        what_spin = 3;
        break;

    case 'r':
        spin_flag = 5;
        break;

    case 'R':
        spin_flag = 6;
        break;

    case 'c':
        is_c = !is_c;
        break;

    case 'q':
        glutLeaveMainLoop();
        break;
    }
    if ('1' <= key && key <= '3') {
        what_spin = key - '0';
    }

    glutPostRedisplay();
}

void make_vertex_shader() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource;
    vertexSource = open_file_to_buf(vertex_shader_code); // 변경: file_to_buf → open_file_to_buf
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "컴파일 성공\n";
    }
}

void make_fragment_shader() {
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource;
    fragmentSource = open_file_to_buf(fragment_shader_code); // 변경: file_to_buf → open_file_to_buf
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "컴파일 성공\n";
    }
}

GLuint make_shader() {
    GLuint shader01;
    shader01 = glCreateProgram();
    glAttachShader(shader01, vertexShader);
    glAttachShader(shader01, fragmentShader);

    glLinkProgram(shader01);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shader01, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader01, 512, NULL, errorLog);
        std::cerr << "셰이더 생성 실패\n" << errorLog << std::endl;
    }
    else {
        std::cout << "\n컴파일 성공\n";

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    glUseProgram(shader01);

    return shader01;
}

GLvoid init_buffer() {
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cube_15.vertex_count * sizeof(Vertex), cube_15.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_15.face_count * sizeof(Face), cube_15.faces, GL_STATIC_DRAW);


    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, cone_15.vertex_count * sizeof(Vertex), cone_15.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cone_15.face_count * sizeof(Face), cone_15.faces, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO[2]);
    glBindVertexArray(VAO[2]);

    glGenBuffers(1, &VBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphere_15.vertex_count * sizeof(Vertex), sphere_15.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_15.face_count * sizeof(Face), sphere_15.faces, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO[3]);
    glBindVertexArray(VAO[3]);

    glGenBuffers(1, &VBO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, cylinder_15.vertex_count * sizeof(Vertex), cylinder_15.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder_15.face_count * sizeof(Face), cylinder_15.faces, GL_STATIC_DRAW);


    glGenVertexArrays(1, &a_axis);
    glBindVertexArray(a_axis);

    glGenBuffers(1, &b_axis);
    glBindBuffer(GL_ARRAY_BUFFER, b_axis);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLvoid timer(int value) {

    switch (spin_flag)
    {
    case 1:
        if (what_spin == 1 || what_spin == 3)
            axis_dx += 1.0;
        if (what_spin == 2 || what_spin == 3)
            raxis_dx += 1.0;

        break;

    case 2:
        if (what_spin == 1 || what_spin == 3)
            axis_dx -= 1.0;
        if (what_spin == 2 || what_spin == 3)
            raxis_dx -= 1.0;

        break;

    case 3:
        if (what_spin == 1 || what_spin == 3)
            axis_dy += 1.0;
        if (what_spin == 2 || what_spin == 3)
            raxis_dy += 1.0;
        break;

    case 4:
        if (what_spin == 1 || what_spin == 3)
            axis_dy -= 1.0;
        if (what_spin == 2 || what_spin == 3)
            raxis_dy -= 1.0;
        break;

    case 5:
        rotate_dy += 1.0;

        break;

    case 6:
        rotate_dy -= 1.0;

        break;

    default:

        break;
    }



    glutPostRedisplay();
    glutTimerFunc(60, timer, 0);
}
