#include <GL/glew.h>              // GLEW 라이브러리: OpenGL 확장 기능을 쉽게 사용할 수 있게 해줌
#include <GL/freeglut.h>          // FreeGLUT 라이브러리: OpenGL 유틸리티 툴킷, 창 관리 및 입력 처리
#include <GL/freeglut_ext.h>      // FreeGLUT 확장 기능

#include <glm/glm.hpp>            // GLM 라이브러리: 수학 함수와 벡터/매트릭스 연산 지원
#include <glm/ext.hpp>            // GLM 확장 기능
#include <glm/gtc/matrix_transform.hpp> // GLM 매트릭스 변환 기능

#include <iostream>               // 표준 입출력 스트림
#include <random>                 // 난수 생성
#include <fstream>                // 파일 입출력 스트림
#include <iterator>               // 반복자
#include "file_utils.h"           // 사용자 정의 파일 유틸리티 헤더

using namespace std;               // 표준 네임스페이스 사용

// 윈도우 초기 위치 및 크기 설정
const int WIN_X = 10, WIN_Y = 10;
const int WIN_W = 800, WIN_H = 800;

// 배경 색상 (흰색)
const glm::vec3 background_rgb = glm::vec3(1.0f, 1.0f, 1.0f);

// 전역 변수들
bool isCulling = true;             // 백페이스 컬링 활성화 여부
bool isFill = true;                // 폴리곤 채우기 모드 여부
double xMove = 0.0, yMove = 0.0, zMove = 0.0; // 이동 변수
float xRotateAni = 0.0f;           // X축 회전 애니메이션
float yRotateAni = 0.0f;           // Y축 회전 애니메이션
int rotateKey = 0;                  // 회전 키 입력 상태

GLfloat mx = 0.0f;                  // 마우스 X 좌표
GLfloat my = 0.0f;                  // 마우스 Y 좌표

int framebufferWidth, framebufferHeight; // 프레임버퍼 크기
GLuint triangleVertexArrayObject;        // VAO ID
GLuint shaderProgramID;                   // 셰이더 프로그램 ID
GLuint trianglePositionVertexBufferObjectID, triangleColorVertexBufferObjectID; // VBO IDs
GLuint trianglePositionElementBufferObject; // EBO ID

// 인덱스 벡터
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;

// 정점, UV, 노멀 벡터
std::vector< glm::vec3 > vertices;
std::vector< glm::vec2 > uvs;
std::vector< glm::vec3 > normals;

bool isCube = true;                 // 큐브 오브젝트 로드 여부

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
    while (in) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") { // 정점 정보
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (lineHeader == "vt") { // UV 정보
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            uvs.push_back(uv);
        }
        else if (lineHeader == "vn") { // 노멀 정보
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (lineHeader == "f") { // 면 정보
            char a;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

            for (int i = 0; i < 3; i++)
            {
                // "vertex/uv/normal" 형식으로 인덱스 읽기
                in >> vertexIndex[i] >> a >> uvIndex[i] >> a >> normalIndex[i];
                vertexIndices.push_back(vertexIndex[i] - 1); // 인덱스는 1부터 시작하므로 0으로 조정
                uvIndices.push_back(uvIndex[i] - 1);
                normalIndices.push_back(normalIndex[i] - 1);
            }
        }
    }

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

    return true;
}

// VAO를 설정하는 함수
bool Set_VAO() {
    // 삼각형을 구성하는 정점 데이터 - 위치와 색상
    // 현재는 Cube와 Pyramid 중 하나를 로드
    isCube ? Load_Object("cube.obj") : Load_Object("diamond.obj");

    // 색상 배열 (예시 값, 실제로는 오브젝트에 따라 다름)
    float color[] = {
       0.5f, 0.0f, 0.5f, // 정점 4
       0.0f, 0.0f, 1.0f, // 정점 0
       0.0f, 0.0f, 0.0f, // 정점 3

       0.5f, 0.0f, 0.5f, // 정점 4
       0.0f, 0.0f, 0.0f, // 정점 3
       1.0f, 0.0f, 0.0f, // 정점 7

       0.0f, 1.0f, 0.0f, // 정점 2
       0.5f, 0.5f, 0.0f, // 정점 6
       1.0f, 0.0f, 0.0f, // 정점 7

       0.0f, 1.0f, 0.0f, // 정점 2
       1.0f, 0.0f, 0.0f, // 정점 7
       0.0f, 0.0f, 0.0f, // 정점 3

       0.0f, 0.5f, 0.5f, // 정점 1
       1.0f, 1.0f, 1.0f, // 정점 5
       0.0f, 1.0f, 0.0f, // 정점 2

       1.0f, 1.0f, 1.0f, // 정점 5
       0.5f, 0.5f, 0.0f, // 정점 6
       0.0f, 1.0f, 0.0f, // 정점 2

       0.0f, 0.0f, 1.0f, // 정점 0
       0.5f, 0.0f, 0.5f, // 정점 4
       0.0f, 0.5f, 0.5f, // 정점 1

       0.5f, 0.0f, 0.5f, // 정점 4
       1.0f, 1.0f, 1.0f, // 정점 5
       0.0f, 0.5f, 0.5f, // 정점 1

       0.5f, 0.0f, 0.5f, // 정점 4
       1.0f, 0.0f, 0.0f, // 정점 7
       1.0f, 1.0f, 1.0f, // 정점 5

       1.0f, 0.0f, 0.0f, // 정점 7
       0.5f, 0.5f, 0.0f, // 정점 6
       1.0f, 1.0f, 1.0f, // 정점 5

       0.0f, 0.0f, 1.0f, // 정점 0
       0.0f, 0.5f, 0.5f, // 정점 1
       0.0f, 1.0f, 0.0f, // 정점 2

       0.0f, 0.0f, 1.0f, // 정점 0
       0.0f, 1.0f, 0.0f, // 정점 2
       0.0f, 0.0f, 0.0f, // 정점 3

       0.0f, 0.0f, 0.0f, // 추가 정점 (사용되지 않음)
       0.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f
    };

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
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // 정점 속성 활성화
    glEnableVertexAttribArray(positionAttribute);

    // 색상 VBO 생성 및 데이터 복사
    glGenBuffers(1, &triangleColorVertexBufferObjectID); // 색상 VBO 생성
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorVertexBufferObjectID); // 색상 VBO 바인드
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW); // 색상 데이터 전송

    // 셰이더에서 색상 속성의 위치 가져오기
    GLint colorAttribute = glGetAttribLocation(shaderProgramID, "colorAttribute");
    if (colorAttribute == -1) {
        cerr << "color 속성 설정 실패" << endl;
        return false;
    }
    // 색상 VBO 바인드 (다시)
    glBindBuffer(GL_ARRAY_BUFFER, triangleColorVertexBufferObjectID);
    // 색상 속성 포인터 설정 (3개의 float)
    glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // 색상 속성 활성화
    glEnableVertexAttribArray(colorAttribute);

    // VAO 바인드 해제
    glBindVertexArray(0);

    return true;
}

//--- 콜백 함수: 그리기 콜백 함수 
GLvoid drawScene()
{
    // 배경색 설정
    glClearColor(background_rgb.x, background_rgb.y, background_rgb.z, 1.0f);
    // 배경색으로 클리어
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 컬링 및 폴리곤 모드 설정
    isCulling ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); // 깊이 테스트 활성화/비활성화
    isCulling ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE); // 백페이스 컬링 활성화/비활성화
    isFill ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 폴리곤 채우기 모드 설정

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgramID);

    // 모델 변환 행렬 생성
    glm::mat4 TR = glm::mat4(1.0f); // 단위 행렬로 초기화
    TR = glm::translate(TR, glm::vec3(xMove, yMove, zMove)); // 이동 변환
    TR = glm::rotate(TR, glm::radians(30.0f + xRotateAni), glm::vec3(1.0, 0.0, 0.0)); // X축 회전
    TR = glm::rotate(TR, glm::radians(-30.0f + yRotateAni), glm::vec3(0.0, 1.0, 0.0)); // Y축 회전

    // 셰이더의 "transform" 유니폼 변수 위치 가져오기
    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "transform");
    // 변환 행렬을 유니폼 변수에 전달
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    // VAO 바인드
    glBindVertexArray(triangleVertexArrayObject);

    // 인덱스 배열을 사용하여 삼각형 그리기
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    // 라인 그리기 (추가적인 선 그리기, 인덱스 36부터 40개)
    glDrawArrays(GL_LINES, 36, 40);
    // 대체로 삼각형 그리기 (주석 처리됨)
    // glDrawArrays(GL_TRIANGLES, 0, 12);

    // 버퍼 스왑하여 화면에 출력
    glutSwapBuffers();
}

//--- 콜백 함수: 다시 그리기 콜백 함수 
GLvoid Reshape(int w, int h)
{
    // 뷰포트 크기 설정
    glViewport(0, 0, w, h);
}

// 타이머 콜백 함수: 애니메이션 처리
GLvoid TimerFunction1(int value)
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
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'c':
        isCube = true; // 큐브 로드
        Set_VAO();     // VAO 재설정
        break;
    case 'p':
        isCube = false; // 피라미드 로드
        Set_VAO();      // VAO 재설정
        break;
    case 'h':
        isCulling = 1 - isCulling; // 컬링 상태 토글
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
    case 'j':
        xMove -= 0.1; // X축 음의 방향으로 이동
        break;
    case 'l':
        xMove += 0.1; // X축 양의 방향으로 이동
        break;
    case 'i':
        yMove += 0.1; // Y축 양의 방향으로 이동
        break;
    case 'k':
        yMove -= 0.1; // Y축 음의 방향으로 이동
        break;
    case 's':
        rotateKey = 0; // 회전 중지
        xMove = 0.0, yMove = 0.0f, zMove = 0.0f; // 이동 변수 초기화
        xRotateAni = 0.0f; // X축 회전 초기화
        yRotateAni = 0.0f; // Y축 회전 초기화
        break;
    }
    // 화면 재출력 요청
    glutPostRedisplay();
}

// 마우스 입력 콜백 함수
void Mouse(int button, int state, int x, int y)
{
    GLfloat half_w = WIN_W / 2.0f;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 마우스 클릭 시 정규화된 좌표 계산
        mx = (x - half_w) / half_w;
        my = (half_w - y) / half_w;
    }
    Set_VAO();           // VAO 재설정 (필요 시)
    glutPostRedisplay(); // 화면 재출력 요청
}



// 메인 함수
int main(int argc, char** argv)
{
    // GLUT 초기화
    glutInit(&argc, argv);                            // GLUT 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);     // 디스플레이 모드 설정: 더블 버퍼링 및 RGBA 색상
    glutInitWindowPosition(WIN_X, WIN_Y);             // 윈도우 초기 위치 설정
    glutInitWindowSize(WIN_W, WIN_H);                 // 윈도우 초기 크기 설정
    glutCreateWindow("Example1");                      // 윈도우 생성 및 제목 설정

    // GLEW 초기화
    glewExperimental = GL_TRUE;                        // 최신 OpenGL 기능 사용 설정
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

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);        // 그리기 콜백 함수
    glutReshapeFunc(Reshape);          // 창 크기 변경 시 호출되는 콜백 함수
    glutKeyboardFunc(Keyboard);        // 키보드 입력 콜백 함수
    glutMouseFunc(Mouse);              // 마우스 입력 콜백 함수

    // 이벤트 루프 시작
    glutMainLoop();                     // GLUT 이벤트 처리 루프

    return 0;
}
