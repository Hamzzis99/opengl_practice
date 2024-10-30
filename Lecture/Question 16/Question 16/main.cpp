#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "open_file.h"  
#include "open_obj.h"   
#include <random>
#include <vector>

// ���̴� ȣ��� ���� ����.
#define VERTEX_SHADER_CODE "vertex.glsl"
#define FRAGMENT_SHADER_CODE "fragment.glsl"

// ���� ��ġ �ʱ�ȭ
std::random_device rd;
std::mt19937 g(rd());


// �ݹ� �Լ� ����
void renderScene(void);                                        // ����� �������ϴ� �Լ�
void reshapeWindow(int w, int h);                             // â ũ�� ���� �� ȣ��Ǵ� �Լ�
void handleKeyboardInput(unsigned char key, int x, int y);    // Ű���� �Է� ó�� �Լ�
void handleSpecialKeys(int key, int x, int y);                // Ư�� Ű �Է� ó�� �Լ� (�ʿ� �� ����)
void update(int value);                                        // Ÿ�̸� �ݹ� �Լ��� �ִϸ��̼� ������Ʈ

// ���̴� ���� ���� ����
GLuint shaderProgram;     // ���̴� ���α׷� ID
GLuint vertexShader;      // ���ؽ� ���̴� ID
GLuint fragmentShader;    // �����׸�Ʈ ���̴� ID
GLuint VAO[4], VBO[4], EBO[4], axisVAO, axisVBO; // ���ؽ� �迭 ��ü ��

// ���̴� �� ���� �ʱ�ȭ �Լ� ����
void createVertexShader();
void createFragmentShader();
GLuint createShaderProgram();
void initializeBuffers();

// ���� ������
GLclampf backgroundColorR = 1.0f; // ��� ���� R
GLclampf backgroundColorG = 1.0f; // ��� ���� G
GLclampf backgroundColorB = 1.0f; // ��� ���� B
GLint windowWidth = 800, windowHeight = 600; // â ũ��

Model cubeModel;       // ť�� ��
Model coneModel;       // ���� ��
Model sphereModel;     // �� ��
Model cylinderModel;   // �Ǹ��� ��

int rotationFlag = -1; // ȸ�� ��� ���� �÷��� (-1�̸� ȸ�� ����)
int motionFlag = -1;   // �ִϸ��̼� ��� ���� �÷���

// ���� �׸��� ���� ���ؽ� ������ (��ġ�� ����)
// ������ ���������� ���� (0.0f, 0.0f, 0.0f)
const float axisVertices[] = {
    // ��ġ               // ���� (������)
    0.0f, 1.0f, 0.0f,     0.0f, 0.0f, 0.0f, // Y�� ���� ����
    0.0f, -1.0f, 0.0f,    0.0f, 0.0f, 0.0f, // Y�� ���� ����

    -1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f, // X�� ���� ����
    1.0f, 0.0f, 0.0f,     0.0f, 0.0f, 0.0f, // X�� ���� ����

    0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 0.0f, // Z�� ���� ����
    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, 0.0f, // Z�� ���� ����
};

// ȸ���� ��ġ ��� ���� ������
float axisRotationX = 0.0f; // x�� ȸ��
float axisRotationY = 0.0f; // y�� ȸ��

float model1PosX = -0.5f; // ù ��° �� ��ġ x
float model1PosY = 0.0f;  // ù ��° �� ��ġ y
float model1PosZ = 0.0f;  // ù ��° �� ��ġ z

float model2PosX = 0.5f;  // �� ��° �� ��ġ x
float model2PosY = 0.0f;  // �� ��° �� ��ġ y
float model2PosZ = 0.0f;  // �� ��° �� ��ġ z

float model1Scale = 1.0f; // ù ��° �� ������
float model2Scale = 1.0f; // �� ��° �� ������
float axisScale = 1.0f;    // �� ������

float viewRotationX = 0.0f; // �� ȸ�� x
float viewRotationY = 0.0f; // �� ȸ�� y
float viewRotationZ = 0.0f; // �� ȸ�� z

bool isModelChanged = false; // �� ���� ����

// �ִϸ��̼� �Լ� ����
void animateSpiralMove();
void animateCrossMove();
void animateZRotation();
void animateScaleRotation();

// �� ���� ��� �Լ� (����� �뵵)
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

// ���� �Լ�
int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ �� ������ ����
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Example");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW �ʱ�ȭ ����: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // ���̴� ���� �� ������
    createVertexShader();
    createFragmentShader();
    shaderProgram = createShaderProgram();

    // �ݹ� �Լ� ���
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshapeWindow);
    glutKeyboardFunc(handleKeyboardInput);
    // glutSpecialFunc(handleSpecialKeys); // Ư�� Ű ó���� �ʿ��ϸ� �ּ� ����

    // Ÿ�̸� �Լ� ���� (�ִϸ��̼� ������Ʈ)
    glutTimerFunc(60, update, 0);

    // �� �ε�
    loadOBJ("cube.obj", &cubeModel);
    loadOBJ("cone.obj", &coneModel);
    loadOBJ("sphere.obj", &sphereModel);
    loadOBJ("cylinder.obj", &cylinderModel);
    printModelVertices(cubeModel);

    // cube.obj�� �˷ϴ޷��� ���� �Ҵ�
    for (int i = 0; i < cubeModel.vertex_count; ++i) {
        // ����: ���� �ε����� ���� ���� �Ҵ� (����, �ʷ�, �Ķ� ��ȯ)
        switch (i % 3) {
        case 0:
            cubeModel.vertices[i].r = 1.0f; // ����
            cubeModel.vertices[i].g = 0.0f;
            cubeModel.vertices[i].b = 0.0f;
            break;
        case 1:
            cubeModel.vertices[i].r = 0.0f;
            cubeModel.vertices[i].g = 1.0f; // �ʷ�
            cubeModel.vertices[i].b = 0.0f;
            break;
        case 2:
            cubeModel.vertices[i].r = 0.0f;
            cubeModel.vertices[i].g = 0.0f;
            cubeModel.vertices[i].b = 1.0f; // �Ķ�
            break;
        }
    }

    // ���� �׽�Ʈ Ȱ��ȭ
    glEnable(GL_DEPTH_TEST);

    // ���� �ʱ�ȭ
    initializeBuffers();

    // ���� ���� ����
    glutMainLoop();

    return 0;
}

// ����� �������ϴ� �Լ�
void renderScene(void) {
    // ��� ���� �� ���� ���� �ʱ�ȭ
    glClearColor(backgroundColorR, backgroundColorG, backgroundColorB, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ���̴� ���α׷� ���
    glUseProgram(shaderProgram);

    // ��ȯ ����� ��ġ�� ������
    GLuint transformLoc = glGetUniformLocation(shaderProgram, "trans");

    // ���� �׸��� ���� VAO ���ε� �� ��ȯ ����
    glBindVertexArray(axisVAO);

    glm::mat4 axisTransform = glm::mat4(1.0f);
    axisTransform = glm::rotate(axisTransform, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
    axisTransform = glm::rotate(axisTransform, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(axisTransform));

    // �� �׸��� (���ؽ� ���� 6���� ����)
    glDrawArrays(GL_LINES, 0, 6);

    // ù ��° �𵨿� ���� ��ȯ ����
    glm::mat4 modelTransform1 = glm::mat4(1.0f);
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(viewRotationZ), glm::vec3(0.0, 0.0, 1.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(30.0f + viewRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(-45.0f + viewRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(axisScale, axisScale, axisScale));
    modelTransform1 = glm::translate(modelTransform1, glm::vec3(model1PosX, model1PosY, model1PosZ));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationY), glm::vec3(0.0, 1.0, 0.0));

    // ù ��° ���� ũ�⸦ �������� ���
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform1));

    // ù ��° �� �׸��� (ť�� �Ǵ� ��)
    if (!isModelChanged) {
        glBindVertexArray(VAO[0]); // ť�� VAO
        // VAO[0]�� ��ġ�� ���� �Ӽ��� ��� ����
        // glDrawElements�� �̿��Ͽ� ť�� �׸���
        glDrawElements(GL_TRIANGLES, cubeModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }
    else {
        glBindVertexArray(VAO[2]); // �� VAO
        // �� ���� ���� �Ӽ��� �����Ƿ� �⺻ �������� ��������
        glDrawElements(GL_TRIANGLES, sphereModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }

    // �� ��° �𵨿� ���� ��ȯ ����
    glm::mat4 modelTransform2 = glm::mat4(1.0f);
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(viewRotationZ), glm::vec3(0.0, 0.0, 1.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(30.0f + viewRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(-45.0f + viewRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform2 = glm::scale(modelTransform2, glm::vec3(axisScale, axisScale, axisScale));
    modelTransform2 = glm::translate(modelTransform2, glm::vec3(model2PosX, model2PosY, model2PosZ));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(axisRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(axisRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform2 = glm::scale(modelTransform2, glm::vec3(model2Scale, model2Scale, model2Scale));

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform2));

    // �� ��° �� �׸��� (���� �Ǵ� �Ǹ���)
    if (!isModelChanged) {
        glBindVertexArray(VAO[1]); // ���� VAO
        glDrawElements(GL_TRIANGLES, coneModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }
    else {
        glBindVertexArray(VAO[3]); // �Ǹ��� VAO
        glDrawElements(GL_TRIANGLES, cylinderModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }

    // ���� ��ȯ (ȭ�� ������Ʈ)
    glutSwapBuffers();
}

// â ũ�� ���� �� ȣ��Ǵ� �Լ�
void reshapeWindow(int w, int h) {
    // ����Ʈ ����
    glViewport(0, 0, w, h);
}

// Ű���� �Է� ó�� �Լ�
void handleKeyboardInput(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
        rotationFlag = 1; // x�� ���� ���� ȸ��
        break;
    case 'X':
        rotationFlag = 2; // x�� ���� ���� ȸ��
        break;
    case 'y':
        rotationFlag = 3; // y�� ���� ���� ȸ��
        break;
    case 'Y':
        rotationFlag = 4; // y�� ���� ���� ȸ��
        break;
    case 's':
        rotationFlag = -1; // ȸ�� ����
        axisRotationX = 0.0f;
        axisRotationY = 0.0f;
        viewRotationY = 0.0f;
        motionFlag = -1;
        break;
    case 'r':
    case '3':
        rotationFlag = 5; // ���� ����
        break;
    case 'R':
        rotationFlag = 6; // ������ ����
        break;
    case 'c':
        isModelChanged = !isModelChanged; // �� ����
        break;
    case 'e':
        model1Scale += 0.1f; // ù ��° �� ������ ����
        model2Scale += 0.1f; // �� ��° �� ������ ����
        break;
    case 'E':
        model1Scale -= 0.1f; // ù ��° �� ������ ����
        model2Scale -= 0.1f; // �� ��° �� ������ ����
        break;
    case 'w':
        axisScale += 0.1f; // �� ������ ����
        break;
    case 'W':
        axisScale -= 0.1f; // �� ������ ����
        break;
    case 'i':
        model1PosX += 0.1f; // ù ��° �� x �̵�
        break;
    case 'I':
        model1PosX -= 0.1f; // ù ��° �� x �̵�
        break;
    case 'o':
        model1PosY += 0.1f; // ù ��° �� y �̵�
        break;
    case 'O':
        model1PosY -= 0.1f; // ù ��° �� y �̵�
        break;
    case 'p':
        model1PosZ += 0.1f; // ù ��° �� z �̵�
        break;
    case 'P':
        model1PosZ -= 0.1f; // ù ��° �� z �̵�
        break;
    case 'j':
        model2PosX += 0.1f; // �� ��° �� x �̵�
        break;
    case 'J':
        model2PosX -= 0.1f; // �� ��° �� x �̵�
        break;
    case 'k':
        model2PosY += 0.1f; // �� ��° �� y �̵�
        break;
    case 'K':
        model2PosY -= 0.1f; // �� ��° �� y �̵�
        break;
    case 'l':
        model2PosZ += 0.1f; // �� ��° �� z �̵�
        break;
    case 'L':
        model2PosZ -= 0.1f; // �� ��° �� z �̵�
        break;
    case 'q':
        glutLeaveMainLoop(); // ���α׷� ����
        break;
    }

    // 1~5 ���� Ű�� ���� �ִϸ��̼� ����
    if ('1' <= key && key <= '5') {
        motionFlag = key - '0';
    }

    // ��� �ٽ� �׸���
    glutPostRedisplay();
}

// ���̴� ���� �� ������ �Լ�
void createVertexShader() {
    // ���ؽ� ���̴� ����
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource = open_file(VERTEX_SHADER_CODE);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // ������ ���� üũ
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: ���ؽ� ���̴� ������ ����\n" << errorLog << std::endl;
    }
    else {
        std::cout << "���ؽ� ���̴� ������ ����\n";
    }
}

void createFragmentShader() {
    // �����׸�Ʈ ���̴� ����
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource = open_file(FRAGMENT_SHADER_CODE);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // ������ ���� üũ
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: �����׸�Ʈ ���̴� ������ ����\n" << errorLog << std::endl;
    }
    else {
        std::cout << "�����׸�Ʈ ���̴� ������ ����\n";
    }
}

GLuint createShaderProgram() {
    // ���̴� ���α׷� ���� �� ���̴� ����
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // ���̴� ���α׷� ��ũ
    glLinkProgram(program);

    // ��ũ ���� üũ
    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(program, 512, NULL, errorLog);
        std::cerr << "ERROR: ���̴� ���α׷� ��ũ ����\n" << errorLog << std::endl;
    }
    else {
        std::cout << "���̴� ���α׷� ��ũ ����\n";
        // ���̴� ���� (�� �̻� �ʿ� ����)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // ���̴� ���α׷� ���
    glUseProgram(program);

    return program;
}

// ���� �ʱ�ȭ �Լ�
void initializeBuffers() {
    // ù ��° �� (ť��) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cubeModel.vertex_count * sizeof(Vertex), cubeModel.vertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� ���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // ���� �Ӽ� ���� (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeModel.face_count * sizeof(Face), cubeModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (����) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, coneModel.vertex_count * sizeof(Vertex), coneModel.vertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� ���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // ���� ���� ���� �Ӽ��� �����Ƿ� ���� �Ӽ��� ��Ȱ��ȭ�ϰų� �⺻ ������ ���
    // ���⼭�� ���� �Ӽ��� ��Ȱ��ȭ�մϴ�.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneModel.face_count * sizeof(Face), coneModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (��) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[2]);
    glBindVertexArray(VAO[2]);

    glGenBuffers(1, &VBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphereModel.vertex_count * sizeof(Vertex), sphereModel.vertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� ���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // �� ���� ���� �Ӽ��� �����Ƿ� ���� �Ӽ��� ��Ȱ��ȭ�ϰų� �⺻ ������ ���
    // ���⼭�� ���� �Ӽ��� ��Ȱ��ȭ�մϴ�.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereModel.face_count * sizeof(Face), sphereModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (�Ǹ���) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[3]);
    glBindVertexArray(VAO[3]);

    glGenBuffers(1, &VBO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, cylinderModel.vertex_count * sizeof(Vertex), cylinderModel.vertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� ���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // �Ǹ��� ���� ���� �Ӽ��� �����Ƿ� ���� �Ӽ��� ��Ȱ��ȭ�ϰų� �⺻ ������ ���
    // ���⼭�� ���� �Ӽ��� ��Ȱ��ȭ�մϴ�.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderModel.face_count * sizeof(Face), cylinderModel.faces, GL_STATIC_DRAW);

    // �� �׸��⸦ ���� ���� �ʱ�ȭ
    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);

    glGenBuffers(1, &axisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� ���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    // ���� �Ӽ� ���� (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// Ÿ�̸� �ݹ� �Լ� (�ִϸ��̼� ������Ʈ)
void update(int value) {
    // ȸ�� �÷��׿� ���� ȸ�� ������Ʈ
    switch (rotationFlag) {
    case 1:
        axisRotationX += 1.0f; // x�� ���� ���� ȸ��
        break;
    case 2:
        axisRotationX -= 1.0f; // x�� ���� ���� ȸ��
        break;
    case 3:
        axisRotationY += 1.0f; // y�� ���� ���� ȸ��
        break;
    case 4:
        axisRotationY -= 1.0f; // y�� ���� ���� ȸ��
        break;
    case 5:
        viewRotationY += 1.0f; // ���� ����
        break;
    case 6:
        viewRotationY -= 1.0f; // ������ ����
        break;
    default:
        break;
    }

    // ��� �÷��׿� ���� �ִϸ��̼� ����
    switch (motionFlag) {
    case 1:
        animateSpiralMove();
        break;
    case 2:
        animateCrossMove();
        break;
    case 4:
        animateZRotation();
        break;
    case 5:
        animateScaleRotation();
        break;
    default:
        break;
    }

    // ��� �ٽ� �׸���
    glutPostRedisplay();

    // ���� Ÿ�̸� ����
    glutTimerFunc(60, update, 0);
}

// �ִϸ��̼� �Լ���
void animateSpiralMove() {
    viewRotationY += 1.0f;          // y�� ����
    model1PosX += 0.001f;           // ù ��° �� x �̵�
    model2PosX -= 0.001f;           // �� ��° �� x �̵�
}

void animateCrossMove() {
    if (model1PosX < 0.5f)
        model1PosX += 0.01f;        // ù ��° �� x �̵�
    if (model2PosX > -0.5f)
        model2PosX -= 0.01f;        // �� ��° �� x �̵�
}

void animateZRotation() {
    viewRotationZ += 1.0f;          // z�� ȸ��
}

void animateScaleRotation() {
    viewRotationZ += 1.0f;          // z�� ȸ��
    model1Scale += 0.005f;          // ù ��° �� ������ ����
    model2Scale -= 0.005f;          // �� ��° �� ������ ����
}
