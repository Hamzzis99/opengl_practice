// main.cpp
#include <iostream>
#include <vector>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "file_utils.h"
#include "obj_loader.h"

// 셰이더 파일 경로
#define VERTEX_SHADER_PATH "vertex.glsl"
#define FRAGMENT_SHADER_PATH "fragment.glsl"

// 콜백 함수 선언
void drawScene();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

// 셰이더 및 버퍼 객체 ID
GLuint shaderProgram;
GLuint VAO[2], VBO[2], EBO[2]; // 두 개의 모델을 위한 배열

// 축을 위한 VAO 및 VBO
GLuint axesVAO = 0;
GLuint axesVBO[2]; // [0]: 위치, [1]: 색상

// 모델 데이터 구조체
Model cubeModel;
Model tetrahedronModel;

// 전역 변수
int selectedFaceIndices[2] = { -1, -1 }; // 선택된 면의 인덱스
int selectedModel = 0;                   // 선택된 모델 (0: 큐브, 1: 사면체)
bool drawAllFaces = false;               // 모든 면을 그릴지 여부
int windowWidth = 800, windowHeight = 600;

// 난수 생성기
std::random_device rd;
std::mt19937 gen(rd());

// 함수 선언
bool setupShaders();
void setupBuffers();
void setupAxes();
void selectRandomFaces(int model);
void loadModels();

void drawAxes();

// 메인 함수
int main(int argc, char** argv) {
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Model Face Selector");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

    // 셰이더 설정
    if (!setupShaders()) {
        std::cerr << "셰이더 설정 실패" << std::endl;
        return -1;
    }

    // 모델 로드
    loadModels();

    // 버퍼 설정
    setupBuffers();

    // 축 설정
    setupAxes();

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    // 이벤트 루프 시작
    glutMainLoop();

    return 0;
}

// 모델 로드 함수
void loadModels() {
    // 큐브 모델 로드
    if (!Load_Object("cube.obj", cubeModel)) {
        std::cerr << "큐브 모델 로드 실패" << std::endl;
        exit(1);
    }

    // 사면체 모델 로드
    if (!Load_Object("Tetrahedron.obj", tetrahedronModel)) {
        std::cerr << "사면체 모델 로드 실패" << std::endl;
        exit(1);
    }
}

// 셰이더 설정 함수
bool setupShaders() {
    // 버텍스 셰이더 로드 및 컴파일
    char* vertexShaderSource = File_To_Buf(VERTEX_SHADER_PATH);
    if (!vertexShaderSource) {
        std::cerr << "버텍스 셰이더 소스 로드 실패" << std::endl;
        return false;
    }
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 컴파일 오류 체크
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "버텍스 셰이더 컴파일 오류: " << infoLog << std::endl;
        delete[] vertexShaderSource;
        return false;
    }

    // 프래그먼트 셰이더 로드 및 컴파일
    char* fragmentShaderSource = File_To_Buf(FRAGMENT_SHADER_PATH);
    if (!fragmentShaderSource) {
        std::cerr << "프래그먼트 셰이더 소스 로드 실패" << std::endl;
        delete[] vertexShaderSource;
        return false;
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 컴파일 오류 체크
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "프래그먼트 셰이더 컴파일 오류: " << infoLog << std::endl;
        delete[] vertexShaderSource;
        delete[] fragmentShaderSource;
        return false;
    }

    // 셰이더 프로그램 생성 및 링크
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // 셰이더 프로그램 링크
    glLinkProgram(shaderProgram);

    // 링크 오류 체크
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "셰이더 프로그램 링크 오류: " << infoLog << std::endl;
        delete[] vertexShaderSource;
        delete[] fragmentShaderSource;
        return false;
    }

    // 셰이더 객체 삭제
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 메모리 해제
    delete[] vertexShaderSource;
    delete[] fragmentShaderSource;

    return true;
}

// 버퍼 설정 함수
void setupBuffers() {
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    // 큐브 모델 버퍼 설정
    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cubeModel.vertices.size() * sizeof(Vertex), &cubeModel.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeModel.indices.size() * sizeof(unsigned int), &cubeModel.indices[0], GL_STATIC_DRAW);

    // 위치 속성
    GLint posAttrib = glGetAttribLocation(shaderProgram, "positionAttribute");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 색상 속성 (빨간색)
    std::vector<glm::vec3> colors(cubeModel.vertices.size(), glm::vec3(1.0f, 0.0f, 0.0f));
    GLuint colorVBO;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

    GLint colorAttrib = glGetAttribLocation(shaderProgram, "colorAttribute");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);

    // 사면체 모델 버퍼 설정
    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, tetrahedronModel.vertices.size() * sizeof(Vertex), &tetrahedronModel.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tetrahedronModel.indices.size() * sizeof(unsigned int), &tetrahedronModel.indices[0], GL_STATIC_DRAW);

    // 위치 속성
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 색상 속성 (빨간색)
    colors = std::vector<glm::vec3>(tetrahedronModel.vertices.size(), glm::vec3(1.0f, 0.0f, 0.0f));
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

// 축 설정 함수
void setupAxes() {
    // 축의 정점 데이터 (윈도우 크기에 맞게 설정)
    GLfloat axesVertices[] = {
        // x축 (녹색)
        0.0f, (float)windowHeight / 2.0f, 0.0f,
        (float)windowWidth, (float)windowHeight / 2.0f, 0.0f,
        // y축 (빨간색)
        (float)windowWidth / 2.0f, 0.0f, 0.0f,
        (float)windowWidth / 2.0f, (float)windowHeight, 0.0f
    };

    // 축의 색상 데이터
    GLfloat axesColors[] = {
        // x축 색상 (녹색)
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // y축 색상 (빨간색)
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };

    // VAO 생성 및 바인딩
    glGenVertexArrays(1, &axesVAO);
    glBindVertexArray(axesVAO);

    // VBO 생성
    glGenBuffers(2, axesVBO);

    // 위치 VBO 설정
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);
    GLint positionAttribute = glGetAttribLocation(shaderProgram, "positionAttribute");
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // 색상 VBO 설정
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesColors), axesColors, GL_STATIC_DRAW);
    GLint colorAttribute = glGetAttribLocation(shaderProgram, "colorAttribute");
    glEnableVertexAttribArray(colorAttribute);
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // VAO 바인딩 해제
    glBindVertexArray(0);
}

// 랜덤 면 선택 함수
void selectRandomFaces(int model) {
    std::uniform_int_distribution<> dist;
    if (model == 0) {
        // 큐브 모델
        dist = std::uniform_int_distribution<>(0, 5);
    }
    else {
        // 사면체 모델
        dist = std::uniform_int_distribution<>(0, 3);
    }

    selectedFaceIndices[0] = dist(gen);
    selectedFaceIndices[1] = dist(gen);
    while (selectedFaceIndices[0] == selectedFaceIndices[1]) {
        selectedFaceIndices[1] = dist(gen);
    }
    drawAllFaces = false;
}

// 그리기 콜백 함수
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // 깊이 테스트 설정
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 백페이스 컬링 비활성화
    glDisable(GL_CULL_FACE);

    // 변환 행렬 설정
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // 모델별로 회전 각도 조정
    if (selectedModel == 0) {
        // 큐브 모델
        modelMatrix = glm::rotate(modelMatrix,
            glm::radians(30.0f),
            glm::vec3(1.0f, 1.0f, 1.0f));
    }
    else {
        // 사면체 모델 - x축과 y축으로 회전 조정
        modelMatrix = glm::rotate(modelMatrix,
            glm::radians(20.0f),
            glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix,
            glm::radians(-15.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 카메라 위치 조정
    glm::vec3 cameraPos(1.5f, 1.5f, 3.0f);
    if (selectedModel == 1) {
        // 사면체 모델일 때 카메라 위치 조정
        cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    }

    glm::mat4 view = glm::lookAt(
        cameraPos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // 투영 행렬 조정
    float fov = 45.0f;
    if (selectedModel == 1) {
        // 사면체 모델일 때 시야각 조정
        fov = 60.0f;
    }

    glm::mat4 projection = glm::perspective(
        glm::radians(fov),
        (float)windowWidth / (float)windowHeight,
        0.1f,
        100.0f
    );

    glm::mat4 mvp = projection * view * modelMatrix;

    // 셰이더에 행렬 전달
    GLuint mvpLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // VAO 바인딩
    glBindVertexArray(VAO[selectedModel]);

    // 면 그리기
    if (drawAllFaces) {
        // 모든 면 그리기
        if (selectedModel == 0) {
            glDrawElements(GL_TRIANGLES, cubeModel.indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawElements(GL_TRIANGLES, tetrahedronModel.indices.size(), GL_UNSIGNED_INT, 0);
        }
    }
    else if (selectedFaceIndices[0] != -1) {
        // 선택된 면 그리기
        if (selectedModel == 0) {
            // 큐브 모델
            for (int i = 0; i < 2; ++i) {
                if (selectedFaceIndices[i] != -1) {
                    int faceIndex = selectedFaceIndices[i];
                    int startIndex = faceIndex * 6; // 각 면당 6개의 인덱스
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }
            }
        }
        else {
            // 사면체 모델
            for (int i = 0; i < 2; ++i) {
                if (selectedFaceIndices[i] != -1) {
                    int faceIndex = selectedFaceIndices[i];
                    int startIndex = faceIndex * 3; // 각 면당 3개의 인덱스
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }
            }
        }
    }

    // VAO 바인딩 해제
    glBindVertexArray(0);

    // === 축 그리기 ===
    drawAxes();

    glutSwapBuffers();
}

// 축 그리기 함수
void drawAxes() {
    // 깊이 테스트 및 컬링 비활성화 (축이 항상 보이도록)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgram);

    // 변환 행렬 설정 (단위 행렬 사용)
    glm::mat4 orthoProjection = glm::ortho(
        0.0f, (float)windowWidth,
        0.0f, (float)windowHeight,
        -1.0f, 1.0f
    );
    glm::mat4 axesModelMatrix = glm::mat4(1.0f);
    glm::mat4 mvp = orthoProjection * axesModelMatrix;

    // 셰이더에 행렬 전달
    GLuint mvpLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    // VAO 바인딩 및 축 그리기
    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, 4); // 총 4개의 정점 (2개의 선)
    glBindVertexArray(0);

    // 깊이 테스트 및 컬링 복원
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

// 창 크기 변경 콜백 함수
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    windowWidth = w;
    windowHeight = h;

    // 윈도우 크기가 변경되면 축의 정점 좌표를 업데이트
    setupAxes();
}

// 키보드 입력 콜백 함수
void keyboard(unsigned char key, int x, int y) {
    if (key >= '1' && key <= '6') {
        // 큐브 모델의 특정 면 선택
        selectedModel = 0;
        selectedFaceIndices[0] = key - '1';
        selectedFaceIndices[1] = -1;
        drawAllFaces = false;
    }
    else if (key >= '7' && key <= '9') {
        // 사면체 모델의 특정 면 선택
        selectedModel = 1;
        selectedFaceIndices[0] = key - '7';
        selectedFaceIndices[1] = -1;
        drawAllFaces = false;
    }
    else if (key == '0') {
        // 사면체 모델의 네 번째 면 선택
        selectedModel = 1;
        selectedFaceIndices[0] = 3;
        selectedFaceIndices[1] = -1;
        drawAllFaces = false;
    }
    else if (key == 'a') {
        // 모든 면 그리기 (현재 선택된 모델)
        drawAllFaces = true;
        selectedFaceIndices[0] = -1;
        selectedFaceIndices[1] = -1;
    }
    else if (key == 's') {
        // 사면체의 모든 면 그리기
        selectedModel = 1;
        drawAllFaces = true;
        selectedFaceIndices[0] = -1;
        selectedFaceIndices[1] = -1;
    }
    else if (key == 't') {
        // 사면체의 두 개의 랜덤한 면 그리기
        selectedModel = 1;
        selectRandomFaces(1);
    }
    else if (key == 'c') {
        // 큐브의 두 개의 랜덤한 면 그리기
        selectedModel = 0;
        selectRandomFaces(0);
    }
    else if (key == 'q' || key == 27) { // 'q' 또는 ESC 키로 종료
        exit(0);
    }

    glutPostRedisplay();
}
