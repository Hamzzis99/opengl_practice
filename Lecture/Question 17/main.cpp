#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>
#include "open_file.h"
#include "open_obj.h"

// ���̴� ����
#define vertex_shader_glsl "vertex.glsl"
#define fragment_shader_glsl "fragment.glsl"

// random �Լ���
std::random_device rd;
std::default_random_engine dre(rd());

//-------------------------------- �ݹ� �Լ� ����
GLvoid drawScene(GLvoid);
// ȭ�鿡 �׸� ����� �����ϰ� �������ϴ� �Լ��Դϴ�. �ַ� OpenGL ����� ����Ͽ� �׷��� ��Ҹ� �׸��� ������ �մϴ�.

GLvoid Reshape(int w, int h);
// â ũ�Ⱑ ������ �� ȣ��Ǵ� �Լ��Դϴ�. 'w'�� �� â�� �ʺ�, 'h'�� �� â�� ���̸� ��Ÿ����, ����Ʈ�� �����ϰų� ���� ����� �����ϴ� �� ���˴ϴ�.

GLvoid Keyboard(unsigned char key, int x, int y);
// Ű���� �Է��� �߻����� �� ȣ��Ǵ� �Լ��Դϴ�. 'key'�� ���� Ű�� ��Ÿ����, 'x'�� 'y'�� Ű �Է��� �߻��� ���콺 ��ġ ��ǥ�� ��Ÿ���ϴ�.

GLvoid SpecialKeyboard(int key, int x, int y);
// Ư�� Ű(��: ȭ��ǥ Ű, F1~F12 ��)�� ������ �� ȣ��Ǵ� �Լ��Դϴ�. 'key'�� ���� Ư�� Ű�� �ڵ��̰�, 'x'�� 'y'�� Ű �Է��� �߻��� ���콺 ��ġ ��ǥ�Դϴ�.

GLvoid timer(int);
// Ÿ�̸� �ݹ� �Լ���, ������ �������� ����ǵ��� ������ �� �ֽ��ϴ�. 'int' �Ű������� Ÿ�̸ӿ� ���õ� ���� �����ϴ� �� ���� �� �ֽ��ϴ�.

// ���̴� ���� ���� ����
GLuint shader_program;
// ���̴� ���α׷� ��ü�Դϴ�. �� ���α׷��� ���ؽ� ���̴��� �����׸�Ʈ ���̴��� �����Ͽ� GPU���� ����� ��ü ���̴� ���α׷��� ��Ÿ���ϴ�.

GLuint vertexShader;
// ���ؽ� ���̴� ��ü�Դϴ�. ���ؽ�(����) ���� �۾��� ó���ϴ� �ڵ尡 ���ԵǸ�, ���̴� ���α׷��� �����ϵǰ� ����˴ϴ�.

GLuint fragmentShader;
// �����׸�Ʈ ���̴� ��ü�Դϴ�. �����׸�Ʈ(�ȼ�) ���� �۾��� ó���ϴ� �ڵ尡 ���ԵǸ�, ���̴� ���α׷��� �����ϵǰ� ����˴ϴ�.

GLuint VAO[2], VBO[2], CBO[2], EBO[2], a_axis, b_axis;
// VAO (Vertex Array Object): ���� �迭 ��ü�Դϴ�. ���� �����͸� �����ϰ� OpenGL���� ȿ������ ���� ��ȯ�� ���� ���˴ϴ�.
// VBO (Vertex Buffer Object): ���� ���� ��ü�Դϴ�. ���� ��ġ �����͸� �����ϴ� �� ���˴ϴ�.
// CBO (Color Buffer Object): ���� ���� ��ü�Դϴ�. ������ ���� �����͸� �����ϴ� �� ���˴ϴ�.
// EBO (Element Buffer Object): ��� ���� ��ü�Դϴ�. �ε��� ������(������ �׸��� ����)�� �����Ͽ� ���� �����͸� �����ϰ�, �޸� ȿ������ ���Դϴ�.

// �Լ� ����
void make_vertex_shader();
void make_fragment_shader();
GLuint make_shader();
GLvoid init_buffer();

// RGB���� �ػ�. (����)
GLclampf base_r = 0.0f;
GLclampf base_g = 0.0f;
GLclampf base_b = 0.0f;
GLint width{ 800 }, height{ 600 };

Model cube;
Model pyramid;

//------------------------ Bool ������ (������ ������ ���� �̸�)
bool shouldDrawCube = false;           // ť�긦 �׷��� �ϴ��� ����
bool shouldDrawPyramid = false;        // �Ƕ�̵带 �׷��� �ϴ��� ����
bool isWireframeMode = false;          // ���̾������� ��� Ȱ��ȭ ����
bool isFaceCullingEnabled = false;     // �� ����(Face Culling) Ȱ��ȭ ����
bool isSpinning = false;               // ȸ�� �ִϸ��̼� Ȱ��ȭ ����
bool showTopFace = false;              // ��� �� ǥ�� ����
bool showFrontFace = false;            // ���� �� ǥ�� ����
bool showSideFace = false;             // ���� �� ǥ�� ����
bool showBackFace = false;             // �ĸ� �� ǥ�� ����
bool isObjectOpen = false;             // ��ü ���� ���� ���� (��: �� ���� ��)
bool isOpenRotationEnabled = false;    // ���� ���¿����� ȸ�� Ȱ��ȭ ����
int animation_flag = -1;                // �ִϸ��̼� ���� �÷��� (�������� ����)
bool usePerspectiveProjection = false;  // ���� ���� ��� ����

bool pyramid1_flag = false;
bool pyramid2_flag = false;
glm::mat4 translate_mat(1.0f); // ��� �ʱ�ȭ (x, y, z, 1)

// ���� �迭
const float cube_color[] = {
    1.0f, 0.0f, 0.0f, // Red
    0.0f, 1.0f, 0.0f, // Green
    0.0f, 0.0f, 1.0f, // Blue
    1.0f, 1.0f, 0.0f, // Yellow
    0.0f, 1.0f, 1.0f, // Cyan
    1.0f, 0.0f, 1.0f, // Magenta
    0.0f, 0.0f, 0.0f, // Black
    1.0f, 1.0f, 1.0f  // White
};

const float pyramid_color[] = {
    1.0f, 0.0f, 0.0f, // Red
    0.0f, 1.0f, 0.0f, // Green
    0.0f, 0.0f, 1.0f, // Blue
    1.0f, 1.0f, 0.0f, // Yellow
    0.0f, 1.0f, 1.0f  // Cyan
};

const float axis[] = {
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // X�� Red

    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Y�� Green

    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f  // Z�� Blue
};

float axis_dx{};
float axis_dy{};

float dx{};
float dy{};

// ����ü ����
float face_dx[6];
float face_dy[6];

float face_rotatex[6];
float face_rotatey[6];
float face_rotatez[6];
float face_scale[6]{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

float for_face_rotate_dx{};
float for_face_rotate_dy{};

// �Ƕ�̵� ����
float pyramid_rotate_x[4];
float pyramid_rotate_y[4];
float pyramid_rotate_z[4];

float pyramid_dx[4];
float pyramid_dy[4];
float pyramid_dz[4];

int face; // �̰� ����

// main �Լ�
int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ �� ������ ����
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example 17");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW �ʱ�ȭ ����" << std::endl;
        return -1;
    }

    // ���̴� ���� �� ���α׷� ��ũ
    make_vertex_shader();
    make_fragment_shader();
    shader_program = make_shader();

    // �ݹ� �Լ� ���
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(10, timer, 0);

    // OBJ ���� �б�
    read_obj_file("cube.obj", &cube);
    read_obj_file("pyramid.obj", &pyramid);

    // ���� �׽�Ʈ Ȱ��ȭ
    glEnable(GL_DEPTH_TEST);

    // ���� �ʱ�ȭ
    init_buffer();

    // ���� ���� ����
    glutMainLoop();

    return 0;
}

GLUquadricObj* qobj; // �� �𸣴°� �̵� ������

// �ݹ� �Լ� ����

GLvoid drawScene(GLvoid) {
    // ��� ���� ���� �� ���� �ʱ�ȭ
    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ȱ��ȭ�� ���̴� ���α׷��� �����մϴ�.
    glUseProgram(shader_program);

    // 'trans'��� �̸��� uniform ������ ��ġ�� �����ɴϴ�.
    GLuint modelLoc2 = glGetUniformLocation(shader_program, "trans");
    // 'trans_by_face'��� �̸��� uniform ������ ��ġ�� �����ɴϴ�.
    GLuint facloc = glGetUniformLocation(shader_program, "trans_by_face");
    // �ٽ� �� �� 'trans'��� �̸��� uniform ������ ��ġ�� �����ɴϴ�.
    // ����: �̹� 'trans'�� ��ġ�� 'modelLoc2'�� ���������Ƿ�, 
    // �� ���� �ߺ��� �� �ֽ��ϴ�. ���� �ٸ� �������� ���ȴٸ� �̸��� �ٸ��� �ϴ� ���� �����ϴ�.
    GLuint modloc = glGetUniformLocation(shader_program, "trans");

    // �� VAO ���ε�
    glBindVertexArray(a_axis);

    // ������ ���� ����� ���̴� ���α׷��� 'trans_by_face' uniform ������ �����մϴ�.
    glm::mat4 temp(1.0f);
    glUniformMatrix4fv(facloc, 1, GL_FALSE, glm::value_ptr(temp));

    // �� ȸ�� ����
    temp = glm::rotate(temp, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
    temp = glm::rotate(temp, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));

    // �� �׸��� (��)
    glDrawArrays(GL_LINES, 0, 12); // ���� �迭�� ����Ͽ� �� �׸���

    // �� ����(Face Culling) ����
    if (isFaceCullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    // �������� ��� ����
    glm::mat4 proj1 = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f);
    glm::mat4 proj2 = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
    proj2 = glm::translate(proj2, glm::vec3(0.0f, 0.0f, -5.0f));
    GLuint projection = glGetUniformLocation(shader_program, "projection");
    if (usePerspectiveProjection) {
        glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(proj2));
    }
    else {
        glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(proj1));
    }

    // ����ü�� �� �鿡 ���� ��ȯ ��� ���
    glm::mat4 cube_face_trans[6] = {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };
    for (int i = 0; i < 6; ++i) {
        // ��Ŀ� ���(������) ����
        cube_face_trans[i] = glm::scale(
            cube_face_trans[i],
            glm::vec3(face_scale[i], face_scale[i], face_scale[i])
        );

        // ��Ŀ� x�� �� y�� �������� �̵�(Ʈ�������̼�) ����
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(face_dx[i], face_dy[i], 0.0f)
        );

        // ��Ŀ� y���� �߽����� ȸ�� ����
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatey[i]),
            glm::vec3(0.0, 1.0, 0.0)
        );

        // ��Ŀ� ��ü ȸ���� ���� �߰� �̵�(Ʈ�������̼�) ����
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(0.0f, -for_face_rotate_dy, -for_face_rotate_dx)
        );

        // ��Ŀ� x���� �߽����� ȸ�� ����
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatex[i]),
            glm::vec3(1.0, 0.0, 0.0)
        );

        // ��Ŀ� z���� �߽����� ȸ�� ����
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatez[i]),
            glm::vec3(0.0, 0.0, 1.0)
        );

        // ��Ŀ� ��ü ȸ���� ���� �߰� �̵�(Ʈ�������̼�) ����
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(0.0f, for_face_rotate_dy, for_face_rotate_dx)
        );
    }

    // ť�� VAO ���ε�
    glBindVertexArray(VAO[0]);
    if (shouldDrawCube) {
        for (int i = 0; i < cube.face_count / 2; ++i) {
            glUniformMatrix4fv(facloc, 1, GL_FALSE, glm::value_ptr(cube_face_trans[i]));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 6));
        }
    }

    // ��ü �̵� �� ȸ�� ��� ����
    glm::mat4 tr_mat2 = glm::mat4(1.0f);
    tr_mat2 = glm::translate(tr_mat2, glm::vec3(dx, dy, 0.0f));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(30.0f + axis_dx), glm::vec3(1.0, 0.0, 0.0));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(30.0f + axis_dy), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modloc, 1, GL_FALSE, glm::value_ptr(tr_mat2));

    // �Ƕ�̵��� �� �鿡 ���� ��ȯ ��� ���
    glm::mat4 pyramid_face[4] = {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };
    for (int i = 0; i < 4; ++i) {
        pyramid_face[i] = glm::translate(
            pyramid_face[i],
            glm::vec3(-pyramid_dx[i], -pyramid_dy[i], -pyramid_dz[i])
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_x[i]),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_y[i]),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_z[i]),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        pyramid_face[i] = glm::translate(
            pyramid_face[i],
            glm::vec3(pyramid_dx[i], pyramid_dy[i], pyramid_dz[i])
        );
    }

    glm::mat4 p_temp = glm::mat4(1.0f);
    GLuint pyramid_mat = glGetUniformLocation(shader_program, "trans_by_face");
    glBindVertexArray(VAO[1]);
    if (shouldDrawPyramid) {
        for (int i = 0; i < pyramid.face_count; ++i) {
            // 2�� ����, 3��, 4��, 5��
            if (i > 1)
            {
                glUniformMatrix4fv(pyramid_mat, 1, GL_FALSE, glm::value_ptr(pyramid_face[i - 2]));
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 3));
            }
            else {
                glUniformMatrix4fv(pyramid_mat, 1, GL_FALSE, glm::value_ptr(p_temp));
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 3));
            }
        }
    }

    // ���� ���۸� ����
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c':
        shouldDrawCube = !shouldDrawCube;
        break;

    case 'v':
        shouldDrawPyramid = !shouldDrawPyramid;
        break;

    case 'p':
        usePerspectiveProjection = !usePerspectiveProjection;
        break;

    case 'y':
        isSpinning = !isSpinning;
        break;
    case 't':
        showTopFace = !showTopFace;
        break;

    case 'f':
        showFrontFace = !showFrontFace;
        break;

    case 's':
        showSideFace = !showSideFace;
        break;

    case 'b':
        showBackFace = !showBackFace;
        break;

    case 'h':
        isFaceCullingEnabled = !isFaceCullingEnabled;
        break;

    case 'o':
        pyramid1_flag = !pyramid1_flag; // ����� ������ ���
        break;

    case 'r':
        pyramid2_flag = !pyramid2_flag; // ����� ������ ���
        break;

    case 'q':
        glutLeaveMainLoop();
        break;

    case '1':
        face_scale[1] -= 0.1f;
        break;

    case '2':
        // �߰� ������ �ʿ��� ��� ���⿡ �ۼ�
        break;
    }

    if ('1' <= key && key <= '6') {
        face = key - '1';
    }

    glutPostRedisplay();
}

void make_vertex_shader() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource;
    vertexSource = open_file_to_buf(vertex_shader_glsl);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "������ ����\n";
    }
}

void make_fragment_shader() {
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource;
    fragmentSource = open_file_to_buf(fragment_shader_glsl);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "������ ����\n";
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
        std::cerr << "���̴� ���� ����\n" << errorLog << std::endl;
    }
    else {
        std::cout << "\n������ ����\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    glUseProgram(shader01);

    return shader01;
}

// Init_buffer �Լ��� ���� 3D ��ü(ť��, �Ƕ�̵�, ��)�� ���� ���� ������ �����մϴ�.
GLvoid init_buffer() {
    // ť�� VAO ���� �� ���ε�
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    // ť�� VBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cube.vertex_count * sizeof(Vertex), cube.vertices, GL_STATIC_DRAW);

    // ���� �Ӽ� ���� (��ġ)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // ť�� EBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.face_count * sizeof(Face), cube.faces, GL_STATIC_DRAW);

    // ť�� CBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &CBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, CBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_color), cube_color, GL_STATIC_DRAW);

    // ���� �Ӽ� ���� (����)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(1);

    // �Ƕ�̵� VAO ���� �� ���ε�
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    // �Ƕ�̵� VBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, pyramid.vertex_count * sizeof(Vertex), pyramid.vertices, GL_STATIC_DRAW);

    // ���� �Ӽ� ���� (��ġ)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // �Ƕ�̵� CBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &CBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, CBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_color), pyramid_color, GL_STATIC_DRAW);

    // ���� �Ӽ� ���� (����)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(1);

    // �Ƕ�̵� EBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pyramid.face_count * sizeof(Face), pyramid.faces, GL_STATIC_DRAW);

    // �� VAO ���� �� ���ε�
    glGenVertexArrays(1, &a_axis);
    glBindVertexArray(a_axis);

    // �� VBO ����, ���ε� �� ������ ���ε�
    glGenBuffers(1, &b_axis);
    glBindBuffer(GL_ARRAY_BUFFER, b_axis);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

    // ���� �Ӽ� ���� (��ġ)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);

    // ���� �Ӽ� ���� (����)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLvoid SpecialKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        dy += 0.1f;
        break;

    case GLUT_KEY_DOWN:
        dy -= 0.1f;
        break;
    case GLUT_KEY_LEFT:
        dx -= 0.1f;
        break;
    case GLUT_KEY_RIGHT:
        dx += 0.1f;
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

GLvoid timer(int value) {
    if (isSpinning) {
        axis_dy += 5.0f;
    }
    if (showTopFace) {
        face_rotatey[2] += 2.0f;
    }
    if (showBackFace) {
        if (face_scale[0] > 0.0f) {
            face_scale[0] -= 0.1f;
        }
    }
    else {
        if (face_scale[0] < 1.0f) {
            face_scale[0] += 0.1f;
        }
    }

    if (showSideFace) {
        if (face_dy[1] < 0.5f && face_dy[3] < 0.5f) {
            face_dy[1] += 0.1f;
            face_dy[3] += 0.1f;
        }
    }
    else {
        if (face_dy[1] > 0.1f && face_dy[3] > 0.1f) {
            face_dy[1] -= 0.1f;
            face_dy[3] -= 0.1f;
        }
    }
    if (showFrontFace) {
        for_face_rotate_dx = -0.25f;
        for_face_rotate_dy = 0.25f;
        if (face_rotatex[5] < 90.0f) {
            face_rotatex[5] += 5.0f;
        }
    }
    else {
        for_face_rotate_dx = -0.25f;
        for_face_rotate_dy = 0.25f;
        if (face_rotatex[5] > 0.0f) {
            face_rotatex[5] -= 5.0f;
        }
    }

    if (pyramid1_flag) {
        if (pyramid_rotate_z[0] < 233 && pyramid_rotate_x[1] < 233 && pyramid_rotate_z[2] > -233 && pyramid_rotate_x[3] > -233) {
            pyramid_dx[0] = 0.25f;
            pyramid_dy[0] = 0.25f;
            pyramid_rotate_z[0] += 1.0f;

            pyramid_dz[1] = -0.25f;
            pyramid_dy[1] = 0.25f;
            pyramid_rotate_x[1] += 1.0f;

            pyramid_dx[2] = -0.25f;
            pyramid_dy[2] = 0.25f;
            pyramid_rotate_z[2] -= 1.0f;

            pyramid_dz[3] = 0.25f;
            pyramid_dy[3] = 0.25f;
            pyramid_rotate_x[3] -= 1.0f;
        }
    }
    else if (!pyramid2_flag) {
        if (pyramid_rotate_z[0] > 0 && pyramid_rotate_x[1] > 0 && pyramid_rotate_z[2] < 0 && pyramid_rotate_x[3] < 0) {
            pyramid_rotate_z[0] -= 1.0f;
            pyramid_rotate_x[1] -= 1.0f;
            pyramid_rotate_z[2] += 1.0f;
            pyramid_rotate_x[3] += 1.0f;
        }
    }
    if (pyramid2_flag) {
        if (pyramid_rotate_z[0] < 90) {
            pyramid_dx[0] = 0.25f;
            pyramid_dy[0] = 0.25f;
            pyramid_rotate_z[0] += 1.0f;
        }
        if (pyramid_rotate_z[0] > 85 && pyramid_rotate_x[1] < 90) {
            pyramid_dz[1] = -0.25f;
            pyramid_dy[1] = 0.25f;
            pyramid_rotate_x[1] += 1.0f;
        }
        if (pyramid_rotate_x[1] > 85 && pyramid_rotate_z[2] > -90) {
            pyramid_dx[2] = -0.25f;
            pyramid_dy[2] = 0.25f;
            pyramid_rotate_z[2] -= 1.0f;
        }
        if (pyramid_rotate_z[2] < -85 && pyramid_rotate_x[3] > -90) {
            pyramid_dz[3] = 0.25f;
            pyramid_dy[3] = 0.25f;
            pyramid_rotate_x[3] -= 1.0f;
        }
    }
    else if (!pyramid1_flag) {
        if (pyramid_rotate_z[0] > 0) {
            pyramid_rotate_z[0] -= 1.0f;
        }
        if (pyramid_rotate_x[1] > 0) {
            pyramid_rotate_x[1] -= 1.0f;
        }
        if (pyramid_rotate_z[2] < 0) {
            pyramid_rotate_z[2] += 1.0f;
        }
        if (pyramid_rotate_x[3] < 0) {
            pyramid_rotate_x[3] += 1.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);
}
