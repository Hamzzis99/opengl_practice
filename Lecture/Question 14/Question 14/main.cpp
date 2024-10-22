#include <GL/glew.h>              // GLEW 라이브러리
#include <GL/freeglut.h>          // FreeGLUT 라이브러리

#include <glm/glm.hpp>            // GLM 라이브러리
#include <glm/ext.hpp>            // GLM 확장 기능
#include <glm/gtc/matrix_transform.hpp> // GLM 매트릭스 변환 기능
#include <glm/gtc/type_ptr.hpp>   // GLM 포인터 기능

#include <iostream>               // 표준 입출력 스트림
#include <fstream>                // 파일 입출력 스트림
#include <vector>                 // 벡터 컨테이너
#include <string>                 // 문자열 스트림
#include "file_utils.h"           // 사용자 정의 파일 유틸리티 헤더

using namespace std;

// 윈도우 초기 위치 및 크기 설정
const int WIN_X = 10, WIN_Y = 10;
const int WIN_W = 800, WIN_H = 600; // const 유지

// 현재 윈도우 크기를 저장할 변수
int currentWidth = WIN_W;
int currentHeight = WIN_H;

// 배경 색상 (흰색)
const glm::vec3 background_rgb = glm::vec3(1.0f, 1.0f, 1.0f);

// 전역 변수들
bool isCulling = true;             // 백페이스 컬링 활성화 여부
bool isFill = true;                // 폴리곤 채우기 모드 여부
double xMove = 0.0, yMove = 0.0, zMove = 0.0; // 이동 변수
float xRotateAni = 0.0f;           // X축 회전 애니메이션
float yRotateAni = 0.0f;           // Y축 회전 애니메이션
int rotateKey = 0;                 // 회전 키 입력 상태

GLuint triangleVertexArrayObject = 0;    // VAO ID
GLuint shaderProgramID;                  // 셰이더 프로그램 ID
GLuint trianglePositionVertexBufferObjectID = 0, triangleColorVertexBufferObjectID = 0; // VBO IDs
GLuint trianglePositionElementBufferObject = 0; // EBO ID

// 축을 위한 변수
GLuint axesVAO = 0;
GLuint axesVBO = 0;

// 인덱스 벡터
std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

// 정점, UV, 노멀 벡터
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

bool isCube = true;                 // 큐브 오브젝트 로드 여부

unsigned int numIndices = 0; // 현재 로드된 인덱스 개수

// 오브젝트 파일을 로드하는 함수
bool Load_Object(const char* path) {
    // 기존 데이터 초기화
    vertexIndices.clear();
    uvIndices.clear();
    normalIndices.clear();
    vertices.clear();
    uvs.clear();
    normals.clear();

    // 파일 열기
    ifstream in(path);
    if (!in) {
        cerr << path << " 파일을 찾을 수 없습니다." << endl;
        exit(1);
    }

    // 파일 끝까지 읽기
    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") { // 정점 정보
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (lineHeader == "f") { // 면 정보
            string vertexData[3];
            in >> vertexData[0] >> vertexData[1] >> vertexData[2];

            for (int i = 0; i < 3; i++) {
                string v = vertexData[i];
                size_t pos1 = v.find('/');
                if (pos1 == string::npos) {
                    // "v" 형식
                    unsigned int vertexIndex = stoi(v);
                    vertexIndices.push_back(vertexIndex - 1);
                }
                else {
                    // "v/vt/vn" 또는 "v//vn" 형식
                    size_t pos2 = v.find('/', pos1 + 1);
                    unsigned int vertexIndex = stoi(v.substr(0, pos1));
                    vertexIndices.push_back(vertexIndex - 1);

                    if (pos2 != string::npos && pos2 > pos1 + 1) {
                        // "v/vt/vn" 형식
                        unsigned int uvIndex = stoi(v.substr(pos1 + 1, pos2 - pos1 - 1));
                        unsigned int normalIndex = stoi(v.substr(pos2 + 1));
                        uvIndices.push_back(uvIndex - 1);
                        normalIndices.push_back(normalIndex - 1);
                    }
                    else {
                        // "v//vn" 형식
                        if (pos2 != string::npos) {
                            unsigned int normalIndex = stoi(v.substr(pos2 + 1));
                            normalIndices.push_back(normalIndex - 1);
                        }
                    }
                }
            }
        }
        else {
            // 그 외의 경우는 무시 (예: 주석, 공백)
            std::getline(in, lineHeader);
        }
    }

    numIndices = vertexIndices.size(); // 인덱스 개수 업데이트

    return true;
}

// 셰이더 프로그램을 생성하는 함수
bool Make_Shader_Program() {
    // 셰이더 코드 파일을 버퍼로 불러오기
    const GLchar* vertexShaderSource = File_To_Buf("vertex.glsl");
    const GLchar* fragmentShaderSource = File_To_Buf("fragment.glsl");

    // 버텍스 셰이더 객체 생성
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 셰이더 소스 코드 연결
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // 버텍스 셰이더 컴파일
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];

    // 컴파일 상태 확인
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        cerr << "ERROR: 버텍스 셰이더 컴파일 실패\n" << errorLog << endl;
        return false;
    }

    // 프래그먼트 셰이더 객체 생성
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 셰이더 소스 코드 연결
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // 프래그먼트 셰이더 컴파일
    glCompileShader(fragmentShader);
    // 컴파일 상태 확인
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        cerr << "ERROR: 프래그먼트 셰이더 컴파일 실패\n" << errorLog << endl;
        return false;
    }

    // 셰이더 프로그램 생성
    shaderProgramID = glCreateProgram();
    // 셰이더 프로그램에 버텍스 및 프래그먼트 셰이더 첨부
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    // 셰이더 프로그램 링크
    glLinkProgram(shaderProgramID);

    // 셰이더 객체 삭제 (프로그램에 첨부되었으므로 더 이상 필요 없음)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 링크 상태 확인
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: 셰이더 프로그램 링크 실패\n" << errorLog << endl;
        return false;
    }
    // 셰이더 프로그램 활성화
    glUseProgram(shaderProgramID);

    // 메모리 해제
    delete[] vertexShaderSource;
    delete[] fragmentShaderSource;

    return true;
}

// VAO를 설정하는 함수
bool Set_VAO() {
    // 기존 VAO와 VBO, EBO 삭제 (이미 존재할 경우)
    if (triangleVertexArrayObject) {
        glDeleteVertexArrays(1, &triangleVertexArrayObject);
        triangleVertexArrayObject = 0;
    }
    if (trianglePositionVertexBufferObjectID) {
        glDeleteBuffers(1, &trianglePositionVertexBufferObjectID);
        trianglePositionVertexBufferObjectID = 0;
    }
    if (trianglePositionElementBufferObject) {
        glDeleteBuffers(1, &trianglePositionElementBufferObject);
        trianglePositionElementBufferObject = 0;
    }
    if (triangleColorVertexBufferObjectID) {
        glDeleteBuffers(1, &triangleColorVertexBufferObjectID);
        triangleColorVertexBufferObjectID = 0;
    }

    // 객체 로드
    isCube ? Load_Object("cube.obj") : Load_Object("Tetrahedron.obj");

    // 색상 배열 생성 (모든 정점에 동일한 색상 할당, 예: 빨간색)
    std::vector<float> colors(vertices.size() * 3, 0.0f); // 기본 색상을 검은색으로 초기화
    for (size_t i = 0; i < vertices.size(); ++i) {
        colors[3 * i] = 1.0f;     // Red
        colors[3 * i + 1] = 0.0f; // Green
        colors[3 * i + 2] = 0.0f; // Blue
    }

    // VAO 생성
    glGenVertexArrays(1, &triangleVertexArrayObject);
    // VAO 바인드
    glBindVertexArray(triangleVertexArrayObject);

    // VBO 생성 및 정점 데이터 복사
    glGenBuffers(1, &trianglePositionVertexBufferObjectID); // VBO 생성
    glBindBuffer(GL_ARRAY_BUFFER, trianglePositionVertexBufferObjectID); // VBO 바인드
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW); // 정점 데이터 전송

    // EBO 생성 및 인덱스 데이터 복사
    glGenBuffers(1, &trianglePositionElementBufferObject); // EBO 생성
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, trianglePositionElementBufferObject); // EBO 바인드
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(unsigned int), &vertexIndices[0], GL_STATIC_DRAW); // 인덱스 데이터 전송

    // 셰이더에서 위치 속성의 위치 가져오기
    GLint positionAttribute = glGetAttribLocation(shaderProgramID, "positionAttribute");
    if (positionAttribute == -1) {
        cerr << "position 속성 설정 실패" << endl;
        return false;
    }
    // 정점 속성 포인터 설정 (3개의 float, 정점 데이터의 stride와 offset)
    glEnableVertexAttribArray(positionAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, trianglePositionVertexBufferObjectID);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // 색상 VBO 생성 및 데이터 복사
    glGenBuffers(1, &triangleColorVertexBufferObjectID); // 색상 VBO 생성
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorVertexBufferObjectID); // 색상 VBO 바인드
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW); // 색상 데이터 전송

    // 셰이더에서 색상 속성의 위치 가져오기
    GLint colorAttribute = glGetAttribLocation(shaderProgramID, "colorAttribute");
    if (colorAttribute == -1) {
        cerr << "color 속성 설정 실패" << endl;
        return false;
    }
    // 색상 속성 포인터 설정 (3개의 float)
    glEnableVertexAttribArray(colorAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorVertexBufferObjectID);
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // VAO 바인드 해제
    glBindVertexArray(0);

    return true;
}

// 축 VAO를 설정하는 함수
bool Set_Axes_VAO() {
    // 축의 정점 데이터 (윈도우 좌표계 사용)
    GLfloat axesVertices[] = {
        // x축 (빨간색 선)
        0.0f, (float)currentHeight / 2.0f, 0.0f,
        (float)currentWidth, (float)currentHeight / 2.0f, 0.0f,
        // y축 (초록색 선)
        (float)currentWidth / 2.0f, 0.0f, 0.0f,
        (float)currentWidth / 2.0f, (float)currentHeight, 0.0f
    };

    // 축의 색상 데이터
    GLfloat axesColors[] = {
        // x축 색상 (빨간색)
        1.0f, 0.0f, 0.0f, // 시작점 색상
        1.0f, 0.0f, 0.0f, // 끝점 색상
        // y축 색상 (초록색)
        0.0f, 1.0f, 0.0f, // 시작점 색상
        0.0f, 1.0f, 0.0f  // 끝점 색상
    };

    // VAO 생성 및 바인드
    if (axesVAO) {
        glDeleteVertexArrays(1, &axesVAO);
        axesVAO = 0;
    }
    glGenVertexArrays(1, &axesVAO);
    glBindVertexArray(axesVAO);

    // VBO 생성
    GLuint axesVBOs[2];
    glGenBuffers(2, axesVBOs);

    // 위치 VBO 설정
    glBindBuffer(GL_ARRAY_BUFFER, axesVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);
    GLint positionAttribute = glGetAttribLocation(shaderProgramID, "positionAttribute");
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // 색상 VBO 설정
    glBindBuffer(GL_ARRAY_BUFFER, axesVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesColors), axesColors, GL_STATIC_DRAW);
    GLint colorAttribute = glGetAttribLocation(shaderProgramID, "colorAttribute");
    glEnableVertexAttribArray(colorAttribute);
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // VAO 바인드 해제
    glBindVertexArray(0);

    return true;
}

//--- 콜백 함수: 그리기 콜백 함수 
void drawScene()
{
    // 화면 클리어
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 깊이 테스트 및 페이스 컬링 설정
    isCulling ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    isCulling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    isFill ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgramID);

    // === 도형 그리기 시작 ===
    // 원근 투영 행렬 생성
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)currentWidth / (float)currentHeight,
        0.1f,
        100.0f
    );

    // 뷰 행렬 생성 (카메라 위치 조정)
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f), // 카메라 위치를 z축 방향으로 뒤로 이동
        glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 지점
        glm::vec3(0.0f, 1.0f, 0.0f)  // 업 벡터
    );

    // 모델 행렬 생성 및 변환 적용
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(xMove, yMove, zMove));
    model = glm::rotate(model, glm::radians(30.0f + xRotateAni), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-30.0f + yRotateAni), glm::vec3(0.0f, 1.0f, 0.0f));

    // MVP 행렬 계산
    glm::mat4 mvp = projection * view * model;

    // 셰이더에 MVP 행렬 전달
    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "transform");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    // 도형 그리기
    glBindVertexArray(triangleVertexArrayObject);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // === 도형 그리기 끝 ===

    // === 축 그리기 시작 ===
    // 깊이 테스트 및 페이스 컬링 비활성화
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // 정사영 투영 행렬 생성 (윈도우 크기에 맞춤)
    glm::mat4 orthoProjection = glm::ortho(
        0.0f, (float)currentWidth,   // 좌, 우
        0.0f, (float)currentHeight,   // 하, 상
        -1.0f, 1.0f           // 근, 원
    );

    // 모델 행렬 (축은 변환 없음)
    glm::mat4 axesModel = glm::mat4(1.0f);

    // MVP 행렬 계산
    glm::mat4 axesMVP = orthoProjection * axesModel;

    // 셰이더에 MVP 행렬 전달
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axesMVP));

    // 축 그리기
    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, 4); // 총 4개의 정점 (2개의 선)
    glBindVertexArray(0);

    // 깊이 테스트 및 페이스 컬링 복원
    if (isCulling) {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    // === 축 그리기 끝 ===

    // 버퍼 교환
    glutSwapBuffers();
}

//--- 콜백 함수: 다시 그리기 콜백 함수 
void Reshape(int w, int h)
{
    // 뷰포트 크기 설정
    glViewport(0, 0, w, h);

    // 현재 윈도우 크기 업데이트
    currentWidth = w;
    currentHeight = h;

    // 축 VAO 재설정
    Set_Axes_VAO();
}

// 타이머 콜백 함수: 애니메이션 처리
void TimerFunction1(int value)
{
    glutPostRedisplay(); // 화면 재출력 요청
    if (rotateKey == 1)
        xRotateAni += 0.5f; // X축 양의 방향 회전
    if (rotateKey == 2)
        xRotateAni -= 0.5f; // X축 음의 방향 회전
    if (rotateKey == 3)
        yRotateAni += 0.5f; // Y축 양의 방향 회전
    if (rotateKey == 4)
        yRotateAni -= 0.5f; // Y축 음의 방향 회전
    // 다시 타이머 함수 등록 (100ms 후 호출)
    glutTimerFunc(100, TimerFunction1, 1);
}

// 키보드 입력 콜백 함수
void Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'c':
        isCube = true; // 큐브 로드
        Set_VAO();     // VAO 재설정
        break;
    case 'p':
        isCube = false; // 테트라헤드론 로드
        Set_VAO();      // VAO 재설정
        break;
    case 'h':
        isCulling = !isCulling; // 컬링 상태 토글
        break;
    case 'x':
        rotateKey = 1; // X축 양의 방향 회전
        glutTimerFunc(100, TimerFunction1, 1); // 타이머 등록
        break;
    case 'X':
        rotateKey = 2; // X축 음의 방향 회전
        glutTimerFunc(100, TimerFunction1, 1); // 타이머 등록
        break;
    case 'y':
        rotateKey = 3; // Y축 양의 방향 회전
        glutTimerFunc(100, TimerFunction1, 1); // 타이머 등록
        break;
    case 'Y':
        rotateKey = 4; // Y축 음의 방향 회전
        glutTimerFunc(100, TimerFunction1, 1); // 타이머 등록
        break;
    case 'w':
        isFill = false; // 와이어프레임 모드
        break;
    case 'W':
        isFill = true;  // 채우기 모드
        break;
    case 's':
        rotateKey = 0; // 회전 중지
        xMove = 0.0, yMove = 0.0f, zMove = 0.0f; // 이동 변수 초기화
        xRotateAni = 0.0f; // X축 회전 초기화
        yRotateAni = 0.0f; // Y축 회전 초기화
        break;
    case 27: // ESC 키
        exit(0);
        break;
    }
    // 화면 재출력 요청
    glutPostRedisplay();
}

// 특수 키 입력 콜백 함수
void SpecialKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_LEFT:
        xMove -= 0.1; // X축 음의 방향으로 이동
        break;
    case GLUT_KEY_RIGHT:
        xMove += 0.1; // X축 양의 방향으로 이동
        break;
    case GLUT_KEY_UP:
        yMove += 0.1; // Y축 양의 방향으로 이동
        break;
    case GLUT_KEY_DOWN:
        yMove -= 0.1; // Y축 음의 방향으로 이동
        break;
    }
    // 화면 재출력 요청
    glutPostRedisplay();
}

// 메인 함수
int main(int argc, char** argv)
{
    // GLUT 초기화
    glutInit(&argc, argv);                                            // GLUT 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);        // 디스플레이 모드 설정: 더블 버퍼링 및 RGBA 색상, 깊이 버퍼
    glutInitWindowPosition(WIN_X, WIN_Y);                             // 윈도우 초기 위치 설정
    glutInitWindowSize(WIN_W, WIN_H);                                 // 윈도우 초기 크기 설정
    glutCreateWindow("Example1");                                     // 윈도우 생성 및 제목 설정

    // GLEW 초기화
    glewExperimental = GL_TRUE;                                        // 최신 OpenGL 기능 사용 설정
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";

    // 셰이더 프로그램 생성
    if (!Make_Shader_Program()) {
        cerr << "오류: 셰이더 프로그램 생성 실패" << endl;
        std::exit(EXIT_FAILURE);
    }

    // VAO 설정
    if (!Set_VAO()) {
        cerr << "오류: VAO 생성 실패" << endl;
        std::exit(EXIT_FAILURE);
    }

    // 축 VAO 설정
    if (!Set_Axes_VAO()) {
        cerr << "오류: 축 VAO 생성 실패" << endl;
        std::exit(EXIT_FAILURE);
    }

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);        // 그리기 콜백 함수
    glutReshapeFunc(Reshape);          // 창 크기 변경 시 호출되는 콜백 함수
    glutKeyboardFunc(Keyboard);        // 키보드 입력 콜백 함수
    glutSpecialFunc(SpecialKeys);      // 방향키 콜백

    // 타이머 콜백 함수 등록 (애니메이션 시작)
    glutTimerFunc(100, TimerFunction1, 1);

    // 이벤트 루프 시작
    glutMainLoop();                     // GLUT 이벤트 처리 루프

    return 0;
}
