#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <random>
#include <vector>
#include <cmath> // sqrt, cos, sin 함수 사용을 위해 포함
#include "file_utils.h" // 셰이더 파일 로드를 위한 헤더

// M_PI 상수 정의
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 셰이더 파일 경로
#define VERTEX_SHADER_PATH "vertex.glsl"
#define FRAGMENT_SHADER_PATH "fragment.glsl"

// 랜덤 숫자 생성기
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1.0f, 1.0f);
std::uniform_real_distribution<> color_dis(0.0f, 1.0f);
std::uniform_int_distribution<> int_dis(2, 3);
std::uniform_int_distribution<> bool_dis(0, 1);

// 콜백 함수 선언
void drawScene(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void timer(int value);

// 셰이더 프로그램
GLuint shaderProgram;

// VAO, VBO
GLuint VAO, VBO;

// 스파이럴 구조체
struct SpiralPoint {
    float x, y, z;     // 위치
    float r, g, b;     // 색상
};

// 스파이럴 점들을 저장할 벡터
std::vector<std::vector<SpiralPoint>> spirals; // 여러 개의 스파이럴 저장

// 그리기 모드 (점 또는 선)
bool isLineMode = false;

// 애니메이션 관련 변수
int currentPoint = 0;
bool isAnimating = false;

// 윈도우 크기
int width = 800;
int height = 600;

// 배경색
GLclampf bg_r = 1.0f, bg_g = 1.0f, bg_b = 1.0f;

int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("Spiral Animation");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    glewInit();

    // 셰이더 컴파일 및 링크
    GLuint vertexShader = compileShader(VERTEX_SHADER_PATH, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(FRAGMENT_SHADER_PATH, GL_FRAGMENT_SHADER);
    shaderProgram = linkProgram(vertexShader, fragmentShader);

    // VAO 및 VBO 설정
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgram);

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}

void createSpiral(float centerX, float centerY, int turns, bool clockwise) {
    const int pointsPerTurn = 100; // 바퀴당 점 개수
    float angleIncrement = (2.0f * M_PI) / pointsPerTurn;
    float radiusIncrement = 0.005f; // 반지름 증가량

    std::vector<SpiralPoint> newSpiral;

    // 랜덤 색상
    float r = color_dis(gen);
    float g = color_dis(gen);
    float b = color_dis(gen);

    // 안에서 밖으로 스파이럴 생성
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

    // 밖에서 안으로 스파이럴 생성 (반대 방향)
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
    // 배경색 설정
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

    // VBO에 전체 스파이럴 데이터를 업로드
    std::vector<SpiralPoint> allPoints;
    for (const auto& spiral : spirals) {
        allPoints.insert(allPoints.end(), spiral.begin(), spiral.end());
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, allPoints.size() * sizeof(SpiralPoint), allPoints.data(), GL_DYNAMIC_DRAW);

    // 정점 속성 설정
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
        currentPoint += 5; // 한번에 5개의 점씩 증가

        int totalPoints = 0;
        for (const auto& spiral : spirals) {
            totalPoints += spiral.size();
        }

        if (currentPoint >= totalPoints) {
            isAnimating = false;
            currentPoint = 0;
        }
        else {
            glutTimerFunc(16, timer, 0); // 약 60FPS
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
        // 배경색 변경
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
        // 마우스 좌표를 OpenGL 좌표로 변환
        float mouseX = (float)(x) / (width / 2.0f) - 1.0f;
        float mouseY = 1.0f - (float)(y) / (height / 2.0f);

        int turns = int_dis(gen);
        bool clockwise = bool_dis(gen);

        spirals.clear();
        createSpiral(mouseX, mouseY, turns, clockwise);

        // 배경색 변경
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
