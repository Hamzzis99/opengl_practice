#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "open_file.h"  
#include "open_obj.h"   
#include <random>
#include <vector>

// 쉐이더 호출과 변수 지정.
#define VERTEX_SHADER_CODE "vertex.glsl"
#define FRAGMENT_SHADER_CODE "fragment.glsl"

// 랜덤 장치 초기화
std::random_device rd;
std::mt19937 g(rd());


// 콜백 함수 선언
void renderScene(void);                                        // 장면을 렌더링하는 함수
void reshapeWindow(int w, int h);                             // 창 크기 변경 시 호출되는 함수
void handleKeyboardInput(unsigned char key, int x, int y);    // 키보드 입력 처리 함수
void handleSpecialKeys(int key, int x, int y);                // 특수 키 입력 처리 함수 (필요 시 구현)
void update(int value);                                        // 타이머 콜백 함수로 애니메이션 업데이트

// 셰이더 관련 변수 선언
GLuint shaderProgram;     // 셰이더 프로그램 ID
GLuint vertexShader;      // 버텍스 셰이더 ID
GLuint fragmentShader;    // 프래그먼트 셰이더 ID
GLuint VAO[4], VBO[4], EBO[4], axisVAO, axisVBO; // 버텍스 배열 객체 등

// 셰이더 및 버퍼 초기화 함수 선언
void createVertexShader();
void createFragmentShader();
GLuint createShaderProgram();
void initializeBuffers();

// 전역 변수들
GLclampf backgroundColorR = 1.0f; // 배경 색상 R
GLclampf backgroundColorG = 1.0f; // 배경 색상 G
GLclampf backgroundColorB = 1.0f; // 배경 색상 B
GLint windowWidth = 800, windowHeight = 600; // 창 크기

Model cubeModel;       // 큐브 모델
Model coneModel;       // 원뿔 모델
Model sphereModel;     // 구 모델
Model cylinderModel;   // 실린더 모델

int rotationFlag = -1; // 회전 제어를 위한 플래그 (-1이면 회전 없음)
int motionFlag = -1;   // 애니메이션 제어를 위한 플래그

// 축을 그리기 위한 버텍스 데이터 (위치와 색상)
// 색상을 검은색으로 변경 (0.0f, 0.0f, 0.0f)
const float axisVertices[] = {
    // 위치               // 색상 (검은색)
    0.0f, 1.0f, 0.0f,     0.0f, 0.0f, 0.0f, // Y축 양의 방향
    0.0f, -1.0f, 0.0f,    0.0f, 0.0f, 0.0f, // Y축 음의 방향

    -1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 0.0f, // X축 음의 방향
    1.0f, 0.0f, 0.0f,     0.0f, 0.0f, 0.0f, // X축 양의 방향

    0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 0.0f, // Z축 양의 방향
    0.0f, 0.0f, -1.0f,    0.0f, 0.0f, 0.0f, // Z축 음의 방향
};

// 회전과 위치 제어를 위한 변수들
float axisRotationX = 0.0f; // x축 회전
float axisRotationY = 0.0f; // y축 회전

float model1PosX = -0.5f; // 첫 번째 모델 위치 x
float model1PosY = 0.0f;  // 첫 번째 모델 위치 y
float model1PosZ = 0.0f;  // 첫 번째 모델 위치 z

float model2PosX = 0.5f;  // 두 번째 모델 위치 x
float model2PosY = 0.0f;  // 두 번째 모델 위치 y
float model2PosZ = 0.0f;  // 두 번째 모델 위치 z

float model1Scale = 1.0f; // 첫 번째 모델 스케일
float model2Scale = 1.0f; // 두 번째 모델 스케일
float axisScale = 1.0f;    // 축 스케일

float viewRotationX = 0.0f; // 뷰 회전 x
float viewRotationY = 0.0f; // 뷰 회전 y
float viewRotationZ = 0.0f; // 뷰 회전 z

bool isModelChanged = false; // 모델 변경 여부

// 애니메이션 함수 선언
void animateSpiralMove();
void animateCrossMove();
void animateZRotation();
void animateScaleRotation();

// 모델 정보 출력 함수 (디버깅 용도)
void print_model_info(const Model& model) {
    std::cout << "Total Vertices: " << model.vertex_count << std::endl;
    for (size_t i = 0; i < model.vertex_count; ++i) {
        std::cout << "Vertex " << i + 1 << ": ("
            << model.vertices[i].x << ", "
            << model.vertices[i].y << ", "
            << model.vertices[i].z << ")" << std::endl;
    }

    std::cout << "Total Faces: " << model.face_count << std::endl;
    for (size_t i = 0; i < model.face_count; ++i) {
        std::cout << "Face " << i + 1 << ": ("
            << model.faces[i].v1 + 1 << ", "
            << model.faces[i].v2 + 1 << ", "
            << model.faces[i].v3 + 1 << ")" << std::endl;
    }
    std::cout << "\n\n\n";
}

// 메인 함수
int main(int argc, char** argv) {
    // GLUT 초기화 및 윈도우 생성
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL Example");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // 셰이더 생성 및 컴파일
    createVertexShader();
    createFragmentShader();
    shaderProgram = createShaderProgram();

    // 콜백 함수 등록
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshapeWindow);
    glutKeyboardFunc(handleKeyboardInput);
    // glutSpecialFunc(handleSpecialKeys); // 특수 키 처리가 필요하면 주석 해제

    // 타이머 함수 설정 (애니메이션 업데이트)
    glutTimerFunc(60, update, 0);

    // 모델 로드
    loadOBJ("cube.obj", &cubeModel);
    loadOBJ("cone.obj", &coneModel);
    loadOBJ("sphere.obj", &sphereModel);
    loadOBJ("cylinder.obj", &cylinderModel);
    printModelVertices(cubeModel);

    // cube.obj에 알록달록한 색상 할당
    for (int i = 0; i < cubeModel.vertex_count; ++i) {
        // 예시: 정점 인덱스에 따라 색상 할당 (빨강, 초록, 파랑 순환)
        switch (i % 3) {
        case 0:
            cubeModel.vertices[i].r = 1.0f; // 빨강
            cubeModel.vertices[i].g = 0.0f;
            cubeModel.vertices[i].b = 0.0f;
            break;
        case 1:
            cubeModel.vertices[i].r = 0.0f;
            cubeModel.vertices[i].g = 1.0f; // 초록
            cubeModel.vertices[i].b = 0.0f;
            break;
        case 2:
            cubeModel.vertices[i].r = 0.0f;
            cubeModel.vertices[i].g = 0.0f;
            cubeModel.vertices[i].b = 1.0f; // 파랑
            break;
        }
    }

    // 깊이 테스트 활성화
    glEnable(GL_DEPTH_TEST);

    // 버퍼 초기화
    initializeBuffers();

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}

// 장면을 렌더링하는 함수
void renderScene(void) {
    // 배경 색상 및 깊이 버퍼 초기화
    glClearColor(backgroundColorR, backgroundColorG, backgroundColorB, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgram);

    // 변환 행렬의 위치를 가져옴
    GLuint transformLoc = glGetUniformLocation(shaderProgram, "trans");

    // 축을 그리기 위한 VAO 바인딩 및 변환 설정
    glBindVertexArray(axisVAO);

    glm::mat4 axisTransform = glm::mat4(1.0f);
    axisTransform = glm::rotate(axisTransform, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
    axisTransform = glm::rotate(axisTransform, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(axisTransform));

    // 축 그리기 (버텍스 수를 6으로 수정)
    glDrawArrays(GL_LINES, 0, 6);

    // 첫 번째 모델에 대한 변환 설정
    glm::mat4 modelTransform1 = glm::mat4(1.0f);
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(viewRotationZ), glm::vec3(0.0, 0.0, 1.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(30.0f + viewRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(-45.0f + viewRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(axisScale, axisScale, axisScale));
    modelTransform1 = glm::translate(modelTransform1, glm::vec3(model1PosX, model1PosY, model1PosZ));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform1 = glm::rotate(modelTransform1, glm::radians(axisRotationY), glm::vec3(0.0, 1.0, 0.0));

    // 첫 번째 모델의 크기를 절반으로 축소
    modelTransform1 = glm::scale(modelTransform1, glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform1));

    // 첫 번째 모델 그리기 (큐브 또는 구)
    if (!isModelChanged) {
        glBindVertexArray(VAO[0]); // 큐브 VAO
        // VAO[0]은 위치와 색상 속성을 모두 포함
        // glDrawElements를 이용하여 큐브 그리기
        glDrawElements(GL_TRIANGLES, cubeModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }
    else {
        glBindVertexArray(VAO[2]); // 구 VAO
        // 구 모델은 색상 속성이 없으므로 기본 색상으로 렌더링됨
        glDrawElements(GL_TRIANGLES, sphereModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }

    // 두 번째 모델에 대한 변환 설정
    glm::mat4 modelTransform2 = glm::mat4(1.0f);
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(viewRotationZ), glm::vec3(0.0, 0.0, 1.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(30.0f + viewRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(-45.0f + viewRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform2 = glm::scale(modelTransform2, glm::vec3(axisScale, axisScale, axisScale));
    modelTransform2 = glm::translate(modelTransform2, glm::vec3(model2PosX, model2PosY, model2PosZ));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(axisRotationX), glm::vec3(1.0, 0.0, 0.0));
    modelTransform2 = glm::rotate(modelTransform2, glm::radians(axisRotationY), glm::vec3(0.0, 1.0, 0.0));
    modelTransform2 = glm::scale(modelTransform2, glm::vec3(model2Scale, model2Scale, model2Scale));

    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform2));

    // 두 번째 모델 그리기 (원뿔 또는 실린더)
    if (!isModelChanged) {
        glBindVertexArray(VAO[1]); // 원뿔 VAO
        glDrawElements(GL_TRIANGLES, coneModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }
    else {
        glBindVertexArray(VAO[3]); // 실린더 VAO
        glDrawElements(GL_TRIANGLES, cylinderModel.face_count * 3, GL_UNSIGNED_INT, 0);
    }

    // 버퍼 교환 (화면 업데이트)
    glutSwapBuffers();
}

// 창 크기 변경 시 호출되는 함수
void reshapeWindow(int w, int h) {
    // 뷰포트 설정
    glViewport(0, 0, w, h);
}

// 키보드 입력 처리 함수
void handleKeyboardInput(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
        rotationFlag = 1; // x축 양의 방향 회전
        break;
    case 'X':
        rotationFlag = 2; // x축 음의 방향 회전
        break;
    case 'y':
        rotationFlag = 3; // y축 양의 방향 회전
        break;
    case 'Y':
        rotationFlag = 4; // y축 음의 방향 회전
        break;
    case 's':
        rotationFlag = -1; // 회전 정지
        axisRotationX = 0.0f;
        axisRotationY = 0.0f;
        viewRotationY = 0.0f;
        motionFlag = -1;
        break;
    case 'r':
    case '3':
        rotationFlag = 5; // 공전 시작
        break;
    case 'R':
        rotationFlag = 6; // 역방향 공전
        break;
    case 'c':
        isModelChanged = !isModelChanged; // 모델 변경
        break;
    case 'e':
        model1Scale += 0.1f; // 첫 번째 모델 스케일 증가
        model2Scale += 0.1f; // 두 번째 모델 스케일 증가
        break;
    case 'E':
        model1Scale -= 0.1f; // 첫 번째 모델 스케일 감소
        model2Scale -= 0.1f; // 두 번째 모델 스케일 감소
        break;
    case 'w':
        axisScale += 0.1f; // 축 스케일 증가
        break;
    case 'W':
        axisScale -= 0.1f; // 축 스케일 감소
        break;
    case 'i':
        model1PosX += 0.1f; // 첫 번째 모델 x 이동
        break;
    case 'I':
        model1PosX -= 0.1f; // 첫 번째 모델 x 이동
        break;
    case 'o':
        model1PosY += 0.1f; // 첫 번째 모델 y 이동
        break;
    case 'O':
        model1PosY -= 0.1f; // 첫 번째 모델 y 이동
        break;
    case 'p':
        model1PosZ += 0.1f; // 첫 번째 모델 z 이동
        break;
    case 'P':
        model1PosZ -= 0.1f; // 첫 번째 모델 z 이동
        break;
    case 'j':
        model2PosX += 0.1f; // 두 번째 모델 x 이동
        break;
    case 'J':
        model2PosX -= 0.1f; // 두 번째 모델 x 이동
        break;
    case 'k':
        model2PosY += 0.1f; // 두 번째 모델 y 이동
        break;
    case 'K':
        model2PosY -= 0.1f; // 두 번째 모델 y 이동
        break;
    case 'l':
        model2PosZ += 0.1f; // 두 번째 모델 z 이동
        break;
    case 'L':
        model2PosZ -= 0.1f; // 두 번째 모델 z 이동
        break;
    case 'q':
        glutLeaveMainLoop(); // 프로그램 종료
        break;
    }

    // 1~5 숫자 키에 따른 애니메이션 시작
    if ('1' <= key && key <= '5') {
        motionFlag = key - '0';
    }

    // 장면 다시 그리기
    glutPostRedisplay();
}

// 셰이더 생성 및 컴파일 함수
void createVertexShader() {
    // 버텍스 셰이더 생성
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource = open_file(VERTEX_SHADER_CODE);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // 컴파일 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: 버텍스 셰이더 컴파일 실패\n" << errorLog << std::endl;
    }
    else {
        std::cout << "버텍스 셰이더 컴파일 성공\n";
    }
}

void createFragmentShader() {
    // 프래그먼트 셰이더 생성
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource = open_file(FRAGMENT_SHADER_CODE);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // 컴파일 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: 프래그먼트 셰이더 컴파일 실패\n" << errorLog << std::endl;
    }
    else {
        std::cout << "프래그먼트 셰이더 컴파일 성공\n";
    }
}

GLuint createShaderProgram() {
    // 셰이더 프로그램 생성 및 셰이더 연결
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // 셰이더 프로그램 링크
    glLinkProgram(program);

    // 링크 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(program, 512, NULL, errorLog);
        std::cerr << "ERROR: 셰이더 프로그램 링크 실패\n" << errorLog << std::endl;
    }
    else {
        std::cout << "셰이더 프로그램 링크 성공\n";
        // 셰이더 삭제 (더 이상 필요 없음)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // 셰이더 프로그램 사용
    glUseProgram(program);

    return program;
}

// 버퍼 초기화 함수
void initializeBuffers() {
    // 첫 번째 모델 (큐브) 버퍼 초기화
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cubeModel.vertex_count * sizeof(Vertex), cubeModel.vertices, GL_STATIC_DRAW);

    // 위치 속성 설정 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성 설정 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeModel.face_count * sizeof(Face), cubeModel.faces, GL_STATIC_DRAW);

    // 두 번째 모델 (원뿔) 버퍼 초기화
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, coneModel.vertex_count * sizeof(Vertex), coneModel.vertices, GL_STATIC_DRAW);

    // 위치 속성 설정 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 원뿔 모델은 색상 속성이 없으므로 색상 속성을 비활성화하거나 기본 색상을 사용
    // 여기서는 색상 속성을 비활성화합니다.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneModel.face_count * sizeof(Face), coneModel.faces, GL_STATIC_DRAW);

    // 세 번째 모델 (구) 버퍼 초기화
    glGenVertexArrays(1, &VAO[2]);
    glBindVertexArray(VAO[2]);

    glGenBuffers(1, &VBO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphereModel.vertex_count * sizeof(Vertex), sphereModel.vertices, GL_STATIC_DRAW);

    // 위치 속성 설정 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 구 모델은 색상 속성이 없으므로 색상 속성을 비활성화하거나 기본 색상을 사용
    // 여기서는 색상 속성을 비활성화합니다.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereModel.face_count * sizeof(Face), sphereModel.faces, GL_STATIC_DRAW);

    // 네 번째 모델 (실린더) 버퍼 초기화
    glGenVertexArrays(1, &VAO[3]);
    glBindVertexArray(VAO[3]);

    glGenBuffers(1, &VBO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, cylinderModel.vertex_count * sizeof(Vertex), cylinderModel.vertices, GL_STATIC_DRAW);

    // 위치 속성 설정 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 실린더 모델은 색상 속성이 없으므로 색상 속성을 비활성화하거나 기본 색상을 사용
    // 여기서는 색상 속성을 비활성화합니다.
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    // glDisableVertexAttribArray(1);

    glGenBuffers(1, &EBO[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderModel.face_count * sizeof(Face), cylinderModel.faces, GL_STATIC_DRAW);

    // 축 그리기를 위한 버퍼 초기화
    glGenVertexArrays(1, &axisVAO);
    glBindVertexArray(axisVAO);

    glGenBuffers(1, &axisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

    // 위치 속성 설정 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);
    // 색상 속성 설정 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// 타이머 콜백 함수 (애니메이션 업데이트)
void update(int value) {
    // 회전 플래그에 따른 회전 업데이트
    switch (rotationFlag) {
    case 1:
        axisRotationX += 1.0f; // x축 양의 방향 회전
        break;
    case 2:
        axisRotationX -= 1.0f; // x축 음의 방향 회전
        break;
    case 3:
        axisRotationY += 1.0f; // y축 양의 방향 회전
        break;
    case 4:
        axisRotationY -= 1.0f; // y축 음의 방향 회전
        break;
    case 5:
        viewRotationY += 1.0f; // 공전 시작
        break;
    case 6:
        viewRotationY -= 1.0f; // 역방향 공전
        break;
    default:
        break;
    }

    // 모션 플래그에 따른 애니메이션 수행
    switch (motionFlag) {
    case 1:
        animateSpiralMove();
        break;
    case 2:
        animateCrossMove();
        break;
    case 4:
        animateZRotation();
        break;
    case 5:
        animateScaleRotation();
        break;
    default:
        break;
    }

    // 장면 다시 그리기
    glutPostRedisplay();

    // 다음 타이머 설정
    glutTimerFunc(60, update, 0);
}

// 애니메이션 함수들
void animateSpiralMove() {
    viewRotationY += 1.0f;          // y축 공전
    model1PosX += 0.001f;           // 첫 번째 모델 x 이동
    model2PosX -= 0.001f;           // 두 번째 모델 x 이동
}

void animateCrossMove() {
    if (model1PosX < 0.5f)
        model1PosX += 0.01f;        // 첫 번째 모델 x 이동
    if (model2PosX > -0.5f)
        model2PosX -= 0.01f;        // 두 번째 모델 x 이동
}

void animateZRotation() {
    viewRotationZ += 1.0f;          // z축 회전
}

void animateScaleRotation() {
    viewRotationZ += 1.0f;          // z축 회전
    model1Scale += 0.005f;          // 첫 번째 모델 스케일 증가
    model2Scale -= 0.005f;          // 두 번째 모델 스케일 감소
}
