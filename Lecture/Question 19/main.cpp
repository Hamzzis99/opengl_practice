#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
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

using namespace std;

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> XYdis(-1, 1);
uniform_real_distribution<double> dis(0, 1);

glm::vec3 cube[12][3]{
    // 앞면
    { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, 0.5, -0.5} },
    { {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5}, {-0.5, -0.5, -0.5} },

    // 뒷면
    { {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, 0.5} },
    { {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5}, {-0.5, -0.5, 0.5} },

    // 상단면
    { {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5}, {0.5, 0.5, 0.5} },
    { {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5}, {-0.5, 0.5, -0.5} },

    // 하단면
    { {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, -0.5, 0.5} },
    { {0.5, -0.5, 0.5}, {-0.5, -0.5, 0.5}, {-0.5, -0.5, -0.5} },

    // 왼쪽면
    { {-0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {-0.5, 0.5, 0.5} },
    { {-0.5, 0.5, 0.5}, {-0.5, -0.5, 0.5}, {-0.5, -0.5, -0.5} },

    // 오른쪽면
    { {0.5, -0.5, -0.5}, {0.5, 0.5, -0.5}, {0.5, 0.5, 0.5} },
    { {0.5, 0.5, 0.5}, {0.5, -0.5, 0.5}, {0.5, -0.5, -0.5} },
};

GLfloat rectShape[4][3] = {
    {-1,1,0},{-1,-1,0},{1,-1,0},{1,1,0}
}; //--- 사각형 위치 값

GLfloat lineShape[10][2][3] = {};	//--- 선분 위치 값

GLfloat dotShape[10][3] = {};	//--- 점 위치 값

GLfloat rectColors[4][3] = { //--- 삼각형 꼭지점 색상
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
    { 1.0, 1.0, 1.0 }
};

glm::vec3 colors[12][3] = {};

GLfloat XYZShape[3][2][3] = {
    {{-1.0,0.0,0.0},{1.0,0.0,0.0}},
    {{0.0,-1.0,0.0},{0.0,1.0,0.0}},
    {{0.0,0.0,-1.0},{0.0,0.0,1.0}} };

GLfloat XYZcolors[6][3] = { //--- 축 색상
    { 1.0, 0.0, 0.0 },	   	{ 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },	   	{ 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 },	   	{ 0.0, 0.0, 1.0 }
};

glm::vec3 cameraPos = glm::vec3(0.0f, -0.5f, 0.5f); //--- 카메라 위치
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f); //--- 카메라 위쪽 방향

glm::vec3 D_P = glm::vec3(0.0f, 0.0f, 0.0f);

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::mat4 tmodel = glm::mat4(1.0f);

GLuint vao, vbo[2];
GLuint TriPosVbo, TriColorVbo;

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램

float B = 0;
int BSelection = 0;

float M = 0;
int MSelection = 0;

float E = 0;
int ESelection = 0; // 기존 코드에서는 ESelection이 없었음, 추가됨

float F = 0;
int FSelection = 0;

float T = 0;
int TSelection = 0;

float Z = 0;

float Y = 0;
int YSelection = 0;

float R = 0;
int RSelection = 0;

float O = 0;
int OSelection = 0;

int windowWidth = 800;
int windowHeight = 600;

float openGLX, openGLY;
int movingRectangle = -1;

bool start = true;

// --- 새로 추가된 변수 ---
bool EReturning = false; // 포신을 원래 위치로 되돌리는 중인지 여부

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

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Example1");

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    //--- 세이더 읽어와서 세이더 프로그램 만들기
    make_shaderProgram(); //--- 세이더 프로그램 만들기
    InitBuffer();
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE); //--- 상태 설정은 필요한 곳에서 하면 된다.
    //glDisable(GL_DEPTH_TEST | GL_CULL_FACE);	//해제

    glutTimerFunc(50, TimerFunction, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys); // 방향키 콜백 함수 등록
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);

    glutMainLoop();
}

GLvoid drawScene()
{
    GLUquadricObj* qobj;

    glUseProgram(shaderProgramID);
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //--- 깊이 버퍼를 클리어한다.

    glBindVertexArray(vao);

    // 색상 바꾸기
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), XYZcolors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    int modelLocation = glGetUniformLocation(shaderProgramID, "model"); //--- 버텍스 세이더에서 모델링 변환 행렬 변수값을 받아온다.
    int viewLocation = glGetUniformLocation(shaderProgramID, "view"); //--- 버텍스 세이더에서 뷰잉 변환 행렬 변수값을 받아온다.
    int projLocation = glGetUniformLocation(shaderProgramID, "projection"); //--- 버텍스 세이더에서 투영 변환 행렬 변수값을 받아온다.

    //투영 변환
    glm::mat4 pTransform = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f); //--- 투영 공간 설정: fovy, aspect, near, far
    projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0)); //--- 공간을 z축 이동
    pTransform = glm::perspective(glm::radians(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 200.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &pTransform[0][0]);
    //카메라 위치만큼 뺀다음 다시 제자리로
    //뷰 변환
    if (YSelection == 1)
    {
        cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    if (RSelection == 1)
    {
        cameraPos = glm::vec3(0.0f, -0.5f, 0.5f);
    }

    if (YSelection == 1)
    {
        cameraDirection -= cameraPos;
        cameraDirection = glm::rotate(glm::mat4(1.0f), glm::radians(Y), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraDirection, 1.0);
        cameraDirection += cameraPos;
    }

    if (RSelection == 1)
    {
        cameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(R), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos, 1.0);
    }
    if (OSelection == 1)
    {
        cameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(O), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos, 1.0);
        OSelection = 0;
    }

    view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    if (start)
    {
        double m = 0.0;
        double cx, cy, cz;
        for (int i = 0; i < 12; i++)
        {
            cx = 0.3 + m;
            cy = 0.0 + m;
            cz = 0.3 + m;

            m += 0.05;

            colors[i][0].x = cx;
            colors[i][1].x = cx;
            colors[i][2].x = cx;

            colors[i][0].y = cy;
            colors[i][1].y = cy;
            colors[i][2].y = cy;

            colors[i][0].z = cz;
            colors[i][1].z = cz;
            colors[i][2].z = cz;
        }
        cout << "b/B: 크레인의 아래 몸체가 x축 방향으로 양/음 방향으로 이동한다. 다시 누르면 멈춘다." << endl;
        cout << "m/M: 크레인의 중앙 몸체가 y축에 대하여 양/음 방향으로 회전한다. 다시 누르면 멈춘다" << endl;
        cout << "f/F: 포신이 y축에 대하여 양/음 방향으로 회전하는데, 두 포신이 서로 반대방향으로 회전한다. 다시 누르면 멈춘다." << endl;
        cout << "e/E: 2개 포신이 조금씩 이동해서 한 개가 된다/다시 제자리로 이동해서 2개가 된다." << endl;
        cout << "t/T: 크레인의 맨 위 2개의 팔이 z축에 대하여 양/음 방향으로 서로 반대방향으로 회전한다. 다시 누르면 멈춘다." << endl;
        cout << "x/X: 모든 움직임 멈추기" << endl;
        cout << "c/C: 모든 움직임이 초기화된다." << endl;
        cout << "Q: 프로그램 종료하기" << endl;
        cout << "위아래 방향키: 카메라가 z축 양/음 방향으로 이동" << endl;
        cout << "좌우 방향키: 카메라가 x축 양/음 방향으로 이동" << endl;
        cout << "y/Y: 카메라 기준 y축에 대하여 회전" << endl;
        cout << "o : 화면의 중심의 y축에 대하여 카메라가 회전 (중점에 대하여 공전)" << endl;
        cout << "r: a 명령어와 같이 화면의 중심의 축에 대하여 카메라가 회전하는 애니메이션을 진행한다/멈춘다." << endl;
        start = false;
    }

    model = glm::mat4(1.0f);

    //축 그리기
    for (int i = 0; i < 3; i++)
    {
        // 색상 바꾸기
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZcolors[i * 2], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // modelTransform 변수에 변환 값 적용하기
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZShape[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glLineWidth(1.0);
        glDrawArrays(GL_LINES, 0, 2);
    }

    //바닥 그리기
    for (int i = 0; i < 4; i++)
    {
        // 색상 바꾸기
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), rectColors[i], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 사각형 그리기
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), rectShape, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_QUADS, 0, 4);
    }
    //-------------------------------------------------
    //s r t p 코드 작성시에는 반대 방향으로.
    model = glm::mat4(1.0f);
    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));
    //이동
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.15f));
    //축소
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.1));

    //위 몸통
    for (int i = 0; i < 12; i++) {
        // modelTransform 변수에 변환 값 적용하기
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &colors[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &cube[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);


        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    //-------------------------------------------------glm::mat4(1.0f)
    //위 왼쪽 대포
    model = glm::mat4(1.0f);

    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    //T회전
    model = glm::rotate(model, glm::radians(T / 3), glm::vec3(1.0f, 0.0f, 0.0f));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));

    //이동
    model = glm::translate(model, glm::vec3(-0.05f, 0.0f, 0.2f));
    //축소
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    qobj = gluNewQuadric(); // 객체 생성하기
    gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일
    gluQuadricNormals(qobj, GLU_SMOOTH); // 생략 가능
    gluQuadricOrientation(qobj, GLU_OUTSIDE); // 생략 가능
    gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 객체 만들기
    //--------------------------------------------------------
    //위 오른쪽 대포
    model = glm::mat4(1.0f);
    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    //T회전
    model = glm::rotate(model, glm::radians(-T / 3), glm::vec3(1.0f, 0.0f, 0.0f));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));

    //이동
    model = glm::translate(model, glm::vec3(0.05f, 0.0f, 0.2f));
    //축소
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    qobj = gluNewQuadric(); // 객체 생성하기
    gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일
    gluQuadricNormals(qobj, GLU_SMOOTH); // 생략 가능
    gluQuadricOrientation(qobj, GLU_OUTSIDE); // 생략 가능
    gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 객체 만들기

    //-------------------------------------------------
    //s r t p 코드 작성시에는 반대 방향으로.
    model = glm::mat4(1.0f);
    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));

    //이동
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

    //축소
    model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

    //아래 몸통
    for (int i = 0; i < 12; i++) {
        // modelTransform 변수에 변환 값 적용하기
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &colors[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &cube[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    //-----------------------------------------------
    //아래 왼쪽 대포
    model = glm::mat4(1.0f);
    //F회전
    model = glm::rotate(model, glm::radians(-F), glm::vec3(0.0f, 0.0f, 1.0f));
    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    // x축으로 90도 회전
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //E이동
    model = glm::translate(model, glm::vec3(-E, 0, 0));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));
    //이동
    model = glm::translate(model, glm::vec3(-0.05f, 0.05f, 0.05f));
    //축소
    model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    qobj = gluNewQuadric(); // 객체 생성하기
    gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일
    gluQuadricNormals(qobj, GLU_SMOOTH); // 생략 가능
    gluQuadricOrientation(qobj, GLU_OUTSIDE); // 생략 가능
    gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 객체 만들기
    //--------------------------------------------------------
    //아래 오른쪽 대포
    model = glm::mat4(1.0f);
    //F회전
    model = glm::rotate(model, glm::radians(F), glm::vec3(0.0f, 0.0f, 1.0f));
    //M회전
    model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
    // x축으로 90도 회전
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //E이동
    model = glm::translate(model, glm::vec3(E, 0, 0));
    //B이동
    model = glm::translate(model, glm::vec3(B, 0, 0));
    //이동
    model = glm::translate(model, glm::vec3(0.05f, 0.05f, 0.05f));
    //축소
    model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    qobj = gluNewQuadric(); // 객체 생성하기
    gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일
    gluQuadricNormals(qobj, GLU_SMOOTH); // 생략 가능
    gluQuadricOrientation(qobj, GLU_OUTSIDE); // 생략 가능
    gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 객체 만들기

    glutSwapBuffers(); //--- 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void InitBuffer()
{
    glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
    glBindVertexArray(vao); //--- VAO를 바인드하기
    glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기
}

void make_shaderProgram()
{
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    //-- shader Program
    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);
    //--- 세이더 삭제하기
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    //--- Shader Program 사용하기
    glUseProgram(shaderProgramID);
}

void make_vertexShaders()
{
    vertexSource = filetobuf("vertex.glsl");
    //--- 버텍스 세이더 객체 만들기
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
    //--- 버텍스 세이더 컴파일하기
    glCompileShader(vertexShader);
    //--- 컴파일이 제대로 되지 않은 경우: 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

void make_fragmentShaders()
{
    fragmentSource = filetobuf("fragment.glsl");
    //--- 프래그먼트 세이더 객체 만들기
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //--- 세이더 코드를 세이더 객체에 넣기
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
    //--- 프래그먼트 세이더 컴파일
    glCompileShader(fragmentShader);
    //--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); // Open file for reading 
    if (!fptr) // Return NULL on failure 
        return NULL;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
    length = ftell(fptr); // Find out how many bytes into the file we are 
    buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
    fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
    fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
    fclose(fptr); // Close the file 
    buf[length] = 0; // Null terminator 
    return buf; // Return the buffer 
}

bool cdx1 = true;
bool bcnt = true, Bcnt = true, mcnt = true, Mcnt = true, Ecnt = true, fcnt = true, Fcnt = true;
bool tcnt = true, Tcnt = true, rcnt = true, ycnt = true;

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case '-':
        cameraPos.z += 0.1;
        break;
    case '=':
        cameraPos.z -= 0.1;
        break;
    case 'w':
        cameraDirection.y += 0.1;
        break;
    case 'a':
        cameraDirection.x -= 0.1;
        break;
    case 's':
        cameraDirection.y -= 0.1;
        break;
    case 'd':
        cameraDirection.x += 0.1;
        break;
    case 'b':
        if (Bcnt)
        {
            BSelection = 1;
            Bcnt = false;
        }
        else
        {
            BSelection = 0;
            Bcnt = true;
        }
        break;
    case 'B':
        if (Bcnt)
        {
            BSelection = 2;
            Bcnt = false;
        }
        else
        {
            BSelection = 0;
            Bcnt = true;
        }
        break;
    case 'm':
        if (mcnt)
        {
            MSelection = 1;
            mcnt = false;
        }
        else
        {
            MSelection = 0;
            mcnt = true;
        }
        break;
    case 'M':
        if (Mcnt)
        {
            MSelection = 2;
            Mcnt = false;
        }
        else
        {
            MSelection = 0;
            Mcnt = true;
        }
        break;
    case 'e':
        if (Ecnt)
        {
            E += 0.01;
            if (E > 0.05)
            {
                Ecnt = false;
            }
        }
        else
        {
            E -= 0.01;
            if (E < -0.05)
            {
                Ecnt = true;
            }
        }
        break;
    case 'E':
        EReturning = true; // 대문자 E가 눌리면 포신을 원래 위치로 되돌림
        break;
    case 'f':
        if (fcnt)
        {
            FSelection = 1;
            fcnt = false;
        }
        else
        {
            FSelection = 0;
            fcnt = true;
        }
        break;
    case 'F':
        if (Fcnt)
        {
            FSelection = 2;
            Fcnt = false;
        }
        else
        {
            FSelection = 0;
            Fcnt = true;
        }
        break;
    case 't':
        if (tcnt)
        {
            TSelection = 1;
            tcnt = false;
        }
        else
        {
            TSelection = 0;
            tcnt = true;
        }
        break;
    case 'T':
        if (Tcnt)
        {
            TSelection = 2;
            Tcnt = false;
        }
        else
        {
            TSelection = 0;
            Tcnt = true;
        }
        break;
    case 'z':
        Z += 1;
        break;
    case 'Z':
        Z -= 1;
        break;
    case 'y':
        if (ycnt)
        {
            YSelection = 1;
            ycnt = false;
        }
        else
        {
            YSelection = 0;
            ycnt = true;
        }
        break;
    case 'o':
        cameraPos = glm::vec3(0.0f, -0.5f, 0.5f);
        OSelection = 1;
        O += 1;
        break;
    case 'r':
        cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
        if (rcnt)
        {
            RSelection = 1;
            rcnt = false;
        }
        else
        {
            RSelection = 0;
            rcnt = true;
        }
        break;
    case 'x':
        BSelection = 0;
        MSelection = 0;
        FSelection = 0;
        TSelection = 0;
        RSelection = 0;
        YSelection = 0;
        break;
    case 'c':
        cameraPos = glm::vec3(0.0f, -0.5f, 0.5f); //--- 카메라 위치
        cameraDirection = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 바라보는 방향
        cameraUp = glm::vec3(0.0f, 0.0f, 1.0f); //--- 카메라 위쪽 방향
        B = 0;
        BSelection = 0;
        M = 0;
        MSelection = 0;
        E = 0;
        ESelection = 0;
        F = 0;
        FSelection = 0;
        T = 0;
        TSelection = 0;
        RSelection = 0;
        YSelection = 0;
        cdx1 = true;
        bcnt = true, Bcnt = true, mcnt = true, Mcnt = true, Ecnt = true, fcnt = true, Fcnt = true;
        tcnt = true, Tcnt = true, rcnt = true, ycnt = true;
        R = 0;
        Y = 0;
        EReturning = false; // EReturning 플래그 초기화
        break;
    case 'q':
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid SpecialKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_UP:
        cameraPos.y += 0.1;
        cameraDirection.y += 0.1;
        break;
    case GLUT_KEY_DOWN:
        cameraPos.y -= 0.1;
        cameraDirection.y -= 0.1;
        break;
    case GLUT_KEY_LEFT:
        cameraPos.x -= 0.1;
        cameraDirection.x -= 0.1;
        break;
    case GLUT_KEY_RIGHT:
        cameraPos.x += 0.1;
        cameraDirection.x += 0.1;
        break;
    }
    glutPostRedisplay(); // 화면 갱신
}

int movingMouse = -1;
float beforeX, beforeY;

GLvoid Mouse(int button, int state, int x, int y)
{
    float openGLX, openGLY;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        WindowToOpenGL(x, y, openGLX, openGLY);

        movingMouse = 0;

        beforeX = openGLX;
        beforeY = openGLY;
    }
    else if (state == GLUT_UP)
    {
        movingMouse = -1;
    }
}

GLvoid Motion(int x, int y)
{
    if (movingMouse >= 0)
    {
        WindowToOpenGL(x, y, openGLX, openGLY);

        float deltaX = openGLX - beforeX;
        float deltaY = openGLY - beforeY;

        //이동

        //마우스값 넣기
        beforeX = openGLX;
        beforeY = openGLY;

        glutPostRedisplay();  // 화면을 다시 그립니다.
    }
}

GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y)
{
    x = (2.0f * mouseX) / windowWidth - 1.0f;
    y = 1.0f - (2.0f * mouseY) / windowHeight;
}

GLvoid TimerFunction(int value)
{
    switch (value)
    {
    case 1:
        if (BSelection == 1)
        {
            B -= 0.05;
        }
        if (BSelection == 2)
        {
            B += 0.05;
        }
        if (MSelection == 1)
        {
            M -= 2;
        }
        if (MSelection == 2)
        {
            M += 2;
        }
        if (FSelection == 1)
        {
            F += 2;
            if (F > 90)
            {
                FSelection = 2;
            }
        }
        if (FSelection == 2)
        {
            F -= 2;
            if (F < 0)
            {
                FSelection = 1;
            }
        }

        if (TSelection == 1)
        {
            T += 8;
            if (T > 90)
            {
                TSelection = 2;
            }
        }
        if (TSelection == 2)
        {
            T -= 8;
            if (T < -90)
            {
                TSelection = 1;
            }
        }
        if (RSelection == 1)
        {
            R += 3;
        }
        if (YSelection == 1)
        {
            Y += 5;
        }

        // EReturning 로직 추가
        if (EReturning)
        {
            if (E > 0.0f)
            {
                E -= 0.01f;
                if (E <= 0.0f)
                {
                    E = 0.0f;
                    EReturning = false; // 원래 위치에 도달하면 플래그 해제
                }
            }
            else if (E < 0.0f)
            {
                E += 0.01f;
                if (E >= 0.0f)
                {
                    E = 0.0f;
                    EReturning = false; // 원래 위치에 도달하면 플래그 해제
                }
            }
        }

        break;
    }
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}
