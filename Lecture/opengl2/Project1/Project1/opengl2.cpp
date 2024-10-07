#include <iostream>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

// 전역 변수 및 초기 설정
GLfloat base_r = 0.5f, base_g = 0.5f, base_b = 0.5f; // 배경색 초기화

// 사각형 구조체 정의 (이름 충돌 방지를 위해 MyRectangle 사용)
struct MyRectangle {
    GLfloat x1, y1; // 좌측 하단 좌표값
    GLfloat x2, y2; // 우측 상단 좌표값
    GLfloat r, g, b; // 색상
};

// 사각형 4개를 저장할 배열
MyRectangle rects[4];

// 창 크기 초기값
int window_width = 800;
int window_height = 600;

// 랜덤 색상 생성 함수
void generateRandomColor(GLfloat& r, GLfloat& g, GLfloat& b) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    r = static_cast<GLfloat>(dis(gen));
    g = static_cast<GLfloat>(dis(gen));
    b = static_cast<GLfloat>(dis(gen));
}

// 사각형 초기화 함수
void initRectangles() {
    // 사각형 1 (좌상단)
    rects[0].x1 = -1.0f;
    rects[0].y1 = 0.0f;
    rects[0].x2 = 0.0f;
    rects[0].y2 = 1.0f;
    rects[0].r = 1.0f; // 빨간색
    rects[0].g = 0.0f;
    rects[0].b = 0.0f;

    // 사각형 2 (우상단)
    rects[1].x1 = 0.0f;
    rects[1].y1 = 0.0f;
    rects[1].x2 = 1.0f;
    rects[1].y2 = 1.0f;
    rects[1].r = 0.0f; // 초록색
    rects[1].g = 1.0f;
    rects[1].b = 0.0f;

    // 사각형 3 (좌하단)
    rects[2].x1 = -1.0f;
    rects[2].y1 = -1.0f;
    rects[2].x2 = 0.0f;
    rects[2].y2 = 0.0f;
    rects[2].r = 0.0f; // 파란색
    rects[2].g = 0.0f;
    rects[2].b = 1.0f;

    // 사각형 4 (우하단)
    rects[3].x1 = 0.0f;
    rects[3].y1 = -1.0f;
    rects[3].x2 = 1.0f;
    rects[3].y2 = 0.0f;
    rects[3].r = 1.0f; // 노란색
    rects[3].g = 1.0f;
    rects[3].b = 0.0f;
}

// 디스플레이 콜백 함수
void display() {
    // 배경색 설정 및 화면 초기화
    glClearColor(base_r, base_g, base_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 각 사각형 그리기
    for (int i = 0; i < 4; ++i) {
        glColor3f(rects[i].r, rects[i].g, rects[i].b);
        glRectf(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);
    }

    // 화면 업데이트
    glutSwapBuffers();
}

// 마우스 좌표를 OpenGL 좌표로 변환하는 함수
void convertToOpenGLCoords(int x, int y, GLfloat& ogl_x, GLfloat& ogl_y) {
    ogl_x = ((GLfloat)x / (window_width / 2)) - 1.0f;
    ogl_y = 1.0f - ((GLfloat)y / (window_height / 2));
}

// 클릭한 영역의 사각형 인덱스를 반환하는 함수
int getClickedRectangleIndex(GLfloat x, GLfloat y) {
    for (int i = 0; i < 4; ++i) {
        if (x >= rects[i].x1 && x <= rects[i].x2 && y >= rects[i].y1 && y <= rects[i].y2) {
            return i;
        }
    }
    return -1;
}

// 사분면 확인 함수
int getQuadrant(GLfloat x, GLfloat y) {
    if (x < 0 && y > 0) return 0;  // 좌상단
    if (x > 0 && y > 0) return 1;  // 우상단
    if (x < 0 && y < 0) return 2;  // 좌하단
    if (x > 0 && y < 0) return 3;  // 우하단
    return -1;
}

// 마우스 콜백 함수
void mouseCallback(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return; // 버튼이 눌렸을 때만 처리

    GLfloat ogl_x, ogl_y;
    convertToOpenGLCoords(x, y, ogl_x, ogl_y);

    int rectIndex = getClickedRectangleIndex(ogl_x, ogl_y);
    bool insideRect = (rectIndex != -1);

    if (button == GLUT_LEFT_BUTTON) {
        if (insideRect) {
            // 사각형 내부 클릭: 사각형 색상 랜덤 변경
            generateRandomColor(rects[rectIndex].r, rects[rectIndex].g, rects[rectIndex].b);
        }
        else {
            // 사각형 외부 클릭: 배경색 랜덤 변경
            generateRandomColor(base_r, base_g, base_b);
        }
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        if (insideRect) {
            // 사각형 내부 클릭: 사각형 크기 축소 (최소 크기 제한)
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
            // 사각형 외부 클릭: 해당 사분면의 사각형 크기 확대
            int quadrant = getQuadrant(ogl_x, ogl_y);
            if (quadrant != -1) {
                rects[quadrant].x1 -= 0.05f;
                rects[quadrant].y1 -= 0.05f;
                rects[quadrant].x2 += 0.05f;
                rects[quadrant].y2 += 0.05f;
            }
        }
    }

    // 화면 갱신 요청
    glutPostRedisplay();
}

// 키보드 콜백 함수
void keyboardCallback(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') {
        glutLeaveMainLoop();
    }
}

// 윈도우 리사이즈 콜백 함수
void reshapeCallback(int w, int h) {
    window_width = w;
    window_height = h;
    glViewport(0, 0, w, h);
}

// 초기화 함수
void initialize() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // GLEW 초기화 실패
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }

    // 사각형 초기화
    initRectangles();
}

// 메인 함수
int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 더블 버퍼링과 RGBA 컬러 모드 사용
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("실습 2");

    // GLEW 및 사각형 초기화
    initialize();

    // 콜백 함수 등록
    glutDisplayFunc(display);
    glutReshapeFunc(reshapeCallback);
    glutMouseFunc(mouseCallback);
    glutKeyboardFunc(keyboardCallback);

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}
