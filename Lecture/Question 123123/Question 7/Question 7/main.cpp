#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "file_utils.h" // 수정된 헤더 파일 포함
#include <random>
#include <vector>
#include <cmath> // 삼각형 높이를 계산하기 위해 필요

// 랜덤 숫자 생성기
std::random_device rd;
std::mt19937 g(rd());

//------------------------------------------------------
// 콜백 함수 선언
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
//------------------------------------------------------
// 셰이더 용 선언
GLuint shader_program;
GLuint vertexShader;
GLuint fragmentShader;
GLuint VAO[2], VBO, EBO, line_buffer;

// 전역변수
GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;
GLint width{ 800 }, height{ 600 };

// 사분면별 삼각형 인덱스
unsigned int index_buffer[] = {
    0, 1, 2,    // 삼각형 1
    3, 4, 5,    // 삼각형 2
    6, 7, 8,    // 삼각형 3
    9, 10, 11   // 삼각형 4
};

// 사분면을 선으로 그릴지 면으로 그릴지 결정하는 변수
bool is_face = true;

// 사분면을 구분하는 선
float line[] = {
    0.0f, 1.0f, 0.0f, // y축 위쪽
    0.0f, -1.0f, 0.0f, // y축 아래쪽

    -1.0f, 0.0f, 0.0f, // x축 왼쪽
    1.0f, 0.0f, 0.0f   // x축 오른쪽
};

//------------------------------------------------------
// 필요한 함수 선언
int click_area(GLclampf x, GLclampf y);
void create_tri(int clicked_area, GLclampf x, GLclampf y);
void init_tri();
void init_buffer();

//------------------------------------------------------
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

    // 디버그 출력 활성화 (선택 사항)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
            if (type == GL_DEBUG_TYPE_ERROR)
                std::cerr << "GL ERROR: " << message << std::endl;
        }, nullptr);

    // 셰이더 컴파일 및 프로그램 링크
    vertexShader = compileShader("vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = compileShader("fragment.glsl", GL_FRAGMENT_SHADER);
    shader_program = linkProgram(vertexShader, fragmentShader);
    if (shader_program == 0) {
        std::cerr << "셰이더 프로그램 링크 실패" << std::endl;
        return -1;
    }

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);

    // 버퍼 초기화 및 삼각형 초기화
    init_buffer();
    init_tri();

    // GLUT 메인 루프 시작
    glutMainLoop();

    return 0;
}

GLvoid drawScene(GLvoid) {
    // 배경 색상 설정 및 화면 클리어
    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 셰이더 프로그램 사용
    glUseProgram(shader_program);

    // 사분면 구분 선 그리기
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINES, 0, 4); // x축과 y축을 그리는 선은 4개의 정점

    // 삼각형 그리기
    glBindVertexArray(VAO[0]);
    if (is_face) {
        glDrawArrays(GL_TRIANGLES, 0, 12); // 4사분면 각각 3개의 정점, 총 12개
    }
    else {
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // 선 모드로 24개의 인덱스
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

GLvoid Mouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    // 마우스 좌표를 OpenGL NDC 좌표로 변환
    GLclampf mouse_x = (static_cast<float>(x) / width) * 2.0f - 1.0f;
    GLclampf mouse_y = 1.0f - (static_cast<float>(y) / height) * 2.0f;

    int clicked = click_area(mouse_x, mouse_y);
    if (clicked == -1) {
        std::cout << "클릭 위치가 사분면 밖입니다." << std::endl;
        return;
    }

    if (button == GLUT_LEFT_BUTTON) {
        // 왼쪽 마우스 클릭: 해당 사분면의 모든 삼각형 삭제 및 새로운 삼각형 생성
        std::cout << "왼쪽 클릭: 사분면 " << clicked + 1 << "의 모든 삼각형을 삭제하고 새로운 삼각형을 생성합니다." << std::endl;
        // VBO 초기화 (사분면별 삼각형 삭제)
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 18 * clicked, sizeof(float) * 18, nullptr);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        // 오른쪽 마우스 클릭: 해당 사분면에 최대 3개의 삼각형 추가
        std::cout << "오른쪽 클릭: 사분면 " << clicked + 1 << "에 삼각형을 추가합니다." << std::endl;
        // 삼각형 생성 함수 호출
        create_tri(clicked, mouse_x, mouse_y);
    }

    glutPostRedisplay();
}

int click_area(GLclampf x, GLclampf y) {
    if (-1.0f < x && x < 0.0f && 0.0f < y && y < 1.0f) {
        return 0; // 1사분면
    }
    else if (0.0f < x && x < 1.0f && 0.0f < y && y < 1.0f) {
        return 1; // 2사분면
    }
    else if (-1.0f < x && x < 0.0f && -1.0f < y && y < 0.0f) {
        return 2; // 3사분면
    }
    else if (0.0f < x && x < 1.0f && -1.0f < y && y < 0.0f) {
        return 3; // 4사분면
    }

    return -1; // 사분면 밖
}

void create_tri(int clicked_area, GLclampf x, GLclampf y) {
    // 사분면별 삼각형 카운트 관리 (최대 3개)
    static int tri_counts[4] = { 0, 0, 0, 0 };
    const int MAX_TRIANGLES = 3;

    if (tri_counts[clicked_area] >= MAX_TRIANGLES) {
        std::cout << "사분면 " << clicked_area + 1 << "에 더 이상 삼각형을 추가할 수 없습니다." << std::endl;
        return;
    }

    // 삼각형의 색상과 크기 랜덤 설정
    GLclampf tri_r = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_g = static_cast<GLclampf>(g() % 100) / 100.0f;
    GLclampf tri_b = static_cast<GLclampf>(g() % 100) / 100.0f;
    float size = 0.1f + static_cast<float>(g() % 50) / 500.0f; // 0.1f ~ 0.19f

    // 이등변 삼각형의 정점 계산
    float height = size * (std::sqrt(3.0f) / 2.0f);
    float triX1 = x;
    float triY1 = y + height / 2.0f;
    float triX2 = x - size / 2.0f;
    float triY2 = y - height / 2.0f;
    float triX3 = x + size / 2.0f;
    float triY3 = y - height / 2.0f;

    // 삼각형 데이터 저장
    float tri_vertices[18] = {
        triX1, triY1, 0.0f, tri_r, tri_g, tri_b,
        triX2, triY2, 0.0f, tri_r, tri_g, tri_b,
        triX3, triY3, 0.0f, tri_r, tri_g, tri_b
    };

    // VBO 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 18 * tri_counts[clicked_area],
        sizeof(tri_vertices), tri_vertices);

    tri_counts[clicked_area]++;
    std::cout << "삼각형을 그렸습니다. 사분면: " << clicked_area + 1
        << ", 현재 삼각형 수: " << tri_counts[clicked_area] << std::endl;
}

void init_tri() {
    // 초기 사분면에 삼각형 1개씩 그리기
    for (int i = 0; i < 4; ++i) {
        // 사분면 중앙 위치 설정
        GLclampf x, y;
        switch (i) {
        case 0: // 1사분면
            x = -0.5f;
            y = 0.5f;
            break;
        case 1: // 2사분면
            x = 0.5f;
            y = 0.5f;
            break;
        case 2: // 3사분면
            x = -0.5f;
            y = -0.5f;
            break;
        case 3: // 4사분면
            x = 0.5f;
            y = -0.5f;
            break;
        }

        create_tri(i, x, y);
    }
}

void init_buffer() {
    // 삼각형 VAO 설정
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 각 사분면별로 최대 3개의 삼각형, 각 삼각형 당 3개의 정점, 각 정점 당 6개의 속성 (x, y, z, r, g, b)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * 3 * 4, nullptr, GL_DYNAMIC_DRAW);

    // 정점 속성 설정
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 설정
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // EBO 설정 (사분면별 삼각형 인덱스)
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // 사분면별 삼각형 인덱스 (각 삼각형 당 3개의 인덱스)
    unsigned int indices[12] = {
        0, 1, 2,    // 삼각형 1
        3, 4, 5,    // 삼각형 2
        6, 7, 8,    // 삼각형 3
        9, 10, 11   // 삼각형 4
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // 사분면 구분 선 VAO 설정
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &line_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
