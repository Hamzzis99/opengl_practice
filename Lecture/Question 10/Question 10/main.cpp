#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <random>
#include <vector>
#include <cmath> // sqrt, cos, sin �Լ� ����� ���� ����
#include "file_utils.h" // ���̴� ���� �ε带 ���� ���

// M_PI ��� ����
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ���̴� ���� ���
#define VERTEX_SHADER_PATH "vertex.glsl"
#define FRAGMENT_SHADER_PATH "fragment.glsl"

// ���� ���� ������
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1.0f, 1.0f);
std::uniform_real_distribution<> color_dis(0.0f, 1.0f);
std::uniform_int_distribution<> int_dis(2, 3);
std::uniform_int_distribution<> bool_dis(0, 1);

// �ݹ� �Լ� ����
void drawScene(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void timer(int value);

// ���̴� ���α׷�
GLuint shaderProgram;

// VAO, VBO
GLuint VAO, VBO;

// �����̷� ����ü
struct SpiralPoint {
    float x, y, z;     // ��ġ
    float r, g, b;     // ����
};

// �����̷� ������ ������ ����
std::vector<std::vector<SpiralPoint>> spirals; // ���� ���� �����̷� ����

// �׸��� ��� (�� �Ǵ� ��)
bool isLineMode = false;

// �ִϸ��̼� ���� ����
int currentPoint = 0;
bool isAnimating = false;

// ������ ũ��
int width = 800;
int height = 600;

// ����
GLclampf bg_r = 1.0f, bg_g = 1.0f, bg_b = 1.0f;

int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("Spiral Animation");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    glewInit();

    // ���̴� ������ �� ��ũ
    GLuint vertexShader = compileShader(VERTEX_SHADER_PATH, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(FRAGMENT_SHADER_PATH, GL_FRAGMENT_SHADER);
    shaderProgram = linkProgram(vertexShader, fragmentShader);

    // VAO �� VBO ����
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // ���̴� ���α׷� ���
    glUseProgram(shaderProgram);

    // �ݹ� �Լ� ���
    glutDisplayFunc(drawScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    // ���� ���� ����
    glutMainLoop();

    return 0;
}

void createSpiral(float centerX, float centerY, int turns, bool clockwise) {
    const int pointsPerTurn = 100; // ������ �� ����
    float angleIncrement = (2.0f * M_PI) / pointsPerTurn;
    float radiusIncrement = 0.005f; // ������ ������

    std::vector<SpiralPoint> newSpiral;

    // ���� ����
    float r = color_dis(gen);
    float g = color_dis(gen);
    float b = color_dis(gen);

    // �ȿ��� ������ �����̷� ����
    float radius = 0.0f;
    for (int i = 0; i < turns * pointsPerTurn; ++i) {
        float angle = i * angleIncrement;
        if (!clockwise) {
            angle = -angle;
        }

        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);

        newSpiral.push_back({ x, y, 0.0f, r, g, b });

        radius += radiusIncrement;
    }

    // �ۿ��� ������ �����̷� ���� (�ݴ� ����)
    radius -= radiusIncrement;
    for (int i = 0; i < turns * pointsPerTurn; ++i) {
        float angle = (turns * pointsPerTurn - i) * angleIncrement;
        if (clockwise) {
            angle = -angle;
        }

        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);

        newSpiral.push_back({ x, y, 0.0f, r, g, b });

        radius -= radiusIncrement;
    }

    spirals.push_back(newSpiral);
}

void drawScene(void) {
    // ���� ����
    glClearColor(bg_r, bg_g, bg_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    int totalPoints = 0;
    for (const auto& spiral : spirals) {
        totalPoints += spiral.size();
    }

    if (totalPoints == 0) {
        glutSwapBuffers();
        return;
    }

    // VBO�� ��ü �����̷� �����͸� ���ε�
    std::vector<SpiralPoint> allPoints;
    for (const auto& spiral : spirals) {
        allPoints.insert(allPoints.end(), spiral.begin(), spiral.end());
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, allPoints.size() * sizeof(SpiralPoint), allPoints.data(), GL_DYNAMIC_DRAW);

    // ���� �Ӽ� ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpiralPoint), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SpiralPoint), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int offset = 0;
    for (const auto& spiral : spirals) {
        int drawCount = isAnimating ? std::min(currentPoint - offset, (int)spiral.size()) : spiral.size();
        if (drawCount > 0) {
            if (isLineMode) {
                glDrawArrays(GL_LINE_STRIP, offset, drawCount);
            }
            else {
                glDrawArrays(GL_POINTS, offset, drawCount);
            }
        }
        offset += spiral.size();
    }

    glutSwapBuffers();
}

void timer(int value) {
    if (isAnimating) {
        currentPoint += 5; // �ѹ��� 5���� ���� ����

        int totalPoints = 0;
        for (const auto& spiral : spirals) {
            totalPoints += spiral.size();
        }

        if (currentPoint >= totalPoints) {
            isAnimating = false;
            currentPoint = 0;
        }
        else {
            glutTimerFunc(16, timer, 0); // �� 60FPS
        }

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
        isLineMode = false;
        glutPostRedisplay();
        break;
    case 'l':
        isLineMode = true;
        glutPostRedisplay();
        break;
    case '1': case '2': case '3': case '4': case '5': {
        int numSpirals = key - '0';
        spirals.clear();
        for (int i = 0; i < numSpirals; ++i) {
            float centerX = dis(gen);
            float centerY = dis(gen);
            int turns = int_dis(gen);
            bool clockwise = bool_dis(gen);
            createSpiral(centerX, centerY, turns, clockwise);
        }
        // ���� ����
        bg_r = color_dis(gen);
        bg_g = color_dis(gen);
        bg_b = color_dis(gen);

        isAnimating = true;
        currentPoint = 0;
        glutTimerFunc(0, timer, 0);
        break;
    }
    case 'q':
        glutLeaveMainLoop();
        break;
    default:
        break;
    }
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        // ���콺 ��ǥ�� OpenGL ��ǥ�� ��ȯ
        float mouseX = (float)(x) / (width / 2.0f) - 1.0f;
        float mouseY = 1.0f - (float)(y) / (height / 2.0f);

        int turns = int_dis(gen);
        bool clockwise = bool_dis(gen);

        spirals.clear();
        createSpiral(mouseX, mouseY, turns, clockwise);

        // ���� ����
        bg_r = color_dis(gen);
        bg_g = color_dis(gen);
        bg_b = color_dis(gen);

        isAnimating = true;
        currentPoint = 0;
        glutTimerFunc(0, timer, 0);
    }
}

void reshape(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, width, height);
}
