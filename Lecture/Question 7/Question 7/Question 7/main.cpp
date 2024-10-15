#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <random>
#include <vector>
#include "file_utils.h"



// 셰이더 파일 경로
#define VERTEX_SHADER_PATH "vertex.glsl"
#define FRAGMENT_SHADER_PATH "fragment.glsl"

// 랜덤 숫자 생성기
std::random_device rd;
std::mt19937 rng(rd());

// 콜백 함수 선언
void displayCallback();
void reshapeCallback(int width, int height);
void keyboardCallback(unsigned char key, int x, int y);
void mouseCallback(int button, int state, int x, int y);

// 셰이더 관련 변수
GLuint shaderProgram;
GLuint vertexShader;
GLuint fragmentShader;

// VAO, VBO, EBO
GLuint VAOs[4], VBOs[4], EBO;

// 배경 색상
GLclampf clearColorR = 1.0f;
GLclampf clearColorG = 1.0f;
GLclampf clearColorB = 1.0f;

// 윈도우 크기
GLint windowWidth = 800, windowHeight = 600;

// 도형 카운트
int pointCount = 0;
int lineCount = 0;
int triangleCount = 0;
int rectangleCount = 0;

// 도형 구조체
struct Shape {
    std::vector<GLclampf> vertices; // 좌표 및 색상
    GLclampf r, g, b;
    char type; // 'p', 'l', 't', 'r'
};

Shape shapesList[100];
int currentIndex = 0;
unsigned int rectangleIndices[100][6];

// 현재 선택된 도구 ('p', 'l', 't', 'r', 'n')
char currentTool = 'n';

// 선 그리기 상태
bool isFirstLineClick = true;
float lineStartX, lineStartY;

// 삼각형 그리기 상태
int triangleClickCount = 0;
float triVertices[6];

// 사각형 그리기 상태
// 단일 클릭으로 중앙에 그리기 때문에 추가 상태 필요 없음

// 도형 생성 함수들
void createPoint(float x, float y);
void createLine(float x, float y);
void createTriangle(float x, float y);
void createRectangle(float x, float y);
void clearBuffers();
void moveShape(char direction);
GLuint compileShader(const char* filePath, GLenum shaderType);

int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100); // 창 위치
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Drawing Tool");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(glewStatus) << std::endl;
        return -1;
    }

    // 디버그 출력 활성화
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
            if (type == GL_DEBUG_TYPE_ERROR)
                std::cerr << "GL ERROR: " << message << std::endl;
        }, nullptr);

    // 셰이더 컴파일 및 프로그램 링크
    vertexShader = compileShader(VERTEX_SHADER_PATH, GL_VERTEX_SHADER);
    fragmentShader = compileShader(FRAGMENT_SHADER_PATH, GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 프로그램 링크 확인
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "셰이더 프로그램 링크 실패:\n" << infoLog << std::endl;
        return -1;
    }

    // 프로그램 유효성 검사
    glValidateProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "셰이더 프로그램 유효성 검사 실패:\n" << infoLog << std::endl;
        return -1;
    }

    // 셰이더 삭제
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // VAO 및 VBO 설정
    for (int i = 0; i < 4; ++i) {
        glGenVertexArrays(1, &VAOs[i]);
        glBindVertexArray(VAOs[i]);

        glGenBuffers(1, &VBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 100 * (i + 1), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // EBO 설정
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 * 100, nullptr, GL_DYNAMIC_DRAW);

    // 콜백 함수 등록
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);

    // GLUT 메인 루프 시작
    glutMainLoop();
    return 0;
}

GLuint compileShader(const char* filePath, GLenum shaderType) {
    char* shaderSource = readShaderSource(filePath);
    if (!shaderSource) {
        std::cerr << "셰이더 파일 읽기 실패: " << filePath << std::endl;
        return 0;
    }

    std::cout << "셰이더 소스 (" << filePath << "):\n" << shaderSource << std::endl;

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    delete[] shaderSource;

    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        std::cerr << typeStr << " 셰이더 컴파일 실패:\n" << infoLog << std::endl;
        return 0;
    }

    std::cout << ((shaderType == GL_VERTEX_SHADER) ? "버텍스" : "프래그먼트") << " 셰이더 컴파일 성공." << std::endl;
    return shader;
}

void displayCallback() {
    glClearColor(clearColorR, clearColorG, clearColorB, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // 점 그리기
    glBindVertexArray(VAOs[0]);
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, pointCount);

    // 선 그리기
    glBindVertexArray(VAOs[1]);
    glDrawArrays(GL_LINES, 0, 2 * lineCount);

    // 삼각형 그리기
    glBindVertexArray(VAOs[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3 * triangleCount);

    // 사각형 그리기
    glBindVertexArray(VAOs[3]);
    glDrawElements(GL_TRIANGLES, 6 * rectangleCount, GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

void reshapeCallback(int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
}

void keyboardCallback(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
    case 'l':
    case 't':
    case 'r':
        currentTool = key;
        std::cout << "현재 도구: " << key << std::endl;
        break;
    case 'w':
    case 'a':
    case 's':
    case 'd':
        moveShape(key);
        break;
    case 'c':
        clearBuffers();
        break;
    case 'q':
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 마우스 좌표를 OpenGL NDC 좌표로 변환
        float ndcX = (static_cast<float>(x) / windowWidth) * 2.0f - 1.0f;
        float ndcY = 1.0f - (static_cast<float>(y) / windowHeight) * 2.0f;

        switch (currentTool) {
        case 'p':
            createPoint(ndcX, ndcY);
            pointCount++;
            break;
        case 'l':
            createLine(ndcX, ndcY);
            break;
        case 't':
            createTriangle(ndcX, ndcY);
            break;
        case 'r':
            createRectangle(ndcX, ndcY);
            break;
        default:
            std::cout << "도구를 선택하세요 (p: 점, l: 선, t: 삼각형, r: 사각형)" << std::endl;
            break;
        }
        glutPostRedisplay();
    }
}

void createPoint(float x, float y) {
    float r = static_cast<float>(rng() % 1000) / 1000.0f;
    float g_val = static_cast<float>(rng() % 1000) / 1000.0f;
    float b = static_cast<float>(rng() % 1000) / 1000.0f;

    shapesList[currentIndex] = { {x, y, 0.0f}, r, g_val, b, 'p' };

    float vertexData[6] = {
        x, y, 0.0f,
        r, g_val, b
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertexData) * pointCount, sizeof(vertexData), vertexData);
    currentIndex++;
}

void createLine(float x, float y) {
    float r = static_cast<float>(rng() % 1000) / 1000.0f;
    float g_val = static_cast<float>(rng() % 1000) / 1000.0f;
    float b = static_cast<float>(rng() % 1000) / 1000.0f;

    if (isFirstLineClick) {
        // 시작점 설정
        lineStartX = x;
        lineStartY = y;
        isFirstLineClick = false;
        std::cout << "선 시작점 설정 완료. 끝점을 클릭하세요." << std::endl;
    }
    else {
        // 끝점 설정 및 선 그리기
        shapesList[currentIndex] = { {lineStartX, lineStartY, 0.0f}, r, g_val, b, 'l' };

        float lineData[12] = {
            lineStartX, lineStartY, 0.0f, r, g_val, b,
            x, y, 0.0f, r, g_val, b
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(lineData) * lineCount, sizeof(lineData), lineData);
        currentIndex++;
        lineCount++;
        isFirstLineClick = true;
    }
}

void createTriangle(float x, float y) {
    float r = static_cast<float>(rng() % 1000) / 1000.0f;
    float g_val = static_cast<float>(rng() % 1000) / 1000.0f;
    float b = static_cast<float>(rng() % 1000) / 1000.0f;

    // 클릭한 점을 중앙으로 삼각형을 생성
    float size = 0.1f; // 삼각형 크기
    float triX1 = x - size / 2.0f;
    float triY1 = y - size / 2.0f;
    float triX2 = x + size / 2.0f;
    float triY2 = y - size / 2.0f;
    float triX3 = x;
    float triY3 = y + size / 2.0f;

    shapesList[currentIndex] = { {triX1, triY1, 0.0f, triX2, triY2, 0.0f, triX3, triY3, 0.0f},
        r, g_val, b, 't' };

    float triData[18] = {
        triX1, triY1, 0.0f, r, g_val, b,
        triX2, triY2, 0.0f, r, g_val, b,
        triX3, triY3, 0.0f, r, g_val, b
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(triData) * triangleCount, sizeof(triData), triData);
    currentIndex++;
    triangleCount++;
}

void createRectangle(float x, float y) {
    float r = static_cast<float>(rng() % 1000) / 1000.0f;
    float g_val = static_cast<float>(rng() % 1000) / 1000.0f;
    float b = static_cast<float>(rng() % 1000) / 1000.0f;

    // 클릭한 점을 중앙으로 사각형을 생성
    float size = 0.2f; // 사각형 크기
    float rectX1 = x - size / 2.0f;
    float rectY1 = y - size / 2.0f;
    float rectX2 = x + size / 2.0f;
    float rectY2 = y - size / 2.0f;
    float rectX3 = x + size / 2.0f;
    float rectY3 = y + size / 2.0f;
    float rectX4 = x - size / 2.0f;
    float rectY4 = y + size / 2.0f;

    shapesList[currentIndex] = { {rectX1, rectY1, 0.0f, rectX2, rectY2, 0.0f, rectX3, rectY3, 0.0f, rectX4, rectY4, 0.0f},
        r, g_val, b, 'r' };

    float rectData[24] = {
        rectX1, rectY1, 0.0f, r, g_val, b,
        rectX2, rectY2, 0.0f, r, g_val, b,
        rectX3, rectY3, 0.0f, r, g_val, b,
        rectX4, rectY4, 0.0f, r, g_val, b
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(rectData) * rectangleCount, sizeof(rectData), rectData);

    // 인덱스 설정 (두 개의 삼각형으로 사각형을 그립니다)
    rectangleIndices[rectangleCount][0] = 4 * rectangleCount;
    rectangleIndices[rectangleCount][1] = 4 * rectangleCount + 2;
    rectangleIndices[rectangleCount][2] = 4 * rectangleCount + 1;
    rectangleIndices[rectangleCount][3] = 4 * rectangleCount;
    rectangleIndices[rectangleCount][4] = 4 * rectangleCount + 3;
    rectangleIndices[rectangleCount][5] = 4 * rectangleCount + 2;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices[rectangleCount]) * rectangleCount, sizeof(rectangleIndices[rectangleCount]), rectangleIndices[rectangleCount]);

    currentIndex++;
    rectangleCount++;
}

void clearBuffers() {
    for (int i = 0; i < 4; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 100 * (i + 1), nullptr, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 * 100, nullptr, GL_DYNAMIC_DRAW);

    pointCount = 0;
    lineCount = 0;
    triangleCount = 0;
    rectangleCount = 0;
    currentIndex = 0;
}

void moveShape(char direction) {
    float dx = 0.0f, dy = 0.0f;
    switch (direction) {
    case 'w': dy = 0.1f; break;
    case 'a': dx = -0.1f; break;
    case 's': dy = -0.1f; break;
    case 'd': dx = 0.1f; break;
    }

    if (currentIndex == 0) return;

    int target = rng() % currentIndex;
    shapesList[target].vertices[0] += dx;
    shapesList[target].vertices[1] += dy;

    switch (shapesList[target].type) {
    case 'p': {
        float vertexData[6] = {
            shapesList[target].vertices[0],
            shapesList[target].vertices[1],
            shapesList[target].vertices[2],
            shapesList[target].r,
            shapesList[target].g,
            shapesList[target].b
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertexData) * target, sizeof(vertexData), vertexData);
        break;
    }
    case 'l': {
        // 선의 끝점을 고정된 크기로 설정
        float lineData[12] = {
            shapesList[target].vertices[0], shapesList[target].vertices[1], shapesList[target].vertices[2],
            shapesList[target].r, shapesList[target].g, shapesList[target].b,
            shapesList[target].vertices[0] + 0.1f, shapesList[target].vertices[1] - 0.1f, shapesList[target].vertices[2],
            shapesList[target].r, shapesList[target].g, shapesList[target].b
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(lineData) * target, sizeof(lineData), lineData);
        break;
    }
    case 't': {
        // 삼각형의 크기를 고정된 크기로 설정
        float size = 0.1f;
        float triX1 = shapesList[target].vertices[0] - size / 2.0f;
        float triY1 = shapesList[target].vertices[1] - size / 2.0f;
        float triX2 = shapesList[target].vertices[0] + size / 2.0f;
        float triY2 = shapesList[target].vertices[1] - size / 2.0f;
        float triX3 = shapesList[target].vertices[0];
        float triY3 = shapesList[target].vertices[1] + size / 2.0f;

        float triData[18] = {
            triX1, triY1, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b,
            triX2, triY2, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b,
            triX3, triY3, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(triData) * target, sizeof(triData), triData);
        break;
    }
    case 'r': {
        // 사각형의 크기를 고정된 크기로 설정
        float size = 0.2f;
        float rectX1 = shapesList[target].vertices[0] - size / 2.0f;
        float rectY1 = shapesList[target].vertices[1] - size / 2.0f;
        float rectX2 = shapesList[target].vertices[0] + size / 2.0f;
        float rectY2 = shapesList[target].vertices[1] - size / 2.0f;
        float rectX3 = shapesList[target].vertices[0] + size / 2.0f;
        float rectY3 = shapesList[target].vertices[1] + size / 2.0f;
        float rectX4 = shapesList[target].vertices[0] - size / 2.0f;
        float rectY4 = shapesList[target].vertices[1] + size / 2.0f;

        float rectData[24] = {
            rectX1, rectY1, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b,
            rectX2, rectY2, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b,
            rectX3, rectY3, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b,
            rectX4, rectY4, 0.0f, shapesList[target].r, shapesList[target].g, shapesList[target].b
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(rectData) * target, sizeof(rectData), rectData);
        break;
    }
    }
}
