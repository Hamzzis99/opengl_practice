#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>

// 함수 선언
GLvoid RenderScene(GLvoid);
GLvoid HandleReshape(int width, int height);
GLvoid HandleKeyboard(unsigned char key, int x, int y);
GLvoid HandleMouse(int button, int state, int x, int y);
GLvoid HandleMotion(int x, int y);

// 배경 색상 구성 요소
GLclampf backgroundRed = 1.0f;
GLclampf backgroundGreen = 1.0f;
GLclampf backgroundBlue = 1.0f;

// 랜덤 속성을 가진 사각형 구조체
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

// 랜덤 숫자 생성기 설정
std::random_device randomDevice;
std::mt19937 generator(randomDevice());

// 사각형을 저장할 배열 (마지막 인덱스는 지우개용)
RandomRectangle rectangleList[41];

// 유틸리티 함수 선언
void InitializeRectangles();
void MergeRectangles(int targetIndex);
bool IsOverlap(int rectIndex1, int rectIndex2);

// 지우개 크기 변수
GLclampf eraserWidth{};
GLclampf eraserHeight{};

// 드래그 상태를 추적하는 플래그
bool isDragging = false;

// 프로그램의 진입점
int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Rectangle Example");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "GLEW 성공적으로 초기화됨\n";
    }

    // 콜백 함수 등록
    glutDisplayFunc(RenderScene);
    glutReshapeFunc(HandleReshape);
    glutKeyboardFunc(HandleKeyboard);
    glutMouseFunc(HandleMouse);
    glutMotionFunc(HandleMotion);

    // 사각형 초기화
    InitializeRectangles();

    // GLUT 이벤트 처리 루프로 진입
    glutMainLoop();

    return 0;
}

// 장면을 렌더링하는 함수
GLvoid RenderScene(GLvoid) {
    // 배경 색상을 설정하고 버퍼를 지움
    glClearColor(backgroundRed, backgroundGreen, backgroundBlue, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 생성된 모든 사각형을 그리기
    for (const RandomRectangle& rect : rectangleList) {
        if (rect.isCreated) {
            glColor3f(rect.colorR, rect.colorG, rect.colorB);
            glRectf(rect.startX, rect.startY, rect.endX, rect.endY);
        }
    }

    // 렌더링된 이미지를 표시하기 위해 버퍼 교체
    glutSwapBuffers();
}

// 창 크기 변경을 처리하는 함수
GLvoid HandleReshape(int width, int height) {
    glViewport(0, 0, width, height);
}

// 키보드 입력을 처리하는 함수
GLvoid HandleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'r':
        InitializeRectangles(); // 'r' 키를 누르면 사각형 초기화
        break;
    case 'q':
        glutLeaveMainLoop(); // 'q' 키를 누르면 프로그램 종료
        break;
    }

    // 화면 다시 그리기 요청
    glutPostRedisplay();
}

// 사각형을 랜덤한 위치와 색상으로 초기화하는 함수
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
    rectangleList[40].colorR = 0.0f; // 지우개 색상 초기화
    rectangleList[40].colorG = 0.0f;
    rectangleList[40].colorB = 0.0f;
}

// 마우스 이벤트를 처리하는 함수
GLvoid HandleMouse(int button, int state, int x, int y) {
    // 마우스 좌표를 OpenGL 좌표로 변환
    GLclampf mouseX = (static_cast<float>(x - 800 / 2.0) * (1.0f / (800 / 2.0)));
    GLclampf mouseY = -(static_cast<float>(y - 600 / 2.0) * (1.0f / (600 / 2.0)));

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 왼쪽 버튼을 누르면 지우개 사각형 생성
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
        // 왼쪽 버튼을 놓으면 지우개 사각형 제거
        rectangleList[40].isCreated = false;
        isDragging = false;
    }
    glutPostRedisplay();
}

// 마우스 이동을 처리하는 함수
GLvoid HandleMotion(int x, int y) {
    // 마우스 좌표를 OpenGL 좌표로 변환
    GLclampf mouseX = (static_cast<float>(x - 800 / 2.0) * (1.0f / (800 / 2.0)));
    GLclampf mouseY = -(static_cast<float>(y - 600 / 2.0) * (1.0f / (600 / 2.0)));

    if (isDragging) {
        // 지우개 사각형의 위치 업데이트
        rectangleList[40].startX = mouseX - eraserWidth;
        rectangleList[40].startY = mouseY + eraserHeight;
        rectangleList[40].endX = mouseX + eraserWidth;
        rectangleList[40].endY = mouseY - eraserHeight;

        // 다른 사각형과 겹치는지 확인하고 병합
        for (int i = 0; i < 40; ++i) {
            if (rectangleList[40].isCreated && rectangleList[i].isCreated && IsOverlap(i, 40)) {
                MergeRectangles(i);
                break;
            }
        }
    }
    glutPostRedisplay();
}

// 사각형을 병합하는 함수
void MergeRectangles(int targetIndex) {
    // 대상 사각형을 제거하고 지우개 크기 증가
    rectangleList[targetIndex].isCreated = false;
    eraserWidth += 0.025f;
    eraserHeight += 0.025f;

    // 지우개의 색상을 병합된 사각형의 색상으로 변경
    rectangleList[40].colorR = rectangleList[targetIndex].colorR;
    rectangleList[40].colorG = rectangleList[targetIndex].colorG;
    rectangleList[40].colorB = rectangleList[targetIndex].colorB;

    std::cout << "병합된 인덱스: " << targetIndex << std::endl;
}

// 두 사각형이 겹치는지 확인하는 함수
bool IsOverlap(int rectIndex1, int rectIndex2) {
    RandomRectangle rect1 = rectangleList[rectIndex1];
    RandomRectangle rect2 = rectangleList[rectIndex2];

    // 사각형이 겹치지 않으면 false 반환
    if (rect1.endX < rect2.startX || rect1.startX > rect2.endX ||
        rect1.endY > rect2.startY || rect1.startY < rect2.endY) {
        return false;
    }

    return true;
}
