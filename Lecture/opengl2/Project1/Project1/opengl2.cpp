#include <iostream>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

// ���� ���� �� �ʱ� ����
GLfloat base_r = 0.5f, base_g = 0.5f, base_b = 0.5f; // ���� �ʱ�ȭ

// �簢�� ����ü ���� (�̸� �浹 ������ ���� MyRectangle ���)
struct MyRectangle {
    GLfloat x1, y1; // ���� �ϴ� ��ǥ��
    GLfloat x2, y2; // ���� ��� ��ǥ��
    GLfloat r, g, b; // ����
};

// �簢�� 4���� ������ �迭
MyRectangle rects[4];

// â ũ�� �ʱⰪ
int window_width = 800;
int window_height = 600;

// ���� ���� ���� �Լ�
void generateRandomColor(GLfloat& r, GLfloat& g, GLfloat& b) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    r = static_cast<GLfloat>(dis(gen));
    g = static_cast<GLfloat>(dis(gen));
    b = static_cast<GLfloat>(dis(gen));
}

// �簢�� �ʱ�ȭ �Լ�
void initRectangles() {
    // �簢�� 1 (�»��)
    rects[0].x1 = -1.0f;
    rects[0].y1 = 0.0f;
    rects[0].x2 = 0.0f;
    rects[0].y2 = 1.0f;
    rects[0].r = 1.0f; // ������
    rects[0].g = 0.0f;
    rects[0].b = 0.0f;

    // �簢�� 2 (����)
    rects[1].x1 = 0.0f;
    rects[1].y1 = 0.0f;
    rects[1].x2 = 1.0f;
    rects[1].y2 = 1.0f;
    rects[1].r = 0.0f; // �ʷϻ�
    rects[1].g = 1.0f;
    rects[1].b = 0.0f;

    // �簢�� 3 (���ϴ�)
    rects[2].x1 = -1.0f;
    rects[2].y1 = -1.0f;
    rects[2].x2 = 0.0f;
    rects[2].y2 = 0.0f;
    rects[2].r = 0.0f; // �Ķ���
    rects[2].g = 0.0f;
    rects[2].b = 1.0f;

    // �簢�� 4 (���ϴ�)
    rects[3].x1 = 0.0f;
    rects[3].y1 = -1.0f;
    rects[3].x2 = 1.0f;
    rects[3].y2 = 0.0f;
    rects[3].r = 1.0f; // �����
    rects[3].g = 1.0f;
    rects[3].b = 0.0f;
}

// ���÷��� �ݹ� �Լ�
void display() {
    // ���� ���� �� ȭ�� �ʱ�ȭ
    glClearColor(base_r, base_g, base_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // �� �簢�� �׸���
    for (int i = 0; i < 4; ++i) {
        glColor3f(rects[i].r, rects[i].g, rects[i].b);
        glRectf(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);
    }

    // ȭ�� ������Ʈ
    glutSwapBuffers();
}

// ���콺 ��ǥ�� OpenGL ��ǥ�� ��ȯ�ϴ� �Լ�
void convertToOpenGLCoords(int x, int y, GLfloat& ogl_x, GLfloat& ogl_y) {
    ogl_x = ((GLfloat)x / (window_width / 2)) - 1.0f;
    ogl_y = 1.0f - ((GLfloat)y / (window_height / 2));
}

// Ŭ���� ������ �簢�� �ε����� ��ȯ�ϴ� �Լ�
int getClickedRectangleIndex(GLfloat x, GLfloat y) {
    for (int i = 0; i < 4; ++i) {
        if (x >= rects[i].x1 && x <= rects[i].x2 && y >= rects[i].y1 && y <= rects[i].y2) {
            return i;
        }
    }
    return -1;
}

// ��и� Ȯ�� �Լ�
int getQuadrant(GLfloat x, GLfloat y) {
    if (x < 0 && y > 0) return 0;  // �»��
    if (x > 0 && y > 0) return 1;  // ����
    if (x < 0 && y < 0) return 2;  // ���ϴ�
    if (x > 0 && y < 0) return 3;  // ���ϴ�
    return -1;
}

// ���콺 �ݹ� �Լ�
void mouseCallback(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return; // ��ư�� ������ ���� ó��

    GLfloat ogl_x, ogl_y;
    convertToOpenGLCoords(x, y, ogl_x, ogl_y);

    int rectIndex = getClickedRectangleIndex(ogl_x, ogl_y);
    bool insideRect = (rectIndex != -1);

    if (button == GLUT_LEFT_BUTTON) {
        if (insideRect) {
            // �簢�� ���� Ŭ��: �簢�� ���� ���� ����
            generateRandomColor(rects[rectIndex].r, rects[rectIndex].g, rects[rectIndex].b);
        }
        else {
            // �簢�� �ܺ� Ŭ��: ���� ���� ����
            generateRandomColor(base_r, base_g, base_b);
        }
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        if (insideRect) {
            // �簢�� ���� Ŭ��: �簢�� ũ�� ��� (�ּ� ũ�� ����)
            GLfloat min_size = 0.2f;
            GLfloat width = rects[rectIndex].x2 - rects[rectIndex].x1;
            GLfloat height = rects[rectIndex].y2 - rects[rectIndex].y1;

            if (width > min_size && height > min_size) {
                rects[rectIndex].x1 += 0.05f;
                rects[rectIndex].y1 += 0.05f;
                rects[rectIndex].x2 -= 0.05f;
                rects[rectIndex].y2 -= 0.05f;
            }
        }
        else {
            // �簢�� �ܺ� Ŭ��: �ش� ��и��� �簢�� ũ�� Ȯ��
            int quadrant = getQuadrant(ogl_x, ogl_y);
            if (quadrant != -1) {
                rects[quadrant].x1 -= 0.05f;
                rects[quadrant].y1 -= 0.05f;
                rects[quadrant].x2 += 0.05f;
                rects[quadrant].y2 += 0.05f;
            }
        }
    }

    // ȭ�� ���� ��û
    glutPostRedisplay();
}

// Ű���� �ݹ� �Լ�
void keyboardCallback(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') {
        glutLeaveMainLoop();
    }
}

// ������ �������� �ݹ� �Լ�
void reshapeCallback(int w, int h) {
    window_width = w;
    window_height = h;
    glViewport(0, 0, w, h);
}

// �ʱ�ȭ �Լ�
void initialize() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // GLEW �ʱ�ȭ ����
        std::cerr << "GLEW �ʱ�ȭ ����: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }

    // �簢�� �ʱ�ȭ
    initRectangles();
}

// ���� �Լ�
int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���� ���۸��� RGBA �÷� ��� ���
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("�ǽ� 2");

    // GLEW �� �簢�� �ʱ�ȭ
    initialize();

    // �ݹ� �Լ� ���
    glutDisplayFunc(display);
    glutReshapeFunc(reshapeCallback);
    glutMouseFunc(mouseCallback);
    glutKeyboardFunc(keyboardCallback);

    // ���� ���� ����
    glutMainLoop();

    return 0;
}
