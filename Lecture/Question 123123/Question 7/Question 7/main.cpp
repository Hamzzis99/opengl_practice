#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "file_utils.h" // ������ ��� ���� ����
#include <random>
#include <vector>
#include <cmath> // �ﰢ�� ���̸� ����ϱ� ���� �ʿ�

// ���� ���� ������
std::random_device rd;
std::mt19937 g(rd());

//------------------------------------------------------
// �ݹ� �Լ� ����
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
//------------------------------------------------------
// ���̴� �� ����
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;
GLuint VAO[2], VBO, EBO, line_buffer;

// ��������
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

// ��и麰 �ﰢ�� �ε���
unsigned int index_buffer[] = {
    0, 1, 2,    // �ﰢ�� 1
    3, 4, 5,    // �ﰢ�� 2
    6, 7, 8,    // �ﰢ�� 3
    9, 10, 11   // �ﰢ�� 4
};

// ��и��� ������ �׸��� ������ �׸��� �����ϴ� ����
bool is_face = true;

// ��и��� �����ϴ� ��
float line[] = {
    0.0f, 1.0f, 0.0f, // y�� ����
    0.0f, -1.0f, 0.0f, // y�� �Ʒ���

    -1.0f, 0.0f, 0.0f, // x�� ����
    1.0f, 0.0f, 0.0f   // x�� ������
};

//------------------------------------------------------
// �ʿ��� �Լ� ����
int click_area(GLclampf x, GLclampf y);
void create_tri(int clicked_area, GLclampf x, GLclampf y);
void init_tri();
void init_buffer();

//------------------------------------------------------
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

    // ����� ��� Ȱ��ȭ (���� ����)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
            if (type == GL_DEBUG_TYPE_ERROR)
                std::cerr << "GL ERROR: " << message << std::endl;
        }, nullptr);

    // ���̴� ������ �� ���α׷� ��ũ
    vertexShader = compileShader("vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = compileShader("fragment.glsl", GL_FRAGMENT_SHADER);
    shader_program = linkProgram(vertexShader, fragmentShader);
    if (shader_program == 0) {
        std::cerr << "���̴� ���α׷� ��ũ ����" << std::endl;
        return -1;
    }

    // �ݹ� �Լ� ���
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);

    // ���� �ʱ�ȭ �� �ﰢ�� �ʱ�ȭ
    init_buffer();
    init_tri();

    // GLUT ���� ���� ����
    glutMainLoop();

    return 0;
}

GLvoid drawScene(GLvoid) {
    // ��� ���� ���� �� ȭ�� Ŭ����
    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ���̴� ���α׷� ���
    glUseProgram(shader_program);

    // ��и� ���� �� �׸���
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINES, 0, 4); // x��� y���� �׸��� ���� 4���� ����

    // �ﰢ�� �׸���
    glBindVertexArray(VAO[0]);
    if (is_face) {
        glDrawArrays(GL_TRIANGLES, 0, 12); // 4��и� ���� 3���� ����, �� 12��
    }
    else {
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // �� ���� 24���� �ε���
    }

    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

GLvoid Keyboard(unsigned char key, int x, int y) {
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

GLvoid Mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    // ���콺 ��ǥ�� OpenGL NDC ��ǥ�� ��ȯ
    GLclampf mouse_x = (static_cast<float>(x) / width) * 2.0f - 1.0f;
    GLclampf mouse_y = 1.0f - (static_cast<float>(y) / height) * 2.0f;

    int clicked = click_area(mouse_x, mouse_y);
    if (clicked == -1) {
        std::cout << "Ŭ�� ��ġ�� ��и� ���Դϴ�." << std::endl;
        return;
    }

    if (button == GLUT_LEFT_BUTTON) {
        // ���� ���콺 Ŭ��: �ش� ��и��� ��� �ﰢ�� ���� �� ���ο� �ﰢ�� ����
        std::cout << "���� Ŭ��: ��и� " << clicked + 1 << "�� ��� �ﰢ���� �����ϰ� ���ο� �ﰢ���� �����մϴ�." << std::endl;
        // VBO �ʱ�ȭ (��и麰 �ﰢ�� ����)
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 18 * clicked, sizeof(float) * 18, nullptr);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        // ������ ���콺 Ŭ��: �ش� ��и鿡 �ִ� 3���� �ﰢ�� �߰�
        std::cout << "������ Ŭ��: ��и� " << clicked + 1 << "�� �ﰢ���� �߰��մϴ�." << std::endl;
        // �ﰢ�� ���� �Լ� ȣ��
        create_tri(clicked, mouse_x, mouse_y);
    }

    glutPostRedisplay();
}

int click_area(GLclampf x, GLclampf y) {
    if (-1.0f < x && x < 0.0f && 0.0f < y && y < 1.0f) {
        return 0; // 1��и�
    }
    else if (0.0f < x && x < 1.0f && 0.0f < y && y < 1.0f) {
        return 1; // 2��и�
    }
    else if (-1.0f < x && x < 0.0f && -1.0f < y && y < 0.0f) {
        return 2; // 3��и�
    }
    else if (0.0f < x && x < 1.0f && -1.0f < y && y < 0.0f) {
        return 3; // 4��и�
    }

    return -1; // ��и� ��
}

void create_tri(int clicked_area, GLclampf x, GLclampf y) {
    // ��и麰 �ﰢ�� ī��Ʈ ���� (�ִ� 3��)
    static int tri_counts[4] = { 0, 0, 0, 0 };
    const int MAX_TRIANGLES = 3;

    if (tri_counts[clicked_area] >= MAX_TRIANGLES) {
        std::cout << "��и� " << clicked_area + 1 << "�� �� �̻� �ﰢ���� �߰��� �� �����ϴ�." << std::endl;
        return;
    }

    // �ﰢ���� ����� ũ�� ���� ����
    GLclampf tri_r = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_g = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_b = static_cast<GLclampf>(g() % 100) / 100.0f;
    float size = 0.1f + static_cast<float>(g() % 50) / 500.0f; // 0.1f ~ 0.19f

    // �̵ �ﰢ���� ���� ���
    float height = size * (std::sqrt(3.0f) / 2.0f);
    float triX1 = x;
    float triY1 = y + height / 2.0f;
    float triX2 = x - size / 2.0f;
    float triY2 = y - height / 2.0f;
    float triX3 = x + size / 2.0f;
    float triY3 = y - height / 2.0f;

    // �ﰢ�� ������ ����
    float tri_vertices[18] = {
        triX1, triY1, 0.0f, tri_r, tri_g, tri_b,
        triX2, triY2, 0.0f, tri_r, tri_g, tri_b,
        triX3, triY3, 0.0f, tri_r, tri_g, tri_b
    };

    // VBO ������Ʈ
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 18 * tri_counts[clicked_area],
        sizeof(tri_vertices), tri_vertices);

    tri_counts[clicked_area]++;
    std::cout << "�ﰢ���� �׷Ƚ��ϴ�. ��и�: " << clicked_area + 1
        << ", ���� �ﰢ�� ��: " << tri_counts[clicked_area] << std::endl;
}

void init_tri() {
    // �ʱ� ��и鿡 �ﰢ�� 1���� �׸���
    for (int i = 0; i < 4; ++i) {
        // ��и� �߾� ��ġ ����
        GLclampf x, y;
        switch (i) {
        case 0: // 1��и�
            x = -0.5f;
            y = 0.5f;
            break;
        case 1: // 2��и�
            x = 0.5f;
            y = 0.5f;
            break;
        case 2: // 3��и�
            x = -0.5f;
            y = -0.5f;
            break;
        case 3: // 4��и�
            x = 0.5f;
            y = -0.5f;
            break;
        }

        create_tri(i, x, y);
    }
}

void init_buffer() {
    // �ﰢ�� VAO ����
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // �� ��и麰�� �ִ� 3���� �ﰢ��, �� �ﰢ�� �� 3���� ����, �� ���� �� 6���� �Ӽ� (x, y, z, r, g, b)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * 3 * 4, nullptr, GL_DYNAMIC_DRAW);

    // ���� �Ӽ� ����
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ���� �Ӽ� ����
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // EBO ���� (��и麰 �ﰢ�� �ε���)
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // ��и麰 �ﰢ�� �ε��� (�� �ﰢ�� �� 3���� �ε���)
    unsigned int indices[12] = {
        0, 1, 2,    // �ﰢ�� 1
        3, 4, 5,    // �ﰢ�� 2
        6, 7, 8,    // �ﰢ�� 3
        9, 10, 11   // �ﰢ�� 4
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // ��и� ���� �� VAO ����
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &line_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
