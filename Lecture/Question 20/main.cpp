#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것 (안전하지 않은 C 런타임 경고 비활성화)
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gl/glew.h> // GLEW 라이브러리 포함
#include <gl/freeglut.h> // FreeGLUT 라이브러리 포함
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp> // GLM 수학 라이브러리 포함
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>

using namespace std;

// 랜덤 숫자 생성기 초기화
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<double> XYdis(-1, 1);
uniform_real_distribution<double> dis(0, 1);

// 큐브의 각 삼각형 면 정의 (12개의 삼각형으로 구성된 6면체)
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

// 사각형의 위치 값 정의 (4개의 정점)
GLfloat rectShape[4][3] = {
	{-1,1,0},{-1,-1,0},{1,-1,0},{1,1,0}
}; //--- 사각형 위치 값

GLfloat lineShape[10][2][3] = {};	//--- 선분 위치 값 (현재 사용되지 않음)

GLfloat dotShape[10][3] = {};	//--- 점 위치 값 (현재 사용되지 않음)

// 사각형의 각 정점 색상 정의
GLfloat rectColors[4][3] = {
	{ 1.0, 0.0, 0.0 }, // 빨강
	{ 0.0, 1.0, 0.0 }, // 초록
	{ 0.0, 0.0, 1.0 }, // 파랑
	{ 1.0, 1.0, 1.0 }  // 흰색
};

// 큐브의 각 삼각형에 대한 색상 배열 (동적으로 할당됨)
glm::vec3 colors[12][3] = {};

// XYZ 축을 그리기 위한 위치 값 정의
GLfloat XYZShape[3][2][3] = {
	{{-1.0,0.0,0.0},{1.0,0.0,0.0}}, // X축
	{{0.0,-1.0,0.0},{0.0,1.0,0.0}}, // Y축
	{{0.0,0.0,-1.0},{0.0,0.0,1.0}}  // Z축
};

// XYZ 축의 색상 정의 (X: 빨강, Y: 초록, Z: 파랑)
GLfloat XYZcolors[6][3] = {
	{ 1.0, 0.0, 0.0 },	{ 1.0, 0.0, 0.0 }, // X축 빨강
	{ 0.0, 1.0, 0.0 },	{ 0.0, 1.0, 0.0 }, // Y축 초록
	{ 0.0, 0.0, 1.0 },	{ 0.0, 0.0, 1.0 }  // Z축 파랑
};

GLUquadricObj* qobj; // 쿼드릭 객체 포인터

// 카메라 설정 (여러 뷰포트를 위한 위치 및 방향)
glm::vec3 cameraPos = glm::vec3(0.0f, -0.5f, 0.5f); //--- 첫 번째 카메라 위치
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 첫 번째 카메라 바라보는 방향
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f); //--- 첫 번째 카메라 위쪽 방향

glm::vec3 cameraPos2 = glm::vec3(0.0f, 0.01f, 0.5f); //--- 두 번째 카메라 위치
glm::vec3 cameraDirection2 = glm::vec3(0.0f, 0.0f, 0.0f); //--- 두 번째 카메라 바라보는 방향
glm::vec3 cameraUp2 = glm::vec3(0.0f, 0.0f, 1.0f); //--- 두 번째 카메라 위쪽 방향

glm::vec3 cameraPos3 = glm::vec3(0.0f, -0.5f, 0.0f); //--- 세 번째 카메라 위치
glm::vec3 cameraDirection3 = glm::vec3(0.0f, 0.0f, 0.0f); //--- 세 번째 카메라 바라보는 방향
glm::vec3 cameraUp3 = glm::vec3(0.0f, 0.0f, 1.0f); //--- 세 번째 카메라 위쪽 방향

glm::vec3 D_P = glm::vec3(0.0f, 0.0f, 0.0f); //--- 추가적인 벡터 (사용 용도 불명)


// 변환 행렬 초기화
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 pTransform = glm::mat4(1.0f);

glm::mat4 view2 = glm::mat4(1.0f);
glm::mat4 pTransform2 = glm::mat4(1.0f);

glm::mat4 view3 = glm::mat4(1.0f);
glm::mat4 pTransform3 = glm::mat4(1.0f);

glm::mat4 tmodel = glm::mat4(1.0f); //--- 추가적인 모델 변환 행렬

// 버텍스 배열 객체 및 버퍼 객체 정의
GLuint vao, vbo[2];
GLuint TriPosVbo, TriColorVbo;

// 셰이더 소스 코드 및 셰이더 객체
GLchar* vertexSource, * fragmentSource; //--- 버텍스 및 프래그먼트 셰이더 소스 저장 변수
GLuint vertexShader, fragmentShader; //--- 셰이더 객체
GLuint shaderProgramID; //--- 셰이더 프로그램 ID

// 유니폼 위치 변수
int modelLocation;
int viewLocation;
int projLocation;

// 애니메이션 및 제어 변수 초기화
float B = 0;
int BSelection = 0;

float M = 0;
int MSelection = 0;

float E = 0;

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

// 윈도우 크기 설정
int windowWidth = 800;
int windowHeight = 600;

// 마우스 좌표 변환 변수
float openGLX, openGLY;
int movingRectangle = -1;

bool start = true; //--- 초기 실행 여부

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
void drawTank();

// 메인 함수: 윈도우 생성 및 콜백 함수 설정
void main(int argc, char** argv)
{
	// GLUT 초기화
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 더블 버퍼링 및 RGBA 모드 설정
	glutInitWindowPosition(100, 100); // 초기 윈도우 위치
	glutInitWindowSize(windowWidth, windowHeight); // 초기 윈도우 크기
	glutCreateWindow("Example20"); // 윈도우 생성

	// GLEW 초기화
	glewExperimental = GL_TRUE;
	glewInit();

	// 셰이더 프로그램 생성
	make_shaderProgram(); //--- 셰이더 프로그램 생성 함수 호출
	InitBuffer(); //--- 버퍼 초기화 함수 호출
	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화
	//glEnable(GL_CULL_FACE); //--- 뒷면 제거 활성화 (현재 주석 처리)
	//glDisable(GL_DEPTH_TEST | GL_CULL_FACE);	//--- 깊이 테스트 및 면 제거 비활성화 (현재 주석 처리)

	// 콜백 함수 등록
	glutTimerFunc(50, TimerFunction, 1); // 타이머 콜백 함수 등록
	glutDisplayFunc(drawScene); // 디스플레이 콜백 함수 등록
	glutReshapeFunc(Reshape); // 리쉐이프 콜백 함수 등록
	glutKeyboardFunc(Keyboard); // 키보드 콜백 함수 등록
	glutSpecialFunc(SpecialKeys); // 방향키 콜백 함수 등록
	glutMouseFunc(Mouse); // 마우스 클릭 콜백 함수 등록
	glutMotionFunc(Motion); // 마우스 움직임 콜백 함수 등록

	glutMainLoop(); // GLUT 메인 루프 시작
}

// 장면 그리기 함수 (디스플레이 콜백)
GLvoid drawScene()
{
	// 첫 번째 뷰포트 설정
	glViewport(0, 0, 450, 300);

	glUseProgram(shaderProgramID); // 셰이더 프로그램 사용
	glClearColor(0.0, 0.0, 0.0, 1.0f); // 배경색 설정 (검정색)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 컬러 및 깊이 버퍼 클리어

	glBindVertexArray(vao); // VAO 바인드

	// XYZ 축 색상 업데이트
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), XYZcolors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// 유니폼 위치 가져오기
	modelLocation = glGetUniformLocation(shaderProgramID, "model");
	viewLocation = glGetUniformLocation(shaderProgramID, "view");
	projLocation = glGetUniformLocation(shaderProgramID, "projection");

	// 투영 변환 설정
	pTransform = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f); // 원근 투영 설정
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0)); // Z축 이동
	pTransform = glm::perspective(glm::radians(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 200.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &pTransform[0][0]); // 프로젝션 유니폼 설정

	// 카메라 회전 및 위치 설정
	if (YSelection == 1)
	{
		cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraDirection2 = glm::vec3(0.0f, 0.0f, 0.0f); //--- 두 번째 카메라 바라보는 방향
		cameraDirection3 = glm::vec3(0.0f, 0.0f, 0.0f); //--- 세 번째 카메라 바라보는 방향
	}

	if (RSelection == 1)
	{
		cameraPos = glm::vec3(0.0f, -0.5f, 0.5f);
		cameraPos2 = glm::vec3(0.0f, 0.01f, 0.5f); //--- 두 번째 카메라 위치
		cameraPos3 = glm::vec3(0.0f, -0.5f, 0.0f);
	}

	if (YSelection == 1)
	{
		cameraDirection -= cameraPos;
		cameraDirection = glm::rotate(glm::mat4(1.0f), glm::radians(Y), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraDirection, 1.0);
		cameraDirection += cameraPos;

		cameraDirection2 -= cameraPos2;
		cameraDirection2 = glm::rotate(glm::mat4(1.0f), glm::radians(Y), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraDirection2, 1.0);
		cameraDirection2 += cameraPos2;

		cameraDirection3 -= cameraPos3;
		cameraDirection3 = glm::rotate(glm::mat4(1.0f), glm::radians(Y), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraDirection3, 1.0);
		cameraDirection3 += cameraPos3;
	}

	if (RSelection == 1)
	{
		cameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(R), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos, 1.0);
		cameraPos2 = glm::rotate(glm::mat4(1.0f), glm::radians(R), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos2, 1.0);
		cameraPos3 = glm::rotate(glm::mat4(1.0f), glm::radians(R), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos3, 1.0);
	}
	if (OSelection == 1)
	{
		cameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(O), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos, 1.0);
		cameraPos2 = glm::rotate(glm::mat4(1.0f), glm::radians(O), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos2, 1.0);
		cameraPos3 = glm::rotate(glm::mat4(1.0f), glm::radians(O), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraPos3, 1.0);
		OSelection = 0;
	}

	// 뷰 변환 설정
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	// 초기 설정 (한 번만 실행)
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
		// 콘솔에 명령어 설명 출력
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

	drawTank(); // 탱크 그리기 함수 호출

	// 두 번째 뷰포트 설정
	glViewport(500, 300, 250, 250);
	// 투영 변환 설정 (직교 투영)
	pTransform2 = glm::mat4(1.0f);
	pTransform2 = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &pTransform2[0][0]);

	// 두 번째 카메라 뷰 변환 설정
	view2 = glm::lookAt(cameraPos2, cameraDirection2, cameraUp2);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view2[0][0]);

	drawTank(); // 탱크 그리기 함수 호출

	// 세 번째 뷰포트 설정
	glViewport(500, 25, 250, 250);
	// 투영 변환 설정 (직교 투영)
	pTransform3 = glm::mat4(1.0f);
	pTransform3 = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &pTransform3[0][0]);

	// 세 번째 카메라 뷰 변환 설정
	view3 = glm::lookAt(cameraPos3, cameraDirection3, cameraUp3);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view3[0][0]);

	drawTank(); // 탱크 그리기 함수 호출

	glutSwapBuffers(); //--- 더블 버퍼링 스왑
}

// 창 리사이즈 콜백 함수
GLvoid Reshape(int w, int h)
{
	// 현재는 뷰포트를 변경하지 않음
	//glViewport(0, 0, w, h);
}

// 버퍼 초기화 함수
void InitBuffer()
{
	glGenVertexArrays(1, &vao); // VAO 생성
	glBindVertexArray(vao); // VAO 바인드
	glGenBuffers(2, vbo); // VBO 2개 생성
}

// 셰이더 프로그램 생성 함수
void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 셰이더 생성 함수 호출
	make_fragmentShaders(); //--- 프래그먼트 셰이더 생성 함수 호출
	// 셰이더 프로그램 생성 및 링크
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	// 컴파일된 셰이더 삭제
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	// 셰이더 프로그램 사용
	glUseProgram(shaderProgramID);
}

// 버텍스 셰이더 생성 함수
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl"); // 셰이더 파일 읽기
	// 버텍스 셰이더 객체 생성
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// 셰이더 코드 설정
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	// 버텍스 셰이더 컴파일
	glCompileShader(vertexShader);
	// 컴파일 오류 체크
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

// 프래그먼트 셰이더 생성 함수
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl"); // 셰이더 파일 읽기
	// 프래그먼트 셰이더 객체 생성
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// 셰이더 코드 설정
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	// 프래그먼트 셰이더 컴파일
	glCompileShader(fragmentShader);
	// 컴파일 오류 체크
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

// 파일을 버퍼로 읽는 함수
char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // 파일 열기 (바이너리 모드)
	if (!fptr) // 실패 시 NULL 반환
		return NULL;
	fseek(fptr, 0, SEEK_END); // 파일 끝으로 이동
	length = ftell(fptr); // 파일 길이 측정
	buf = (char*)malloc(length + 1); // 버퍼 할당 (널 종료 문자 포함)
	fseek(fptr, 0, SEEK_SET); // 파일 시작으로 이동
	fread(buf, length, 1, fptr); // 파일 내용 읽기
	fclose(fptr); // 파일 닫기
	buf[length] = 0; // 널 종료 문자 추가
	return buf; // 버퍼 반환
}

// 애니메이션 제어를 위한 변수들
bool cdx1 = true;
bool bcnt = true, Bcnt = true, mcnt = true, Mcnt = true, Ecnt = true, fcnt = true, Fcnt = true;
bool tcnt = true, Tcnt = true, rcnt = true, ycnt = true;

// 키보드 입력 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '-':
		cameraPos.z += 0.1; // 카메라 Z축 이동 (뒤로)
		cameraPos2.z += 0.1;
		cameraPos3.z += 0.1;
		break;
	case '=':
		cameraPos.z -= 0.1; // 카메라 Z축 이동 (앞으로)
		cameraPos2.z -= 0.1;
		cameraPos3.z -= 0.1;
		break;
	case 'w':
		cameraDirection.y += 0.1; // 카메라 Y축 방향 조정
		cameraDirection2.y += 0.1;
		cameraDirection3.y += 0.1;
		break;
	case 'a':
		cameraDirection.x -= 0.1; // 카메라 X축 방향 조정
		cameraDirection2.x -= 0.1;
		cameraDirection3.x -= 0.1;
		break;
	case 's':
		cameraDirection.y -= 0.1; // 카메라 Y축 방향 조정
		cameraDirection2.y -= 0.1;
		cameraDirection3.y -= 0.1;
		break;
	case 'd':
		cameraDirection.x += 0.1; // 카메라 X축 방향 조정
		cameraDirection2.x += 0.1;
		cameraDirection3.x += 0.1;
		break;
	case 'b':
		if (Bcnt)
		{
			BSelection = 1; // 아래 몸체 x축 양방향 이동 선택
			Bcnt = false;
		}
		else
		{
			BSelection = 0; // 이동 정지
			Bcnt = true;
		}
		break;
	case 'B':
		if (Bcnt)
		{
			BSelection = 2; // 아래 몸체 x축 음방향 이동 선택
			Bcnt = false;
		}
		else
		{
			BSelection = 0; // 이동 정지
			Bcnt = true;
		}
		break;
	case 'm':
		if (mcnt)
		{
			MSelection = 1; // 중앙 몸체 y축 양방향 회전 선택
			mcnt = false;
		}
		else
		{
			MSelection = 0; // 회전 정지
			mcnt = true;
		}
		break;
	case 'M':
		if (Mcnt)
		{
			MSelection = 2; // 중앙 몸체 y축 음방향 회전 선택
			Mcnt = false;
		}
		else
		{
			MSelection = 0; // 회전 정지
			Mcnt = true;
		}
		break;
	case 'e':
		if (Ecnt)
		{
			E += 0.01; // 포신 이동
			if (E > 0.05)
			{
				Ecnt = false;
			}
		}
		else
		{
			E -= 0.01; // 포신 이동 반대 방향
			if (E < -0.05)
			{
				Ecnt = true;
			}
		}
		break;
	case 'E':
		break; // 'E' 입력에 대한 동작 정의 없음
	case 'f':
		if (fcnt)
		{
			FSelection = 1; // 포신 y축 양방향 회전 선택
			fcnt = false;
		}
		else
		{
			FSelection = 0; // 회전 정지
			fcnt = true;
		}
		break;
	case 'F':
		if (Fcnt)
		{
			FSelection = 2; // 포신 y축 음방향 회전 선택
			Fcnt = false;
		}
		else
		{
			FSelection = 0; // 회전 정지
			Fcnt = true;
		}
		break;
	case 't':
		if (tcnt)
		{
			TSelection = 1; // 팔 z축 양방향 회전 선택
			tcnt = false;
		}
		else
		{
			TSelection = 0; // 회전 정지
			tcnt = true;
		}
		break;
	case 'T':
		if (Tcnt)
		{
			TSelection = 2; // 팔 z축 음방향 회전 선택
			Tcnt = false;
		}
		else
		{
			TSelection = 0; // 회전 정지
			Tcnt = true;
		}
		break;
	case 'z':
		Z += 1; // Z값 증가 (사용 용도 불명)
		break;
	case 'Z':
		Z -= 1; // Z값 감소 (사용 용도 불명)
		break;
	case 'y':
		if (ycnt)
		{
			YSelection = 1; // 카메라 y축 회전 활성화
			ycnt = false;
		}
		else
		{
			YSelection = 0; // 카메라 y축 회전 비활성화
			ycnt = true;
		}
		break;
	case 'o':
		cameraPos = glm::vec3(0.0f, -0.5f, 0.5f); // 카메라 위치 초기화
		cameraPos2 = glm::vec3(0.0f, 0.01f, 0.5f); // 두 번째 카메라 위치 초기화
		cameraPos3 = glm::vec3(0.0f, -0.5f, 0.0f); // 세 번째 카메라 위치 초기화
		OSelection = 1; // 공전 활성화
		O += 1; // 공전 각도 증가
		break;
	case 'r':
		cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); // 카메라 방향 초기화
		cameraDirection2 = glm::vec3(0.0f, 0.0f, 0.0f); // 두 번째 카메라 방향 초기화
		cameraDirection3 = glm::vec3(0.0f, 0.0f, 0.0f); // 세 번째 카메라 방향 초기화
		if (rcnt)
		{
			RSelection = 1; // 회전 애니메이션 활성화
			rcnt = false;
		}
		else
		{
			RSelection = 0; // 회전 애니메이션 비활성화
			rcnt = true;
		}
		break;
	case 'x':
		BSelection = 0; // 모든 이동 및 회전 정지
		MSelection = 0;
		FSelection = 0;
		TSelection = 0;
		RSelection = 0;
		YSelection = 0;
		break;
	case 'c':
		// 모든 카메라 설정 초기화
		cameraPos = glm::vec3(0.0f, -0.5f, 0.5f);
		cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

		cameraPos2 = glm::vec3(0.0f, 0.01f, 0.5f);
		cameraDirection2 = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraUp2 = glm::vec3(0.0f, 0.0f, 1.0f);

		cameraPos3 = glm::vec3(0.0f, -0.5f, 0.0f);
		cameraDirection3 = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraUp3 = glm::vec3(0.0f, 0.0f, 1.0f);

		// 애니메이션 변수 초기화
		B = 0;
		BSelection = 0;
		M = 0;
		MSelection = 0;
		E = 0;
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
		break;
	case 'q':
		glutLeaveMainLoop(); // 프로그램 종료
		break;
	}
	glutPostRedisplay(); // 화면 갱신 요청
}

// 특수 키(방향키) 콜백 함수
GLvoid SpecialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		cameraPos.y += 0.1; // 카메라 Y축 양방향 이동
		cameraDirection.y += 0.1;
		cameraPos2.y += 0.1;
		cameraDirection2.y += 0.1;
		cameraPos3.y += 0.1;
		cameraDirection3.y += 0.1;
		break;
	case GLUT_KEY_DOWN:
		cameraPos.y -= 0.1; // 카메라 Y축 음방향 이동
		cameraDirection.y -= 0.1;
		cameraPos2.y -= 0.1;
		cameraDirection2.y -= 0.1;
		cameraPos3.y -= 0.1;
		cameraDirection3.y -= 0.1;
		break;
	case GLUT_KEY_LEFT:
		cameraPos.x -= 0.1; // 카메라 X축 음방향 이동
		cameraDirection.x -= 0.1;
		cameraPos2.x -= 0.1;
		cameraDirection2.x -= 0.1;
		cameraPos3.x -= 0.1;
		cameraDirection3.x -= 0.1;
		break;
	case GLUT_KEY_RIGHT:
		cameraPos.x += 0.1; // 카메라 X축 양방향 이동
		cameraDirection.x += 0.1;
		cameraPos2.x += 0.1;
		cameraDirection2.x += 0.1;
		cameraPos3.x += 0.1;
		cameraDirection3.x += 0.1;
		break;
	}
	glutPostRedisplay(); // 화면 갱신 요청
}

int movingMouse = -1; // 마우스 이동 상태
float beforeX, beforeY; // 이전 마우스 위치

// 마우스 클릭 콜백 함수
GLvoid Mouse(int button, int state, int x, int y)
{
	float openGLX, openGLY;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		WindowToOpenGL(x, y, openGLX, openGLY); // 윈도우 좌표를 OpenGL 좌표로 변환

		movingMouse = 0; // 마우스 이동 상태 설정

		beforeX = openGLX; // 이전 X 저장
		beforeY = openGLY; // 이전 Y 저장
	}
	else if (state == GLUT_UP)
	{
		movingMouse = -1; // 마우스 이동 상태 해제
	}
}

// 마우스 이동 콜백 함수
GLvoid Motion(int x, int y)
{
	if (movingMouse >= 0)
	{
		WindowToOpenGL(x, y, openGLX, openGLY); // 윈도우 좌표를 OpenGL 좌표로 변환

		float deltaX = openGLX - beforeX; // X 변화량 계산
		float deltaY = openGLY - beforeY; // Y 변화량 계산

		// 이동 로직 추가 가능

		// 현재 마우스 위치 업데이트
		beforeX = openGLX;
		beforeY = openGLY;

		glutPostRedisplay();  // 화면 갱신 요청
	}
}

// 윈도우 좌표를 OpenGL 좌표로 변환하는 함수
GLvoid WindowToOpenGL(int mouseX, int mouseY, float& x, float& y)
{
	x = (2.0f * mouseX) / windowWidth - 1.0f; // X 좌표 변환
	y = 1.0f - (2.0f * mouseY) / windowHeight; // Y 좌표 변환
}

// 타이머 콜백 함수
GLvoid TimerFunction(int value)
{
	switch (value)
	{
	case 1:
		if (BSelection == 1)
		{
			B -= 0.05; // B 값 감소 (x축 음방향 이동)
		}
		if (BSelection == 2)
		{
			B += 0.05; // B 값 증가 (x축 양방향 이동)
		}
		if (MSelection == 1)
		{
			M -= 2; // M 값 감소 (y축 회전)
		}
		if (MSelection == 2)
		{
			M += 2; // M 값 증가 (y축 회전)
		}
		if (FSelection == 1)
		{
			F += 2; // F 값 증가 (포신 회전)
			if (F > 90)
			{
				FSelection = 2; // 회전 방향 변경
			}
		}
		if (FSelection == 2)
		{
			F -= 2; // F 값 감소 (포신 회전)
			if (F < 0)
			{
				FSelection = 1; // 회전 방향 변경
			}
		}

		if (TSelection == 1)
		{
			T += 8; // T 값 증가 (팔 회전)
			if (T > 90)
			{
				TSelection = 2; // 회전 방향 변경
			}
		}
		if (TSelection == 2)
		{
			T -= 8; // T 값 감소 (팔 회전)
			if (T < -90)
			{
				TSelection = 1; // 회전 방향 변경
			}
		}
		if (RSelection == 1)
		{
			R += 3; // R 값 증가 (카메라 회전)
		}
		if (YSelection == 1)
		{
			Y += 5; // Y 값 증가 (카메라 회전)
		}
		break;
	}
	glutPostRedisplay(); // 화면 갱신 요청
	glutTimerFunc(50, TimerFunction, 1); // 타이머 다시 등록
}

// 탱크 그리기 함수
void drawTank()
{
	modelLocation = glGetUniformLocation(shaderProgramID, "model"); //--- 모델 변환 유니폼 위치 가져오기
	viewLocation = glGetUniformLocation(shaderProgramID, "view"); //--- 뷰 변환 유니폼 위치 가져오기
	projLocation = glGetUniformLocation(shaderProgramID, "projection"); //--- 프로젝션 변환 유니폼 위치 가져오기

	model = glm::mat4(1.0f); // 모델 변환 행렬 초기화

	// XYZ 축 그리기
	for (int i = 0; i < 3; i++)
	{
		// 축 색상 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZcolors[i * 2], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// 모델 변환 유니폼 설정
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		// 축 위치 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), XYZShape[i], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glLineWidth(1.0); // 선 두께 설정
		glDrawArrays(GL_LINES, 0, 2); // 선 그리기
	}

	// 바닥 그리기
	for (int i = 0; i < 4; i++)
	{
		// 사각형 색상 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), rectColors[i], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// 사각형 위치 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), rectShape, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// 사각형 그리기 (폴리곤 모드 설정 가능)
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_QUADS, 0, 4); // 사각형 그리기
	}
	//-------------------------------------------------
	// 모델 변환 초기화 및 회전, 이동, 스케일 적용
	model = glm::mat4(1.0f);
	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));
	// z축 이동
	model = glm::translate(model, glm::vec3(0.0, 0.0, 0.15f));
	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.1));

	// 위 몸통 그리기
	for (int i = 0; i < 12; i++) {
		// 모델 변환 유니폼 설정
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		// 큐브 색상 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &colors[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// 큐브 위치 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &cube[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// 삼각형 그리기
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	//-------------------------------------------------
	// 위 왼쪽 대포 그리기
	model = glm::mat4(1.0f);

	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// T 회전 (x축 회전)
	model = glm::rotate(model, glm::radians(T / 3), glm::vec3(1.0f, 0.0f, 0.0f));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));

	// 대포 위치 이동
	model = glm::translate(model, glm::vec3(-0.05f, 0.0f, 0.2f));
	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));

	// 모델 변환 유니폼 설정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// 쿼드릭 객체 생성 및 대포 그리기
	qobj = gluNewQuadric(); // 쿼드릭 객체 생성
	gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일 설정 (채우기)
	gluQuadricNormals(qobj, GLU_SMOOTH); // 노멀 설정 (부드럽게)
	gluQuadricOrientation(qobj, GLU_OUTSIDE); // 방향 설정
	gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 원기둥 그리기 (대포)

	//--------------------------------------------------------
	// 위 오른쪽 대포 그리기
	model = glm::mat4(1.0f);
	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// T 회전 (x축 회전, 반대 방향)
	model = glm::rotate(model, glm::radians(-T / 3), glm::vec3(1.0f, 0.0f, 0.0f));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));

	// 대포 위치 이동
	model = glm::translate(model, glm::vec3(0.05f, 0.0f, 0.2f));
	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));

	// 모델 변환 유니폼 설정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// 쿼드릭 객체 생성 및 대포 그리기
	qobj = gluNewQuadric(); // 쿼드릭 객체 생성
	gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일 설정 (채우기)
	gluQuadricNormals(qobj, GLU_SMOOTH); // 노멀 설정 (부드럽게)
	gluQuadricOrientation(qobj, GLU_OUTSIDE); // 방향 설정
	gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 원기둥 그리기 (대포)

	//-------------------------------------------------
	// 모델 변환 초기화 및 회전, 이동, 스케일 적용
	model = glm::mat4(1.0f);
	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));

	// 이동 (현재는 원점)
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

	// 아래 몸통 그리기
	for (int i = 0; i < 12; i++) {
		// 모델 변환 유니폼 설정
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		// 큐브 색상 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &colors[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// 큐브 위치 업데이트
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec3), &cube[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// 삼각형 그리기
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	//-----------------------------------------------
	// 아래 왼쪽 대포 그리기
	model = glm::mat4(1.0f);
	// F 회전 (포신 회전)
	model = glm::rotate(model, glm::radians(-F), glm::vec3(0.0f, 0.0f, 1.0f));
	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// x축으로 90도 회전
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// E 이동 (포신 이동)
	model = glm::translate(model, glm::vec3(-E, 0, 0));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));
	// 대포 위치 이동
	model = glm::translate(model, glm::vec3(-0.05f, 0.05f, 0.05f));
	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

	// 모델 변환 유니폼 설정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// 쿼드릭 객체 생성 및 대포 그리기
	qobj = gluNewQuadric(); // 쿼드릭 객체 생성
	gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일 설정 (채우기)
	gluQuadricNormals(qobj, GLU_SMOOTH); // 노멀 설정 (부드럽게)
	gluQuadricOrientation(qobj, GLU_OUTSIDE); // 방향 설정
	gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 원기둥 그리기 (대포)
	//--------------------------------------------------------
	// 아래 오른쪽 대포 그리기
	model = glm::mat4(1.0f);
	// F 회전 (포신 회전)
	model = glm::rotate(model, glm::radians(F), glm::vec3(0.0f, 0.0f, 1.0f));
	// M 회전 (y축 회전)
	model = glm::rotate(model, glm::radians(M), glm::vec3(0.0f, 0.0f, 1.0f));
	// x축으로 90도 회전
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	// E 이동 (포신 이동)
	model = glm::translate(model, glm::vec3(E, 0, 0));
	// B 이동 (x축 이동)
	model = glm::translate(model, glm::vec3(B, 0, 0));
	// 대포 위치 이동
	model = glm::translate(model, glm::vec3(0.05f, 0.05f, 0.05f));
	// 스케일 조정
	model = glm::scale(model, glm::vec3(0.25, 0.25, 0.25));

	// 모델 변환 유니폼 설정
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// 쿼드릭 객체 생성 및 대포 그리기
	qobj = gluNewQuadric(); // 쿼드릭 객체 생성
	gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일 설정 (채우기)
	gluQuadricNormals(qobj, GLU_SMOOTH); // 노멀 설정 (부드럽게)
	gluQuadricOrientation(qobj, GLU_OUTSIDE); // 방향 설정
	gluCylinder(qobj, 0.1, 0.1, 0.5, 20, 8); // 원기둥 그리기 (대포)
}
