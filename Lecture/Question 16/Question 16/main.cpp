#include<iostream>
#include<gl/glew.h>
#include<gl/freeglut.h>
#include<gl/freeglut_ext.h>
#include<glm/ext.hpp>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "open_file.h"  
#include "open_obj.h"   
#include<random>
#include<vector>

// ��ó���� ���ǿ� ���� ���� ����
#define vertex_shader_code "vertex.glsl"
#define fragment_shader_code "fragment.glsl"

std::random_device rd;
std::mt19937 g(rd());

// �ݹ� �Լ� ����
void renderScene(void);                   // ����� �������ϴ� �Լ�
void reshapeWindow(int w, int h);         // â ũ�� ���� �� ȣ��Ǵ� �Լ�
void handleKeyboardInput(unsigned char key, int x, int y); // Ű���� �Է� ó�� �Լ�
void handleSpecialKeys(int key, int x, int y);             // Ư�� Ű �Է� ó�� �Լ� (�ʿ��)
void update(int value);                   // Ÿ�̸� �ݹ� �Լ��� �ִϸ��̼� ������Ʈ

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
GLclampf backgroundColorR = 1.0f;
GLclampf backgroundColorG = 1.0f;
GLclampf backgroundColorB = 1.0f;
GLint windowWidth = 800, windowHeight = 600;

Model cubeModel;
Model coneModel;
Model sphereModel;
Model cylinderModel;

int rotationFlag = -1; // ȸ�� ��� ���� �÷��� (-1�̸� ȸ�� ����)
int motionFlag = -1;   // �ִϸ��̼� ��� ���� �÷���

// ���� �׸��� ���� ���ؽ� ������ (��ġ�� ����)
const float axisVertices[] = {
    // ��ġ               // ���� (���)
    0.0f, 1.0f, 0.0f,     1.0f, 1.0f, 1.0f, // Y�� ���� ����
    0.0f, -1.0f, 0.0f,    1.0f, 1.0f, 1.0f, // Y�� ���� ����

    -1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, // X�� ���� ����
    1.0f, 0.0f, 0.0f,     1.0f, 1.0f, 1.0f, // X�� ���� ����

    0.0f, 0.0f, 1.0f,     1.0f, 1.0f, 1.0f, // Z�� ���� ����
    0.0f, 0.0f, -1.0f,    1.0f, 1.0f, 1.0f, // Z�� ���� ����
};

// ȸ���� ��ġ ��� ���� ������
float axisRotationX = 0.0f;
float axisRotationY = 0.0f;

float model1PosX = -0.5f;
float model1PosY = 0.0f;
float model1PosZ = 0.0f;

float model2PosX = 0.5f;
float model2PosY = 0.0f;
float model2PosZ = 0.0f;

float model1Scale = 1.0f;
float model2Scale = 1.0f;
float axisScale = 1.0f;

float viewRotationX = 0.0f;
float viewRotationY = 0.0f;
float viewRotationZ = 0.0f;

bool isModelChanged = false; // �� ���� ����

// �ִϸ��̼� �Լ� ����
void animateSpiralMove();
void animateCrossMove();
void animateZRotation();
void animateScaleRotation();

// ���� �Լ�
int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ �� ������ ����
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Example");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    glewInit();

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
    open_obj("cube.obj", &cubeModel);
    open_obj("cone.obj", &coneModel);
    open_obj("sphere.obj", &sphereModel);
    open_obj("cylinder.obj", &cylinderModel);

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
    // ���� �� ���� ���� �ʱ�ȭ
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

    // �� �׸���
    glDrawArrays(GL_LINES, 0, 12);

    // ù ��° �𵨿� ���� ��ȯ ����
    glm::mat4 modelTransform1 = glm::mat4(1.0f);
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(viewRotationZ), glm::vec3(0.0, 0.0, 1.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(30.0f + viewRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(-45.0f + viewRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(axisScale, axisScale, axisScale));
    modelTransform1 = glm::translate(modelTransform1, glm::vec3(model1PosX, model1PosY, model1PosZ));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(model1Scale, model1Scale, model1Scale));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform1));

    // ù ��° �� �׸��� (ť�� �Ǵ� ��)
    if (!isModelChanged) {
        glBindVertexArray(VAO[0]); // ť�� VAO
        for (int i = 0; i < (cubeModel.face_count * 3) - 1; ++i) {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }
    else {
        glBindVertexArray(VAO[2]); // �� VAO
        for (int i = 0; i < (sphereModel.face_count * 3) - 1; ++i) {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
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
        for (int i = 0; i < (coneModel.face_count * 3); ++i) {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
    }
    else {
        glBindVertexArray(VAO[3]); // �Ǹ��� VAO
        for (int i = 0; i < (cylinderModel.face_count * 3) - 1; ++i) {
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int)));
        }
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
        axisRotationX = 0;
        axisRotationY = 0;
        viewRotationY = 0;
        motionFlag = -1;
        break;
    case 'r':
        rotationFlag = 5; // ���� ����
        break;
    case 'R':
        rotationFlag = 6; // ������ ����
        break;
    case '3':
        rotationFlag = 5; // ���� ����
        break;
    case 'c':
        isModelChanged = !isModelChanged; // �� ����
        break;
    case 'e':
        model1Scale += 0.1f;
        model2Scale += 0.1f;
        break;
    case 'E':
        model1Scale -= 0.1f;
        model2Scale -= 0.1f;
        break;
    case 'w':
        axisScale += 0.1f;
        break;
    case 'W':
        axisScale -= 0.1f;
        break;
    case 'i':
        model1PosX += 0.1f;
        break;
    case 'I':
        model1PosX -= 0.1f;
        break;
    case 'o':
        model1PosY += 0.1f;
        break;
    case 'O':
        model1PosY -= 0.1f;
        break;
    case 'p':
        model1PosZ += 0.1f;
        break;
    case 'P':
        model1PosZ -= 0.1f;
        break;
    case 'j':
        model2PosX += 0.1f;
        break;
    case 'J':
        model2PosX -= 0.1f;
        break;
    case 'k':
        model2PosY += 0.1f;
        break;
    case 'K':
        model2PosY -= 0.1f;
        break;
    case 'l':
        model2PosZ += 0.1f;
        break;
    case 'L':
        model2PosZ -= 0.1f;
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
    // ���ؽ� ���̴� ���� �� ������
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource = open_file(vertex_shader_code);
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
    // �����׸�Ʈ ���̴� ���� �� ������
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource = open_file(fragment_shader_code);
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeModel.face_count * sizeof(Face), cubeModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (����) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, coneModel.vertex_count * sizeof(Vertex), coneModel.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneModel.face_count * sizeof(Face), coneModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (��) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[2]);
    glBindVertexArray(VAO[2]);

    glGenBuffers(1, &VBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphereModel.vertex_count * sizeof(Vertex), sphereModel.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereModel.face_count * sizeof(Face), sphereModel.faces, GL_STATIC_DRAW);

    // �� ��° �� (�Ǹ���) ���� �ʱ�ȭ
    glGenVertexArrays(1, &VAO[3]);
    glBindVertexArray(VAO[3]);

    glGenBuffers(1, &VBO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, cylinderModel.vertex_count * sizeof(Vertex), cylinderModel.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderModel.face_count * sizeof(Face), cylinderModel.faces, GL_STATIC_DRAW);

    // �� �׸��⸦ ���� ���� �ʱ�ȭ
    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);

    glGenBuffers(1, &axisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// Ÿ�̸� �ݹ� �Լ� (�ִϸ��̼� ������Ʈ)
void update(int value) {
    // ȸ�� �÷��׿� ���� ȸ�� ������Ʈ
    switch (rotationFlag) {
    case 1:
        axisRotationX += 1.0f;
        break;
    case 2:
        axisRotationX -= 1.0f;
        break;
    case 3:
        axisRotationY += 1.0f;
        break;
    case 4:
        axisRotationY -= 1.0f;
        break;
    case 5:
        viewRotationY += 1.0f;
        break;
    case 6:
        viewRotationY -= 1.0f;
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
    }

    // ��� �ٽ� �׸���
    glutPostRedisplay();

    // ���� Ÿ�̸� ����
    glutTimerFunc(60, update, 0);
}

// �ִϸ��̼� �Լ���
void animateSpiralMove() {
    viewRotationY += 1.0f;
    model1PosX += 0.001f;
    model2PosX -= 0.001f;
}

void animateCrossMove() {
    if (model1PosX < 0.5f)
        model1PosX += 0.01f;
    if (model2PosX > -0.5f)
        model2PosX -= 0.01f;
}

void animateZRotation() {
    viewRotationZ += 1.0f;
}

void animateScaleRotation() {
    viewRotationZ += 1.0f;
    model1Scale += 0.005f;
    model2Scale -= 0.005f;
}
