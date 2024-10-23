#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "file_utils.h"
#include <random>
#include <vector>
#include <cmath>

// 랜덤 숫자 생성기
std::random_device rd;
std::mt19937 g(rd());

// 콜백 함수 선언
void drawScene(void);
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);

// 셰이더 관련 변수
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;

// VAO, VBO, EBO
GLuint VAO[2], VBO[2], EBO;

// 전역변수
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

// 사분면별 삼각형 데이터 구조체
struct Triangle {
    float vertices[18]; // 각 삼각형 당 3개의 정점, 각 정점당 6개의 속성 (x, y, z, r, g, b)
};

std::vector<Triangle> triangles[4]; // 사분면별 삼각형 리스트

// 현재 그리기 모드 (면 또는 선)
bool is_face = true;

// 함수 선언
int click_area(GLclampf x, GLclampf y);
void create_tri(int clicked_area, GLclampf x, GLclampf y);
void init_buffer();
void init_tri();

// 선 데이터 (x축과 y축을 그리기 위한 선)
float line[] = {
    -1.0f, 0.0f, 0.0f, // x축 왼쪽
    1.0f, 0.0f, 0.0f,  // x축 오른쪽
    0.0f, -1.0f, 0.0f, // y축 아래쪽
    0.0f, 1.0f, 0.0f   // y축 위쪽
};

int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100); // 창 위치
    glutInitWindowSize(width, height);
    glutCreateWindow("실습 8: 사분면별 삼각형 그리기");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }

    // 셰이더 컴파일 및 프로그램 링크
    vertexShader = compileShader("vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = compileShader("fragment.glsl", GL_FRAGMENT_SHADER);
    shader_program = linkProgram(vertexShader, fragmentShader);

    // 버퍼 초기화 및 삼각형 초기화
    init_buffer();
    init_tri();

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}

void drawScene(void) {
    glClearColor(base_r, base_g, base_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    // 사분면 구분 선 그리기
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINES, 0, 4);

    // 삼각형 그리기
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
        std::cout << "그리기 모드: 면(Face)" << std::endl;
        break;

    case 'b':
        is_face = false;
        std::cout << "그리기 모드: 선(Line)" << std::endl;
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

    // 마우스 좌표를 OpenGL 좌표로 변환
    GLclampf mouse_x = (static_cast<float>(x) / width) * 2.0f - 1.0f;
    GLclampf mouse_y = 1.0f - (static_cast<float>(y) / height) * 2.0f;

    int clicked = click_area(mouse_x, mouse_y);
    if (clicked == -1) {
        std::cout << "클릭 위치가 사분면 밖입니다." << std::endl;
        return;
    }

    if (button == GLUT_LEFT_BUTTON) {
        // 해당 사분면의 삼각형 삭제
        triangles[clicked].clear();
        // 새로운 삼각형 생성
        create_tri(clicked, mouse_x, mouse_y);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        // 해당 사분면에 삼각형 추가 (최대 3개)
        if (triangles[clicked].size() < 3) {
            create_tri(clicked, mouse_x, mouse_y);
        }
        else {
            std::cout << "사분면 " << clicked + 1 << "에 더 이상 삼각형을 추가할 수 없습니다." << std::endl;
        }
    }

    glutPostRedisplay();
}

int click_area(GLclampf x, GLclampf y) {
    if (x < 0.0f && y > 0.0f) {
        return 0; // 1사분면
    }
    else if (x > 0.0f && y > 0.0f) {
        return 1; // 2사분면
    }
    else if (x < 0.0f && y < 0.0f) {
        return 2; // 3사분면
    }
    else if (x > 0.0f && y < 0.0f) {
        return 3; // 4사분면
    }
    return -1; // 사분면 밖
}

void create_tri(int clicked_area, GLclampf x, GLclampf y) {
    // 삼각형의 색상과 크기 랜덤 설정
    GLclampf tri_r = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_g = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_b = static_cast<GLclampf>(g() % 100) / 100.0f;
    float size = 0.1f + static_cast<float>(g() % 50) / 500.0f;

    //// 이등변 삼각형의 정점 계산

    float height = size * std::sqrt(3.0f);  // 높이를 두 배로 설정
    float triX1 = x;
    float triY1 = y + height / 2.0f;
    float triX2 = x - size / 2.0f;
    float triY2 = y - height / 2.0f;
    float triX3 = x + size / 2.0f;
    float triY3 = y - height / 2.0f;

    // 삼각형 데이터 저장
    Triangle tri;
    tri.vertices[0] = triX1; tri.vertices[1] = triY1; tri.vertices[2] = 0.0f;
    tri.vertices[3] = tri_r; tri.vertices[4] = tri_g; tri.vertices[5] = tri_b;

    tri.vertices[6] = triX2; tri.vertices[7] = triY2; tri.vertices[8] = 0.0f;
    tri.vertices[9] = tri_r; tri.vertices[10] = tri_g; tri.vertices[11] = tri_b;

    tri.vertices[12] = triX3; tri.vertices[13] = triY3; tri.vertices[14] = 0.0f;
    tri.vertices[15] = tri_r; tri.vertices[16] = tri_g; tri.vertices[17] = tri_b;

    triangles[clicked_area].push_back(tri);

    std::cout << "사분면 " << clicked_area + 1 << "에 삼각형을 생성했습니다. 현재 삼각형 수: " << triangles[clicked_area].size() << std::endl;
}

void init_buffer() {
    // 삼각형 VAO 설정
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    // 정점 속성 설정
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 설정
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 선 VAO 설정
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void init_tri() {
    // 각 사분면의 중앙에 삼각형을 하나씩 생성
    create_tri(0, -0.5f, 0.5f);  // 1사분면
    create_tri(1, 0.5f, 0.5f);   // 2사분면
    create_tri(2, -0.5f, -0.5f); // 3사분면
    create_tri(3, 0.5f, -0.5f);  // 4사분면
}
