#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "file_utils.h"
#include <random>
#include <vector>
#include <cmath>

// ���� ���� ������
std::random_device rd;
std::mt19937 g(rd());

// �ݹ� �Լ� ����
void drawScene(void);
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);

// ���̴� ���� ����
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;

// VAO, VBO, EBO
GLuint VAO[2], VBO[2], EBO;

// ��������
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

// ��и麰 �ﰢ�� ������ ����ü
struct Triangle {
    float vertices[18]; // �� �ﰢ�� �� 3���� ����, �� ������ 6���� �Ӽ� (x, y, z, r, g, b)
};

std::vector<Triangle> triangles[4]; // ��и麰 �ﰢ�� ����Ʈ

// ���� �׸��� ��� (�� �Ǵ� ��)
bool is_face = true;

// �Լ� ����
int click_area(GLclampf x, GLclampf y);
void create_tri(int clicked_area, GLclampf x, GLclampf y);
void init_buffer();
void init_tri();

// �� ������ (x��� y���� �׸��� ���� ��)
float line[] = {
    -1.0f, 0.0f, 0.0f, // x�� ����
    1.0f, 0.0f, 0.0f,  // x�� ������
    0.0f, -1.0f, 0.0f, // y�� �Ʒ���
    0.0f, 1.0f, 0.0f   // y�� ����
};

int main(int argc, char** argv) {
    // GLUT �ʱ�ȭ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100); // â ��ġ
    glutInitWindowSize(width, height);
    glutCreateWindow("�ǽ� 8: ��и麰 �ﰢ�� �׸���");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW �ʱ�ȭ ����: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }

    // ���̴� ������ �� ���α׷� ��ũ
    vertexShader = compileShader("vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = compileShader("fragment.glsl", GL_FRAGMENT_SHADER);
    shader_program = linkProgram(vertexShader, fragmentShader);

    // ���� �ʱ�ȭ �� �ﰢ�� �ʱ�ȭ
    init_buffer();
    init_tri();

    // �ݹ� �Լ� ���
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);

    // ���� ���� ����
    glutMainLoop();

    return 0;
}

void drawScene(void) {
    glClearColor(base_r, base_g, base_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    // ��и� ���� �� �׸���
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINES, 0, 4);

    // �ﰢ�� �׸���
    glBindVertexArray(VAO[0]);
    for (int i = 0; i < 4; ++i) {
        int tri_count = triangles[i].size();
        if (tri_count > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle) * tri_count, triangles[i].data(), GL_DYNAMIC_DRAW);
            if (is_face) {
                glDrawArrays(GL_TRIANGLES, 0, 3 * tri_count);
            }
            else {
                glDrawArrays(GL_LINE_LOOP, 0, 3 * tri_count);
            }
        }
    }

    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
        is_face = true;
        std::cout << "�׸��� ���: ��(Face)" << std::endl;
        break;

    case 'b':
        is_face = false;
        std::cout << "�׸��� ���: ��(Line)" << std::endl;
        break;

    case 'q':
        glutLeaveMainLoop();
        break;
    }

    glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN)
        return;

    // ���콺 ��ǥ�� OpenGL ��ǥ�� ��ȯ
    GLclampf mouse_x = (static_cast<float>(x) / width) * 2.0f - 1.0f;
    GLclampf mouse_y = 1.0f - (static_cast<float>(y) / height) * 2.0f;

    int clicked = click_area(mouse_x, mouse_y);
    if (clicked == -1) {
        std::cout << "Ŭ�� ��ġ�� ��и� ���Դϴ�." << std::endl;
        return;
    }

    if (button == GLUT_LEFT_BUTTON) {
        // �ش� ��и��� �ﰢ�� ����
        triangles[clicked].clear();
        // ���ο� �ﰢ�� ����
        create_tri(clicked, mouse_x, mouse_y);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        // �ش� ��и鿡 �ﰢ�� �߰� (�ִ� 3��)
        if (triangles[clicked].size() < 3) {
            create_tri(clicked, mouse_x, mouse_y);
        }
        else {
            std::cout << "��и� " << clicked + 1 << "�� �� �̻� �ﰢ���� �߰��� �� �����ϴ�." << std::endl;
        }
    }

    glutPostRedisplay();
}

int click_area(GLclampf x, GLclampf y) {
    if (x < 0.0f && y > 0.0f) {
        return 0; // 1��и�
    }
    else if (x > 0.0f && y > 0.0f) {
        return 1; // 2��и�
    }
    else if (x < 0.0f && y < 0.0f) {
        return 2; // 3��и�
    }
    else if (x > 0.0f && y < 0.0f) {
        return 3; // 4��и�
    }
    return -1; // ��и� ��
}

void create_tri(int clicked_area, GLclampf x, GLclampf y) {
    // �ﰢ���� ����� ũ�� ���� ����
    GLclampf tri_r = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_g = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_b = static_cast<GLclampf>(g() % 100) / 100.0f;
    float size = 0.1f + static_cast<float>(g() % 50) / 500.0f;

    //// �̵ �ﰢ���� ���� ���

    float height = size * std::sqrt(3.0f);  // ���̸� �� ��� ����
    float triX1 = x;
    float triY1 = y + height / 2.0f;
    float triX2 = x - size / 2.0f;
    float triY2 = y - height / 2.0f;
    float triX3 = x + size / 2.0f;
    float triY3 = y - height / 2.0f;

    // �ﰢ�� ������ ����
    Triangle tri;
    tri.vertices[0] = triX1; tri.vertices[1] = triY1; tri.vertices[2] = 0.0f;
    tri.vertices[3] = tri_r; tri.vertices[4] = tri_g; tri.vertices[5] = tri_b;

    tri.vertices[6] = triX2; tri.vertices[7] = triY2; tri.vertices[8] = 0.0f;
    tri.vertices[9] = tri_r; tri.vertices[10] = tri_g; tri.vertices[11] = tri_b;

    tri.vertices[12] = triX3; tri.vertices[13] = triY3; tri.vertices[14] = 0.0f;
    tri.vertices[15] = tri_r; tri.vertices[16] = tri_g; tri.vertices[17] = tri_b;

    triangles[clicked_area].push_back(tri);

    std::cout << "��и� " << clicked_area + 1 << "�� �ﰢ���� �����߽��ϴ�. ���� �ﰢ�� ��: " << triangles[clicked_area].size() << std::endl;
}

void init_buffer() {
    // �ﰢ�� VAO ����
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    // ���� �Ӽ� ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ���� �Ӽ� ����
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // �� VAO ����
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void init_tri() {
    // �� ��и��� �߾ӿ� �ﰢ���� �ϳ��� ����
    create_tri(0, -0.5f, 0.5f);  // 1��и�
    create_tri(1, 0.5f, 0.5f);   // 2��и�
    create_tri(2, -0.5f, -0.5f); // 3��и�
    create_tri(3, 0.5f, -0.5f);  // 4��и�
}
