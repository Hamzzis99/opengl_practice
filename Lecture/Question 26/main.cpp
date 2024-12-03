#define _CRT_SECURE_NO_WARNINGS //--- 프로그램의 가장 앞부분에 선언

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>
#include <string>

using namespace std;

// 난수 생성기 초기화
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> XYdis(-1, 1);
uniform_real_distribution<double> dis(0.0, 1.0);

// 변환 속성을 처리하는 구조체
struct Transform
{
    glm::vec3 position; // 위치
    glm::vec3 rotation; // 회전 (각도)
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0); // 스케일 (기본값 1.0)

    // 위치, 회전, 스케일을 기반으로 변환 행렬을 계산
    glm::mat4 GetTransform()
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 S = glm::scale(glm::mat4(1.0), scale);
        glm::mat4 RX = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
        glm::mat4 RY = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
        return T * RX * RY * RZ * S;
    }
};

// 장면 내 객체의 기본 구조체
struct OBJECT {
    GLuint vao, vbo[3]; // VAO와 VBO들
    Transform worldmatrix; // 월드 변환 매트릭스
    Transform modelmatrix; // 모델 변환 매트릭스
    OBJECT* parent{ nullptr }; // 부모 객체 포인터 (없을 경우 nullptr)

    glm::vec3* vertex; // 정점 데이터
    glm::vec3* face; // 면 데이터
    glm::vec3* vertexdata; // 처리된 정점 데이터
    glm::vec3* normaldata; // 노멀 데이터
    glm::vec3* colordata; // 색상 데이터

    int v_count = 0; // 정점 개수
    int f_count = 0; // 면 개수
    int vertex_count = f_count * 3; // 총 정점 개수 (각 면마다 3개의 정점)

    // OBJ 파일을 읽어 정점과 면 데이터를 채움
    void ReadObj(string fileName)
    {
        ifstream in{ fileName };
        string s;

        // 첫 번째 패스: 정점과 면의 개수를 셈
        while (in >> s)
        {
            if (s == "v") v_count++;
            else if (s == "f") f_count++;
        }
        in.close();
        in.open(fileName);

        vertex_count = f_count * 3;

        // 정점, 면 및 관련 데이터에 대한 메모리 할당
        vertex = new glm::vec3[v_count];
        face = new glm::vec3[f_count];
        vertexdata = new glm::vec3[vertex_count];
        normaldata = new glm::vec3[vertex_count];
        colordata = new glm::vec3[vertex_count];

        int v_incount = 0;
        int f_incount = 0;
        while (in >> s)
        {
            if (s == "v") {
                in >> vertex[v_incount].x >> vertex[v_incount].y >> vertex[v_incount].z;
                v_incount++;
            }
            else if (s == "f") {
                in >> face[f_incount].x >> face[f_incount].y >> face[f_incount].z;
                vertexdata[f_incount * 3 + 0] = vertex[static_cast<int>(face[f_incount].x - 1)];
                vertexdata[f_incount * 3 + 1] = vertex[static_cast<int>(face[f_incount].y - 1)];
                vertexdata[f_incount * 3 + 2] = vertex[static_cast<int>(face[f_incount].z - 1)];
                f_incount++;
            }
        }

        // 각 면에 대한 노멀을 계산
        for (int i = 0; i < f_count; i++)
        {
            glm::vec3 normal = glm::cross(vertexdata[i * 3 + 1] - vertexdata[i * 3 + 0], vertexdata[i * 3 + 2] - vertexdata[i * 3 + 0]);
            normaldata[i * 3 + 0] = normal;
            normaldata[i * 3 + 1] = normal;
            normaldata[i * 3 + 2] = normal;
        }
    }

    // 파생 클래스에서 재정의할 가상 초기화 함수
    virtual void Init() {}

    // 파생 클래스에서 재정의할 가상 그리기 함수
    virtual void draw(int shaderID) {}

    // 부모 객체의 변환을 고려하여 결합된 변환 행렬을 가져옴
    glm::mat4 GetTransform()
    {
        if (parent)
            return parent->GetTransform() * worldmatrix.GetTransform();
        return worldmatrix.GetTransform();
    }

    // 모델 변환 매트릭스를 가져옴
    glm::mat4 GetmodelTransform()
    {
        return modelmatrix.GetTransform();
    }
};

// 큐브 객체를 위한 파생 구조체
struct CUBE : OBJECT
{
    // 버퍼와 속성을 설정하여 큐브를 초기화
    void Init()
    {
        // 각 정점에 무작위 색상 할당
        for (int i = 0; i < vertex_count; i++)
        {
            colordata[i].x = dis(gen);
            colordata[i].y = dis(gen);
            colordata[i].z = dis(gen);
        }

        // 큐브를 중심에 맞추기 위해 정점 이동
        for (int i = 0; i < vertex_count; i++)
        {
            vertexdata[i] -= glm::vec3(0.5, 0.5, 0.5);
        }

        // Vertex Array Object (VAO) 생성 및 바인딩
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Vertex Buffer Objects (VBOs) 생성
        glGenBuffers(3, vbo);

        // 정점 위치 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // 정점 색상 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 정점 노멀 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    // 지정된 셰이더 프로그램을 사용하여 큐브를 그리기
    void draw(int shaderID)
    {
        // 셰이더에서 'model' 유니폼의 위치를 가져옴
        unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
        // 'model' 매트릭스 유니폼 설정
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
        // VAO를 바인딩하고 큐브 그리기
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }

    // 정점 버퍼 데이터를 업데이트
    void update()
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }
};

// 큐브와 미니큐브 객체 인스턴스화
CUBE cube;
CUBE minicube;

// 피라미드 객체를 위한 파생 구조체
struct PYRAMID : OBJECT
{
    // 버퍼와 속성을 설정하여 피라미드를 초기화
    void Init()
    {
        // 각 정점에 무작위 색상 할당
        for (int i = 0; i < vertex_count; i++)
        {
            colordata[i].x = dis(gen);
            colordata[i].y = dis(gen);
            colordata[i].z = dis(gen);
        }

        // 피라미드를 중심에 맞추기 위해 정점 이동
        for (int i = 0; i < vertex_count; i++)
        {
            vertexdata[i] -= glm::vec3(0.5, 0.5, 0.5);
        }

        // Vertex Array Object (VAO) 생성 및 바인딩
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Vertex Buffer Objects (VBOs) 생성
        glGenBuffers(3, vbo);

        // 정점 위치 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // 정점 색상 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 정점 노멀 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    // 지정된 셰이더 프로그램을 사용하여 피라미드를 그리기
    void draw(int shaderID)
    {
        // 셰이더에서 'model' 유니폼의 위치를 가져옴
        unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
        // 'model' 매트릭스 유니폼 설정
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
        // VAO를 바인딩하고 피라미드 그리기
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    }

    // 정점 버퍼 데이터를 업데이트
    void update()
    {
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }
};

// 피라미드 객체 인스턴스화
PYRAMID pyramid;

// 원 객체를 위한 파생 구조체
struct CIRCLE : OBJECT
{
    float r = 3.0; // 원의 반지름

    // 버퍼와 속성을 설정하여 원을 초기화
    void Init()
    {
        vertex_count = 360; // 원을 근사화하기 위한 정점 수
        float angle = 0;
        float x, y, z;
        vertexdata = new glm::vec3[vertex_count];
        normaldata = new glm::vec3[vertex_count];
        colordata = new glm::vec3[vertex_count];

        // 원 주위에 정점 생성
        for (int i = 0; i < vertex_count; ++i)
        {
            angle = (float)i / 360 * 2 * 3.1415926535;
            x = cos(angle) * r;
            y = 0;
            z = sin(angle) * r;
            vertexdata[i] = glm::vec3(x, y, z);
        }

        // 각 정점에 무작위 색상 할당
        for (int i = 0; i < vertex_count; i++)
        {
            colordata[i].x = dis(gen);
            colordata[i].y = dis(gen);
            colordata[i].z = dis(gen);
        }

        // Vertex Array Object (VAO) 생성 및 바인딩
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Vertex Buffer Objects (VBOs) 생성
        glGenBuffers(3, vbo);

        // 정점 위치 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // 정점 색상 버퍼 설정
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 정점 노멀 버퍼 설정 (선 그리기에는 사용되지 않지만 일관성을 위해 초기화)
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    // 지정된 셰이더 프로그램을 사용하여 원을 그리기
    void draw(int shaderID)
    {
        // 셰이더에서 'model' 유니폼의 위치를 가져옴
        unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
        // 'model' 매트릭스 유니폼 설정
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
        // VAO를 바인딩하고 원을 선 루프로 그리기
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
    }

    // 원의 형태를 유지하기 위해 정점 위치 업데이트
    void update()
    {
        float angle = 0;
        float x, y, z;

        // 현재 반지름을 기반으로 정점 위치 재계산
        for (int i = 0; i < vertex_count; ++i)
        {
            angle = (float)i / 360 * 2 * 3.1415926535;
            x = cos(angle) * r;
            y = 0;
            z = sin(angle) * r;
            vertexdata[i] = glm::vec3(x, y, z);
        }

        // 새로운 위치로 정점 버퍼 업데이트
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }
};

// 원 객체 인스턴스화
CIRCLE circle;

// 선 형태와 색상을 위한 배열
GLfloat lineShape[10][2][3] = {};	//--- 선분 위치 값

glm::vec3 colors[12][3] = {};

GLfloat XYZShape[3][2][3] = {
    { {-1.0, 0.0, 0.0}, {1.0, 0.0, 0.0} }, // X축
    { {0.0, -1.0, 0.0}, {0.0, 1.0, 0.0} }, // Y축
    { {0.0, 0.0, -1.0}, {0.0, 0.0, 1.0} }  // Z축
};

GLfloat XYZcolors[6][3] = { //--- 축 색상
    { 1.0, 0.0, 0.0 },	// X축 색상 (빨강)
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },	// Y축 색상 (초록)
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },	// Z축 색상 (파랑)
    { 0.0, 0.0, 1.0 }
};

// 카메라 설정
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 4.0f); //--- 카메라 위치
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향

// 변환 매트릭스
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// 축을 위한 전역 VAO 및 VBO들
GLuint vao, vbo[3];
GLuint TriPosVbo, TriColorVbo;

// 셰이더 소스와 프로그램 ID
GLchar* vertexSource, * fragmentSource; //--- 셰이더 소스 코드를 저장하는 변수
GLuint vertexShader, fragmentShader; //--- 셰이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램 ID

// 창의 크기
int windowWidth = 800;
int windowHeight = 800;

// 마우스 상호작용을 위한 변수
float openGLX, openGLY;
int movingRectangle = -1;

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.15;
bool left_button = false;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;

// 렌더링 및 선택을 위한 플래그
bool start_flag = true; // 한 번만 초기화하기 위한 플래그

bool NSelection = true;	// true: 큐브, false: 피라미드
bool YSelection = false;
bool RSelection = false;
bool ZSelection = false;

bool lightingEnabled = true; //--- 조명 켜기/끄기 변수

// 함수 선언
void make_shaderProgram();
void make_vertexShaders();
void make_fragmentShaders();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void InitBuffer();
char* filetobuf(const char*);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y);
GLvoid Motion(int x, int y);
GLvoid TimerFunction(int value);
GLvoid SpecialKeys(int key, int x, int y);
GLvoid mouseWheel(int button, int dir, int x, int y);

// 창과 콜백 함수를 설정하는 메인 함수
int main(int argc, char** argv) //--- 창 생성 및 콜백 함수 설정
{
    // GLUT 초기화
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 깊이 버퍼 활성화
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Example1");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "GLEW Initialized\n";
    }

    // OBJ 모델 로드
    cube.ReadObj("cube.obj");
    minicube.ReadObj("cube.obj");
    pyramid.ReadObj("pyramid.obj");

    // 셰이더 프로그램 생성
    make_shaderProgram();

    // 객체를 위한 버퍼 초기화
    InitBuffer();

    // 3D 객체의 올바른 렌더링을 위해 깊이 테스트 활성화
    glEnable(GL_DEPTH_TEST);

    // 콜백 함수 설정
    glutTimerFunc(10, TimerFunction, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys); // 특수 키 콜백 함수 등록
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutMouseWheelFunc(mouseWheel);

    // GLUT 메인 루프에 진입
    glutMainLoop();
}

// 장면을 렌더링하는 함수
GLvoid drawScene()
{
    // 셰이더 프로그램 사용
    glUseProgram(shaderProgramID);

    // 색상 및 깊이 버퍼 클리어
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 전역 VAO 바인딩 (축을 위해 사용)
    glBindVertexArray(vao);

    // 축의 색상 버퍼 업데이트
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), XYZcolors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // 셰이더에서 유니폼 위치 가져오기
    int modelLocation = glGetUniformLocation(shaderProgramID, "model");
    int viewLocation = glGetUniformLocation(shaderProgramID, "view");
    int projLocation = glGetUniformLocation(shaderProgramID, "projection");

    // 사용자 상호작용에 기반한 투영 매트릭스 설정
    projection = glm::mat4(1.0f);
    projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
    projection = glm::rotate(projection, (float)glm::radians(x_angle + 30), glm::vec3(1.0, 0.0, 0.0));
    projection = glm::rotate(projection, (float)glm::radians(y_angle - 30), glm::vec3(0.0, 1.0, 0.0));

    // 투영 매트릭스를 셰이더에 전달
    unsigned int cameraLocation = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

    // 원근 투영 매트릭스 설정
    glm::mat4 perspect = glm::mat4(1.0f);
    perspect = glm::perspective(glm::radians(fovy), (float)windowWidth / (float)windowHeight, near_1, far_1);
    perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

    // 미니큐브의 변환을 기반으로 조명 위치 계산
    glm::mat4 lightmatrix = minicube.GetTransform();
    glm::vec3 lightposition = glm::vec3(lightmatrix[3]);

    // 조명 위치를 셰이더에 전달
    unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
    glUniform3f(lightPosLocation, lightposition.x, lightposition.y, lightposition.z);

    // 조명 색상을 셰이더에 전달 (조명 활성화 여부에 따라 다름)
    unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
    if (lightingEnabled)
    {
        glUniform3f(lightColorLocation, 1.0, 1.0, 1.0); // 백색 조명
    }
    else
    {
        glUniform3f(lightColorLocation, 0.0, 0.0, 0.0); // 조명 없음
    }

    // 객체 색상을 셰이더에 전달
    unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
    glUniform3f(objColorLocation, 1.0, 0.5, 0.3);

    // 초기화 플래그 확인
    if (start_flag)
    {
        start_flag = false;
    }

    model = glm::mat4(1.0f);

    // 좌표 축 그리기
    for (int i = 0; i < 3; i++)
    {
        // 각 축의 색상 버퍼 업데이트
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZcolors[i * 2], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 모델 매트릭스 유니폼 설정
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        // 각 축의 정점 위치 버퍼 업데이트
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // 선 두께 설정 및 축 그리기
        glLineWidth(2.0);
        glDrawArrays(GL_LINES, 0, 2);
    }

    // 선택된 객체 (큐브 또는 피라미드) 그리기
    model = glm::mat4(1.0f);
    if (NSelection)
    {
        cube.draw(shaderProgramID);
    }
    else
    {
        pyramid.draw(shaderProgramID);
    }

    // 미니큐브와 원 그리기
    minicube.draw(shaderProgramID);
    circle.draw(shaderProgramID);

    // 렌더링된 이미지를 화면에 표시
    glutSwapBuffers();
}

// 창 크기 조정 시 호출되는 콜백 함수
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

// 객체를 렌더링하기 위한 버퍼 초기화 함수
void InitBuffer()
{
    // 전역 VAO 생성 및 바인딩 (축을 위해 사용)
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(2, vbo); // 축을 위한 2개의 VBO 생성

    // 각 객체 초기화 및 바인딩
    cube.Init();
    minicube.Init();
    minicube.parent = &cube; // 미니큐브의 부모를 큐브로 설정
    pyramid.Init();
    circle.Init();

    // 미니큐브의 초기 위치와 스케일 설정
    minicube.worldmatrix.position.z = -3;
    minicube.modelmatrix.scale = glm::vec3(0.5, 0.5, 0.5);
}

// 셰이더 프로그램을 생성하는 함수 (버텍스 및 프래그먼트 셰이더 컴파일 및 링크)
void make_shaderProgram()
{
    make_vertexShaders();   //--- 버텍스 셰이더 생성
    make_fragmentShaders(); //--- 프래그먼트 셰이더 생성

    // 셰이더 프로그램 생성 및 셰이더들 첨부
    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    // 셰이더 객체 링크 후 삭제
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 셰이더 프로그램 사용
    glUseProgram(shaderProgramID);
}

// 버텍스 셰이더를 컴파일하는 함수
void make_vertexShaders()
{
    vertexSource = filetobuf("vertex.glsl");
    //--- 버텍스 셰이더 객체 생성
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //--- 셰이더 소스 코드를 셰이더 객체에 첨부
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
    //--- 버텍스 셰이더 컴파일
    glCompileShader(vertexShader);

    //--- 컴파일 오류 검사
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: 버텍스 셰이더 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

// 프래그먼트 셰이더를 컴파일하는 함수
void make_fragmentShaders()
{
    fragmentSource = filetobuf("fragment.glsl");
    //--- 프래그먼트 셰이더 객체 생성
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //--- 셰이더 소스 코드를 셰이더 객체에 첨부
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
    //--- 프래그먼트 셰이더 컴파일
    glCompileShader(fragmentShader);

    //--- 컴파일 오류 검사
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: 프래그먼트 셰이더 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

// 파일을 버퍼로 읽어들이는 유틸리티 함수
char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); // 파일을 읽기 모드로 열기
    if (!fptr) // 실패 시 NULL 반환
        return NULL;
    fseek(fptr, 0, SEEK_END); // 파일 끝으로 이동
    length = ftell(fptr); // 현재 파일 포인터 위치 가져오기
    buf = (char*)malloc(length + 1); // 널 종료자를 위한 공간을 포함한 버퍼 할당
    fseek(fptr, 0, SEEK_SET); // 파일 시작으로 되돌아가기
    fread(buf, length, 1, fptr); // 파일 내용을 버퍼에 읽기
    fclose(fptr); // 파일 닫기
    buf[length] = 0; // 버퍼 널 종료자 추가
    return buf; // 버퍼 반환
}

// 키보드 입력을 처리하는 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'z':
        minicube.worldmatrix.position.z -= 0.1;
        circle.r += 0.1;
        circle.update();
        break;
    case 'Z':
        minicube.worldmatrix.position.z += 0.1;
        circle.r -= 0.1;
        circle.update();
        break;
    case 'r':
        RSelection = !RSelection;
        break;
    case 'y':
        YSelection = !YSelection;
        break;
    case 'n':
        NSelection = !NSelection;
        break;
    case 'm': //--- 조명 토글
        lightingEnabled = !lightingEnabled;
        cout << "조명 " << (lightingEnabled ? "켜짐" : "꺼짐") << endl;
        break;
    case 'q':
        glutLeaveMainLoop(); // 애플리케이션 종료
        break;
    }
    glutPostRedisplay(); // 장면 다시 그리기 요청
}

// 특수 키 입력을 처리하는 콜백 함수 (예: 화살표 키)
GLvoid SpecialKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        // 필요 시 위쪽 화살표 키 기능 구현
        break;
    case GLUT_KEY_DOWN:
        // 필요 시 아래쪽 화살표 키 기능 구현
        break;
    case GLUT_KEY_LEFT:
        // 필요 시 왼쪽 화살표 키 기능 구현
        break;
    case GLUT_KEY_RIGHT:
        // 필요 시 오른쪽 화살표 키 기능 구현
        break;
    }
    glutPostRedisplay(); // 장면 다시 그리기 요청
}

// 마우스 이동을 추적하기 위한 변수
int movingMouse = -1;
float beforeX, beforeY;

// 마우스 버튼 이벤트를 처리하는 콜백 함수
GLvoid Mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        ox = x;
        oy = y;
        left_button = true; // 마우스 이동 추적 시작
    }
    else
    {
        ox = 0;
        oy = 0;
        pre_x_angle = x_angle;
        pre_y_angle = y_angle;
        left_button = false; // 마우스 이동 추적 중지
    }
}

// 드래그 시 마우스 움직임을 처리하는 콜백 함수
GLvoid Motion(int x, int y)
{
    if (left_button)
    {
        y_angle = x - ox;
        x_angle = y - oy;
        x_angle += pre_x_angle;
        y_angle += pre_y_angle;

        y_angle /= 2;
        x_angle /= 2;
    }
    glutPostRedisplay(); // 장면 다시 그리기 요청
}

// 마우스 휠 이벤트를 처리하는 콜백 함수 (확대/축소)
GLvoid mouseWheel(int button, int dir, int x, int y)
{
    if (dir > 0)
    {
        wheel_scale += dir * 0.1;
    }
    else if (dir < 0)
    {
        wheel_scale += dir * 0.1;
        if (wheel_scale < 0.1)
        {
            wheel_scale = 0.1; // 스케일이 너무 작아지지 않도록 방지
        }
    }
    glutPostRedisplay(); // 장면 다시 그리기 요청
}

// 창 좌표를 OpenGL 좌표로 변환하는 함수
GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y)
{
    x = (2.0f * mouseX) / windowWidth - 1.0f;
    y = 1.0f - (2.0f * mouseY) / windowHeight;
}

// 애니메이션과 업데이트를 처리하는 타이머 콜백 함수
GLvoid TimerFunction(int value)
{
    switch (value)
    {
    case 1:
        if (NSelection && YSelection)
        {
            cube.modelmatrix.rotation.y += 1;
            //cube.update();
        }
        if (!NSelection && YSelection)
        {
            pyramid.modelmatrix.rotation.y += 1;
            //pyramid.update();
        }
        if (RSelection)
        {
            cube.worldmatrix.rotation.y += 1;
            cube.modelmatrix.rotation.y -= 1;
        }
        break;
    }
    glutPostRedisplay(); // 장면 다시 그리기 요청
    glutTimerFunc(10, TimerFunction, 1); // 타이머 콜백 다시 등록
}
