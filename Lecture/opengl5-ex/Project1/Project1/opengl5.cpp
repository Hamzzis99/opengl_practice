#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

// �Լ� ����
GLvoid RenderScene(GLvoid);
GLvoid HandleReshape(int width, int height);
GLvoid HandleKeyboard(unsigned char key, int x, int y);
GLvoid HandleMouse(int button, int state, int x, int y);
GLvoid HandleMotion(int x, int y);

// ��� ���� ���� ���
GLclampf backgroundRed = 1.0f;
GLclampf backgroundGreen = 1.0f;
GLclampf backgroundBlue = 1.0f;

// ���� �Ӽ��� ���� �簢�� ����ü
struct RandomRectangle {
    bool isCreated = false;
    GLclampf startX{};
    GLclampf startY{};
    GLclampf endX{};
    GLclampf endY{};
    GLclampf colorR{};
    GLclampf colorG{};
    GLclampf colorB{};
};

// ���� ���� ������ ����
std::random_device randomDevice;
std::mt19937 generator(randomDevice());

// �簢���� ������ �迭 (������ �ε����� ���찳��)
RandomRectangle rectangleList[41];

// ��ƿ��Ƽ �Լ� ����
void InitializeRectangles();
void MergeRectangles(int targetIndex);
bool IsOverlap(int rectIndex1, int rectIndex2);

// ���찳 ũ�� ����
GLclampf eraserWidth{};
GLclampf eraserHeight{};

// �巡�� ���¸� �����ϴ� �÷���
bool isDragging = false;

// ���α׷��� ������
int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rectangle Example");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW �ʱ�ȭ ����" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW ���������� �ʱ�ȭ��\n";
    }

    // �ݹ� �Լ� ���
    glutDisplayFunc(RenderScene);
    glutReshapeFunc(HandleReshape);
    glutKeyboardFunc(HandleKeyboard);
    glutMouseFunc(HandleMouse);
    glutMotionFunc(HandleMotion);

    // �簢�� �ʱ�ȭ
    InitializeRectangles();

    // GLUT �̺�Ʈ ó�� ������ ����
    glutMainLoop();

    return 0;
}

// ����� �������ϴ� �Լ�
GLvoid RenderScene(GLvoid) {
    // ��� ������ �����ϰ� ���۸� ����
    glClearColor(backgroundRed, backgroundGreen, backgroundBlue, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ������ ��� �簢���� �׸���
    for (const RandomRectangle& rect : rectangleList) {
        if (rect.isCreated) {
            glColor3f(rect.colorR, rect.colorG, rect.colorB);
            glRectf(rect.startX, rect.startY, rect.endX, rect.endY);
        }
    }

    // �������� �̹����� ǥ���ϱ� ���� ���� ��ü
    glutSwapBuffers();
}

// â ũ�� ������ ó���ϴ� �Լ�
GLvoid HandleReshape(int width, int height) {
    glViewport(0, 0, width, height);
}

// Ű���� �Է��� ó���ϴ� �Լ�
GLvoid HandleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'r':
        InitializeRectangles(); // 'r' Ű�� ������ �簢�� �ʱ�ȭ
        break;
    case 'q':
        glutLeaveMainLoop(); // 'q' Ű�� ������ ���α׷� ����
        break;
    }

    // ȭ�� �ٽ� �׸��� ��û
    glutPostRedisplay();
}

// �簢���� ������ ��ġ�� �������� �ʱ�ȭ�ϴ� �Լ�
void InitializeRectangles() {
    for (int i = 0; i < 20; i++) {
        GLclampf posX = (static_cast<float>(generator() % 2000) / 1000.0f) - 1.0f;
        GLclampf posY = (static_cast<float>(generator() % 2000) / 1000.0f) - 1.0f;
        rectangleList[i] = {
            true,
            posX,
            posY,
            posX + 0.025f,
            posY - 0.025f,
            static_cast<float>(generator() % 1000) / 1000.0f,
            static_cast<float>(generator() % 1000) / 1000.0f,
            static_cast<float>(generator() % 1000) / 1000.0f
        };
    }
    for (int j = 20; j < 40; ++j) {
        GLclampf posX = (static_cast<float>(generator() % 2000) / 1000.0f) - 1.0f;
        GLclampf posY = (static_cast<float>(generator() % 2000) / 1000.0f) - 1.0f;
        if (generator() % 2 == 0) {
            rectangleList[j] = {
                true,
                posX,
                posY,
                posX + 0.025f,
                posY - 0.025f,
                static_cast<float>(generator() % 1000) / 1000.0f,
                static_cast<float>(generator() % 1000) / 1000.0f,
                static_cast<float>(generator() % 1000) / 1000.0f
            };
        }
    }
    eraserWidth = 0.025f;
    eraserHeight = 0.025f;
    rectangleList[40].colorR = 0.0f; // ���찳 ���� �ʱ�ȭ
    rectangleList[40].colorG = 0.0f;
    rectangleList[40].colorB = 0.0f;
}

// ���콺 �̺�Ʈ�� ó���ϴ� �Լ�
GLvoid HandleMouse(int button, int state, int x, int y) {
    // ���콺 ��ǥ�� OpenGL ��ǥ�� ��ȯ
    GLclampf mouseX = (static_cast<float>(x - 800 / 2.0) * (1.0f / (800 / 2.0)));
    GLclampf mouseY = -(static_cast<float>(y - 600 / 2.0) * (1.0f / (600 / 2.0)));

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // ���� ��ư�� ������ ���찳 �簢�� ����
        rectangleList[40] = {
            true,
            mouseX - eraserWidth,
            mouseY + eraserHeight,
            mouseX + eraserWidth,
            mouseY - eraserHeight,
            0.0f, 0.0f, 0.0f
        };
        isDragging = true;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        // ���� ��ư�� ������ ���찳 �簢�� ����
        rectangleList[40].isCreated = false;
        isDragging = false;
    }
    glutPostRedisplay();
}

// ���콺 �̵��� ó���ϴ� �Լ�
GLvoid HandleMotion(int x, int y) {
    // ���콺 ��ǥ�� OpenGL ��ǥ�� ��ȯ
    GLclampf mouseX = (static_cast<float>(x - 800 / 2.0) * (1.0f / (800 / 2.0)));
    GLclampf mouseY = -(static_cast<float>(y - 600 / 2.0) * (1.0f / (600 / 2.0)));

    if (isDragging) {
        // ���찳 �簢���� ��ġ ������Ʈ
        rectangleList[40].startX = mouseX - eraserWidth;
        rectangleList[40].startY = mouseY + eraserHeight;
        rectangleList[40].endX = mouseX + eraserWidth;
        rectangleList[40].endY = mouseY - eraserHeight;

        // �ٸ� �簢���� ��ġ���� Ȯ���ϰ� ����
        for (int i = 0; i < 40; ++i) {
            if (rectangleList[40].isCreated && rectangleList[i].isCreated && IsOverlap(i, 40)) {
                MergeRectangles(i);
                break;
            }
        }
    }
    glutPostRedisplay();
}

// �簢���� �����ϴ� �Լ�
void MergeRectangles(int targetIndex) {
    // ��� �簢���� �����ϰ� ���찳 ũ�� ����
    rectangleList[targetIndex].isCreated = false;
    eraserWidth += 0.025f;
    eraserHeight += 0.025f;

    // ���찳�� ������ ���յ� �簢���� �������� ����
    rectangleList[40].colorR = rectangleList[targetIndex].colorR;
    rectangleList[40].colorG = rectangleList[targetIndex].colorG;
    rectangleList[40].colorB = rectangleList[targetIndex].colorB;

    std::cout << "���յ� �ε���: " << targetIndex << std::endl;
}

// �� �簢���� ��ġ���� Ȯ���ϴ� �Լ�
bool IsOverlap(int rectIndex1, int rectIndex2) {
    RandomRectangle rect1 = rectangleList[rectIndex1];
    RandomRectangle rect2 = rectangleList[rectIndex2];

    // �簢���� ��ġ�� ������ false ��ȯ
    if (rect1.endX < rect2.startX || rect1.startX > rect2.endX ||
        rect1.endY > rect2.startY || rect1.startY < rect2.endY) {
        return false;
    }

    return true;
}
