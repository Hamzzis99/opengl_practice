#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "file_utils.h" // 변경된 헤더 파일
#include <random>
#include <vector>

// 셰이더 파일 이름 정의
#define vertex_shader_code "vertex.glsl"
#define fragment_shader_code "fragment.glsl"

// 난수 생성기
std::random_device rd;
std::mt19937 g(rd());

//------------------------------------------------------
// 콜백 함수
void drawScene(void);
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void update(int);

//------------------------------------------------------
// 셰이더 관련 선언
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;
GLuint VAO, VBO, EBO;

void init_buffer();

//------------------------------------------------------
// 전역 변수
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

typedef struct triangle {
    std::vector<GLclampf> vertices;
    GLclampf r, g, b;
    float dx{ 0.01f }, dy{ -0.01f };
    float acc_dx{};
    float acc_dy{};
    float angl{};
};

int motion_type{};

triangle tri_list[4];

unsigned int index_buffer[] = {
    0, 1,
    1, 2,
    2, 0,

    3, 4,
    4, 5,
    5, 3,

    6, 7,
    7, 8,
    8, 6,

    9, 10,
    10, 11,
    11, 9
};

//------------------------------------------------------
// 필요한 함수 선언
int click_area(GLclampf x, GLclampf y);
void create_tri(int, GLclampf, GLclampf);
void init_tri();

//------------------------------------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("OpenGL Example");

    glewExperimental = GL_TRUE;
    glewInit();

    // file_utils를 사용하여 셰이더 컴파일 및 링크
    vertexShader = compileShader(vertex_shader_code, GL_VERTEX_SHADER);
    fragmentShader = compileShader(fragment_shader_code, GL_FRAGMENT_SHADER);
    shader_program = linkProgram(vertexShader, fragmentShader);

    // 셰이더 객체 삭제 (프로그램에 이미 링크되었으므로)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutTimerFunc(1000 / 60, update, 0);

    init_buffer();
    init_tri();

    glutMainLoop();

    return 0;
}

void drawScene(void) {
    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    glValidateProgram(shader_program);

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 12);

    glutSwapBuffers();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        if (motion_type != 1)
            motion_type = 1;
        else
            motion_type = 0;
        break;

    case '2':
        if (motion_type != 2)
            motion_type = 2;
        else
            motion_type = 0;
        break;

    case '3':
        if (motion_type != 3)
            motion_type = 3;
        else
            motion_type = 0;
        break;

    case '4':
        if (motion_type != 4)
            motion_type = 4;
        else
            motion_type = 0;
        break;

    case 'q':
        glutLeaveMainLoop();
        break;
    }
    std::cout << "Motion Type: " << motion_type << std::endl;
    glutPostRedisplay();
}

void init_buffer() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 4개의 삼각형, 각 삼각형당 3개의 정점, 각 정점당 6개의 float (위치 + 색상)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3 * 4, nullptr, GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // EBO (필요 시)
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer), index_buffer, GL_STATIC_DRAW);
}

void Mouse(int button, int state, int x, int y) {
    GLclampf mouse_x = (float)(x - (float)width / 2.0f) * (1.0f / ((float)width / 2.0f));
    GLclampf mouse_y = -(float)(y - (float)height / 2.0f) * (1.0f / ((float)height / 2.0f));
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int clicked = click_area(mouse_x, mouse_y);
        if (clicked != -1) {
            std::cout << "Clicked area: " << clicked << std::endl;
            create_tri(clicked, mouse_x, mouse_y);
        }
    }
}

int click_area(GLclampf x, GLclampf y) {
    if (-1 < x && x < 0 && 0 < y && y < 1) {
        return 0;
    }
    else if (0 < x && x < 1 && 0 < y && y < 1) {
        return 1;
    }
    else if (-1 < x && x < 0 && -1 < y && y < 0) {
        return 2;
    }
    else if (0 < x && x < 1 && -1 < y && y < 0) {
        return 3;
    }
    return -1; // 해당 영역이 아닌 경우
}

void create_tri(int clicked_area, GLclampf x, GLclampf y) {
    if (clicked_area < 0 || clicked_area >= 4) return;

    GLclampf tri_r = float(g() % 100) / 100.0f;
    GLclampf tri_g = float(g() % 100) / 100.0f;
    GLclampf tri_b = float(g() % 100) / 100.0f;

    float tri_vertex1[18] = {
        x, y + 0.2f, 0.0f, tri_r, tri_g, tri_b,
        x - 0.1f, y - 0.1f, 0.0f, tri_r, tri_g, tri_b,
        x + 0.1f, y - 0.1f, 0.0f, tri_r, tri_g, tri_b,
    };
    tri_list[clicked_area].vertices = {
        tri_vertex1[0], tri_vertex1[1], tri_vertex1[2],
        tri_vertex1[6], tri_vertex1[7], tri_vertex1[8],
        tri_vertex1[12], tri_vertex1[13], tri_vertex1[14],
    };
    tri_list[clicked_area].r = tri_r;
    tri_list[clicked_area].g = tri_g;
    tri_list[clicked_area].b = tri_b;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(tri_vertex1) * clicked_area, sizeof(tri_vertex1), tri_vertex1);
}

void init_tri() {
    GLclampf tri_r;
    GLclampf tri_g;
    GLclampf tri_b;

    GLclampf x[4];
    GLclampf y[4];

    for (int i = 0; i < 4; ++i) {
        switch (i) {
        case 0:
            x[i] = float(g() % 800) / 1000.0f - 0.9f;
            y[i] = float(g() % 800) / 1000.0f + 0.2f;
            break;

        case 1:
            x[i] = float(g() % 800) / 1000.0f + 0.1f;
            y[i] = float(g() % 800) / 1000.0f + 0.2f;
            break;

        case 2:
            x[i] = float(g() % 800) / 1000.0f - 0.9f;
            y[i] = float(g() % 800) / 1000.0f - 0.8f;
            break;

        case 3:
            x[i] = float(g() % 800) / 1000.0f + 0.1f;
            y[i] = float(g() % 800) / 1000.0f - 0.8f;
            break;
        }
        tri_r = float(g() % 100) / 100.0f;
        tri_g = float(g() % 100) / 100.0f;
        tri_b = float(g() % 100) / 100.0f;
        float tri_vertex1[18] = {
            x[i], y[i] + 0.1f, 0.0f, tri_r, tri_g, tri_b,
            x[i] - 0.1f, y[i] - 0.2f, 0.0f, tri_r, tri_g, tri_b,
            x[i] + 0.1f, y[i] - 0.2f, 0.0f, tri_r, tri_g, tri_b,
        };
        tri_list[i].vertices = {
            tri_vertex1[0], tri_vertex1[1], tri_vertex1[2],
            tri_vertex1[6], tri_vertex1[7], tri_vertex1[8],
            tri_vertex1[12], tri_vertex1[13], tri_vertex1[14],
        };
        tri_list[i].r = tri_r;
        tri_list[i].g = tri_g;
        tri_list[i].b = tri_b;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(tri_vertex1) * i, sizeof(tri_vertex1), tri_vertex1);
    }
}

void update(int) {
    // motion_type에 따른 애니메이션 로직
    switch (motion_type) {
    case 1:
        // bounce 모션 구현
        // 여기에 bounce 모션 관련 코드를 추가하세요
        break;

    case 2:
        // zigzag 모션 구현
        // 여기에 zigzag 모션 관련 코드를 추가하세요
        break;

    case 3:
        // square spiral 모션 구현
        // 여기에 square spiral 모션 관련 코드를 추가하세요
        break;

    case 4:
        // circle spiral 모션 구현
        // 여기에 circle spiral 모션 관련 코드를 추가하세요
        break;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}
