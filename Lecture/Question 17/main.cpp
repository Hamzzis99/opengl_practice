#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vector>
#include "open_file.h"
#include "open_obj.h"

// 쉐이더 선언
#define vertex_shader_glsl "vertex.glsl"
#define fragment_shader_glsl "fragment.glsl"

// random 함수들
std::random_device rd;
std::default_random_engine dre(rd());

//-------------------------------- 콜백 함수 선언
GLvoid drawScene(GLvoid);
// 화면에 그릴 장면을 정의하고 렌더링하는 함수입니다. 주로 OpenGL 명령을 사용하여 그래픽 요소를 그리는 역할을 합니다.

GLvoid Reshape(int w, int h);
// 창 크기가 조정될 때 호출되는 함수입니다. 'w'는 새 창의 너비를, 'h'는 새 창의 높이를 나타내며, 뷰포트를 조정하거나 투영 행렬을 설정하는 데 사용됩니다.

GLvoid Keyboard(unsigned char key, int x, int y);
// 키보드 입력이 발생했을 때 호출되는 함수입니다. 'key'는 눌린 키를 나타내며, 'x'와 'y'는 키 입력이 발생한 마우스 위치 좌표를 나타냅니다.

GLvoid SpecialKeyboard(int key, int x, int y);
// 특수 키(예: 화살표 키, F1~F12 등)가 눌렸을 때 호출되는 함수입니다. 'key'는 눌린 특수 키의 코드이고, 'x'와 'y'는 키 입력이 발생한 마우스 위치 좌표입니다.

GLvoid timer(int);
// 타이머 콜백 함수로, 일정한 간격으로 실행되도록 설정할 수 있습니다. 'int' 매개변수는 타이머와 관련된 값을 전달하는 데 사용될 수 있습니다.

// 쉐이더 관련 변수 선언
GLuint shader_program;
// 쉐이더 프로그램 객체입니다. 이 프로그램은 버텍스 셰이더와 프래그먼트 셰이더를 결합하여 GPU에서 실행될 전체 셰이더 프로그램을 나타냅니다.

GLuint vertexShader;
// 버텍스 셰이더 객체입니다. 버텍스(정점) 관련 작업을 처리하는 코드가 포함되며, 셰이더 프로그램에 컴파일되고 연결됩니다.

GLuint fragmentShader;
// 프래그먼트 셰이더 객체입니다. 프래그먼트(픽셀) 관련 작업을 처리하는 코드가 포함되며, 셰이더 프로그램에 컴파일되고 연결됩니다.

GLuint VAO[2], VBO[2], CBO[2], EBO[2], a_axis, b_axis;
// VAO (Vertex Array Object): 정점 배열 객체입니다. 정점 데이터를 관리하고 OpenGL에서 효율적인 상태 전환을 위해 사용됩니다.
// VBO (Vertex Buffer Object): 정점 버퍼 객체입니다. 정점 위치 데이터를 저장하는 데 사용됩니다.
// CBO (Color Buffer Object): 색상 버퍼 객체입니다. 정점의 색상 데이터를 저장하는 데 사용됩니다.
// EBO (Element Buffer Object): 요소 버퍼 객체입니다. 인덱스 데이터(정점을 그리는 순서)를 저장하여 정점 데이터를 재사용하고, 메모리 효율성을 높입니다.

// 함수 선언
void make_vertex_shader();
void make_fragment_shader();
GLuint make_shader();
GLvoid init_buffer();

// RGB값과 해상도. (고정)
GLclampf base_r = 0.0f;
GLclampf base_g = 0.0f;
GLclampf base_b = 0.0f;
GLint width{ 800 }, height{ 600 };

Model cube;
Model pyramid;

//------------------------ Bool 변수들 (가독성 개선된 변수 이름)
bool shouldDrawCube = false;           // 큐브를 그려야 하는지 여부
bool shouldDrawPyramid = false;        // 피라미드를 그려야 하는지 여부
bool isWireframeMode = false;          // 와이어프레임 모드 활성화 여부
bool isFaceCullingEnabled = false;     // 면 제거(Face Culling) 활성화 여부
bool isSpinning = false;               // 회전 애니메이션 활성화 여부
bool showTopFace = false;              // 상단 면 표시 여부
bool showFrontFace = false;            // 전면 면 표시 여부
bool showSideFace = false;             // 측면 면 표시 여부
bool showBackFace = false;             // 후면 면 표시 여부
bool isObjectOpen = false;             // 객체 열림 상태 여부 (예: 문 열림 등)
bool isOpenRotationEnabled = false;    // 열림 상태에서의 회전 활성화 여부
int animation_flag = -1;                // 애니메이션 상태 플래그 (변경하지 않음)
bool usePerspectiveProjection = false;  // 원근 투영 사용 여부

bool pyramid1_flag = false;
bool pyramid2_flag = false;
glm::mat4 translate_mat(1.0f); // 행렬 초기화 (x, y, z, 1)

// 색상 배열
const float cube_color[] = {
    1.0f, 0.0f, 0.0f, // Red
    0.0f, 1.0f, 0.0f, // Green
    0.0f, 0.0f, 1.0f, // Blue
    1.0f, 1.0f, 0.0f, // Yellow
    0.0f, 1.0f, 1.0f, // Cyan
    1.0f, 0.0f, 1.0f, // Magenta
    0.0f, 0.0f, 0.0f, // Black
    1.0f, 1.0f, 1.0f  // White
};

const float pyramid_color[] = {
    1.0f, 0.0f, 0.0f, // Red
    0.0f, 1.0f, 0.0f, // Green
    0.0f, 0.0f, 1.0f, // Blue
    1.0f, 1.0f, 0.0f, // Yellow
    0.0f, 1.0f, 1.0f  // Cyan
};

const float axis[] = {
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // X축 Red

    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Y축 Green

    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f  // Z축 Blue
};

float axis_dx{};
float axis_dy{};

float dx{};
float dy{};

// 육면체 선언
float face_dx[6];
float face_dy[6];

float face_rotatex[6];
float face_rotatey[6];
float face_rotatez[6];
float face_scale[6]{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

float for_face_rotate_dx{};
float for_face_rotate_dy{};

// 피라미드 선언
float pyramid_rotate_x[4];
float pyramid_rotate_y[4];
float pyramid_rotate_z[4];

float pyramid_dx[4];
float pyramid_dy[4];
float pyramid_dz[4];

int face; // 이건 뭐여

// main 함수
int main(int argc, char** argv) {
    // GLUT 초기화 및 윈도우 생성
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example 17");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패" << std::endl;
        return -1;
    }

    // 셰이더 생성 및 프로그램 링크
    make_vertex_shader();
    make_fragment_shader();
    shader_program = make_shader();

    // 콜백 함수 등록
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(10, timer, 0);

    // OBJ 파일 읽기
    read_obj_file("cube.obj", &cube);
    read_obj_file("pyramid.obj", &pyramid);

    // 깊이 테스트 활성화
    glEnable(GL_DEPTH_TEST);

    // 버퍼 초기화
    init_buffer();

    // 메인 루프 시작
    glutMainLoop();

    return 0;
}

GLUquadricObj* qobj; // 잘 모르는거 이따 봐야함

// 콜백 함수 정의

GLvoid drawScene(GLvoid) {
    // 배경 색상 설정 및 버퍼 초기화
    glClearColor(base_r, base_g, base_b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 활성화할 셰이더 프로그램을 지정합니다.
    glUseProgram(shader_program);

    // 'trans'라는 이름의 uniform 변수의 위치를 가져옵니다.
    GLuint modelLoc2 = glGetUniformLocation(shader_program, "trans");
    // 'trans_by_face'라는 이름의 uniform 변수의 위치를 가져옵니다.
    GLuint facloc = glGetUniformLocation(shader_program, "trans_by_face");
    // 다시 한 번 'trans'라는 이름의 uniform 변수의 위치를 가져옵니다.
    // 주의: 이미 'trans'의 위치를 'modelLoc2'에 저장했으므로, 
    // 이 줄은 중복될 수 있습니다. 만약 다른 목적으로 사용된다면 이름을 다르게 하는 것이 좋습니다.
    GLuint modloc = glGetUniformLocation(shader_program, "trans");

    // 축 VAO 바인딩
    glBindVertexArray(a_axis);

    // 생성된 단위 행렬을 셰이더 프로그램의 'trans_by_face' uniform 변수에 전달합니다.
    glm::mat4 temp(1.0f);
    glUniformMatrix4fv(facloc, 1, GL_FALSE, glm::value_ptr(temp));

    // 축 회전 적용
    temp = glm::rotate(temp, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
    temp = glm::rotate(temp, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));

    // 선 그리기 (축)
    glDrawArrays(GL_LINES, 0, 12); // 정점 배열을 사용하여 선 그리기

    // 면 제거(Face Culling) 설정
    if (isFaceCullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    // 프로젝션 행렬 설정
    glm::mat4 proj1 = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f);
    glm::mat4 proj2 = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
    proj2 = glm::translate(proj2, glm::vec3(0.0f, 0.0f, -5.0f));
    GLuint projection = glGetUniformLocation(shader_program, "projection");
    if (usePerspectiveProjection) {
        glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(proj2));
    }
    else {
        glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(proj1));
    }

    // 육면체의 각 면에 대한 변환 행렬 계산
    glm::mat4 cube_face_trans[6] = {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };
    for (int i = 0; i < 6; ++i) {
        // 행렬에 축소(스케일) 적용
        cube_face_trans[i] = glm::scale(
            cube_face_trans[i],
            glm::vec3(face_scale[i], face_scale[i], face_scale[i])
        );

        // 행렬에 x축 및 y축 방향으로 이동(트랜슬레이션) 적용
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(face_dx[i], face_dy[i], 0.0f)
        );

        // 행렬에 y축을 중심으로 회전 적용
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatey[i]),
            glm::vec3(0.0, 1.0, 0.0)
        );

        // 행렬에 전체 회전을 위한 추가 이동(트랜슬레이션) 적용
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(0.0f, -for_face_rotate_dy, -for_face_rotate_dx)
        );

        // 행렬에 x축을 중심으로 회전 적용
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatex[i]),
            glm::vec3(1.0, 0.0, 0.0)
        );

        // 행렬에 z축을 중심으로 회전 적용
        cube_face_trans[i] = glm::rotate(
            cube_face_trans[i],
            glm::radians(face_rotatez[i]),
            glm::vec3(0.0, 0.0, 1.0)
        );

        // 행렬에 전체 회전을 위한 추가 이동(트랜슬레이션) 적용
        cube_face_trans[i] = glm::translate(
            cube_face_trans[i],
            glm::vec3(0.0f, for_face_rotate_dy, for_face_rotate_dx)
        );
    }

    // 큐브 VAO 바인딩
    glBindVertexArray(VAO[0]);
    if (shouldDrawCube) {
        for (int i = 0; i < cube.face_count / 2; ++i) {
            glUniformMatrix4fv(facloc, 1, GL_FALSE, glm::value_ptr(cube_face_trans[i]));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 6));
        }
    }

    // 객체 이동 및 회전 행렬 설정
    glm::mat4 tr_mat2 = glm::mat4(1.0f);
    tr_mat2 = glm::translate(tr_mat2, glm::vec3(dx, dy, 0.0f));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(30.0f + axis_dx), glm::vec3(1.0, 0.0, 0.0));
    tr_mat2 = glm::rotate(tr_mat2, glm::radians(30.0f + axis_dy), glm::vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modloc, 1, GL_FALSE, glm::value_ptr(tr_mat2));

    // 피라미드의 각 면에 대한 변환 행렬 계산
    glm::mat4 pyramid_face[4] = {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };
    for (int i = 0; i < 4; ++i) {
        pyramid_face[i] = glm::translate(
            pyramid_face[i],
            glm::vec3(-pyramid_dx[i], -pyramid_dy[i], -pyramid_dz[i])
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_x[i]),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_y[i]),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        pyramid_face[i] = glm::rotate(
            pyramid_face[i],
            glm::radians(pyramid_rotate_z[i]),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        pyramid_face[i] = glm::translate(
            pyramid_face[i],
            glm::vec3(pyramid_dx[i], pyramid_dy[i], pyramid_dz[i])
        );
    }

    glm::mat4 p_temp = glm::mat4(1.0f);
    GLuint pyramid_mat = glGetUniformLocation(shader_program, "trans_by_face");
    glBindVertexArray(VAO[1]);
    if (shouldDrawPyramid) {
        for (int i = 0; i < pyramid.face_count; ++i) {
            // 2는 왼쪽, 3앞, 4오, 5뒤
            if (i > 1)
            {
                glUniformMatrix4fv(pyramid_mat, 1, GL_FALSE, glm::value_ptr(pyramid_face[i - 2]));
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 3));
            }
            else {
                glUniformMatrix4fv(pyramid_mat, 1, GL_FALSE, glm::value_ptr(p_temp));
                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(unsigned int) * 3));
            }
        }
    }

    // 더블 버퍼링 스왑
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c':
        shouldDrawCube = !shouldDrawCube;
        break;

    case 'v':
        shouldDrawPyramid = !shouldDrawPyramid;
        break;

    case 'p':
        usePerspectiveProjection = !usePerspectiveProjection;
        break;

    case 'y':
        isSpinning = !isSpinning;
        break;
    case 't':
        showTopFace = !showTopFace;
        break;

    case 'f':
        showFrontFace = !showFrontFace;
        break;

    case 's':
        showSideFace = !showSideFace;
        break;

    case 'b':
        showBackFace = !showBackFace;
        break;

    case 'h':
        isFaceCullingEnabled = !isFaceCullingEnabled;
        break;

    case 'o':
        pyramid1_flag = !pyramid1_flag; // 변경된 변수명 사용
        break;

    case 'r':
        pyramid2_flag = !pyramid2_flag; // 변경된 변수명 사용
        break;

    case 'q':
        glutLeaveMainLoop();
        break;

    case '1':
        face_scale[1] -= 0.1f;
        break;

    case '2':
        // 추가 동작이 필요한 경우 여기에 작성
        break;
    }

    if ('1' <= key && key <= '6') {
        face = key - '1';
    }

    glutPostRedisplay();
}

void make_vertex_shader() {
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertexSource;
    vertexSource = open_file_to_buf(vertex_shader_glsl);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "컴파일 성공\n";
    }
}

void make_fragment_shader() {
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragmentSource;
    fragmentSource = open_file_to_buf(fragment_shader_glsl);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
    else {
        std::cout << "컴파일 성공\n";
    }
}

GLuint make_shader() {
    GLuint shader01;
    shader01 = glCreateProgram();
    glAttachShader(shader01, vertexShader);
    glAttachShader(shader01, fragmentShader);

    glLinkProgram(shader01);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shader01, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader01, 512, NULL, errorLog);
        std::cerr << "셰이더 생성 실패\n" << errorLog << std::endl;
    }
    else {
        std::cout << "\n컴파일 성공\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    glUseProgram(shader01);

    return shader01;
}

// Init_buffer 함수는 여러 3D 객체(큐브, 피라미드, 축)에 대한 버퍼 설정을 수행합니다.
GLvoid init_buffer() {
    // 큐브 VAO 생성 및 바인딩
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);

    // 큐브 VBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, cube.vertex_count * sizeof(Vertex), cube.vertices, GL_STATIC_DRAW);

    // 정점 속성 설정 (위치)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 큐브 EBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &EBO[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.face_count * sizeof(Face), cube.faces, GL_STATIC_DRAW);

    // 큐브 CBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &CBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, CBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_color), cube_color, GL_STATIC_DRAW);

    // 정점 속성 설정 (색상)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(1);

    // 피라미드 VAO 생성 및 바인딩
    glGenVertexArrays(1, &VAO[1]);
    glBindVertexArray(VAO[1]);

    // 피라미드 VBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &VBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, pyramid.vertex_count * sizeof(Vertex), pyramid.vertices, GL_STATIC_DRAW);

    // 정점 속성 설정 (위치)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // 피라미드 CBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &CBO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, CBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_color), pyramid_color, GL_STATIC_DRAW);

    // 정점 속성 설정 (색상)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(1);

    // 피라미드 EBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &EBO[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pyramid.face_count * sizeof(Face), pyramid.faces, GL_STATIC_DRAW);

    // 축 VAO 생성 및 바인딩
    glGenVertexArrays(1, &a_axis);
    glBindVertexArray(a_axis);

    // 축 VBO 생성, 바인딩 및 데이터 업로드
    glGenBuffers(1, &b_axis);
    glBindBuffer(GL_ARRAY_BUFFER, b_axis);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

    // 정점 속성 설정 (위치)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(0);

    // 정점 속성 설정 (색상)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLvoid SpecialKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        dy += 0.1f;
        break;

    case GLUT_KEY_DOWN:
        dy -= 0.1f;
        break;
    case GLUT_KEY_LEFT:
        dx -= 0.1f;
        break;
    case GLUT_KEY_RIGHT:
        dx += 0.1f;
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

GLvoid timer(int value) {
    if (isSpinning) {
        axis_dy += 5.0f;
    }
    if (showTopFace) {
        face_rotatey[2] += 2.0f;
    }
    if (showBackFace) {
        if (face_scale[0] > 0.0f) {
            face_scale[0] -= 0.1f;
        }
    }
    else {
        if (face_scale[0] < 1.0f) {
            face_scale[0] += 0.1f;
        }
    }

    if (showSideFace) {
        if (face_dy[1] < 0.5f && face_dy[3] < 0.5f) {
            face_dy[1] += 0.1f;
            face_dy[3] += 0.1f;
        }
    }
    else {
        if (face_dy[1] > 0.1f && face_dy[3] > 0.1f) {
            face_dy[1] -= 0.1f;
            face_dy[3] -= 0.1f;
        }
    }
    if (showFrontFace) {
        for_face_rotate_dx = -0.25f;
        for_face_rotate_dy = 0.25f;
        if (face_rotatex[5] < 90.0f) {
            face_rotatex[5] += 5.0f;
        }
    }
    else {
        for_face_rotate_dx = -0.25f;
        for_face_rotate_dy = 0.25f;
        if (face_rotatex[5] > 0.0f) {
            face_rotatex[5] -= 5.0f;
        }
    }

    if (pyramid1_flag) {
        if (pyramid_rotate_z[0] < 233 && pyramid_rotate_x[1] < 233 && pyramid_rotate_z[2] > -233 && pyramid_rotate_x[3] > -233) {
            pyramid_dx[0] = 0.25f;
            pyramid_dy[0] = 0.25f;
            pyramid_rotate_z[0] += 1.0f;

            pyramid_dz[1] = -0.25f;
            pyramid_dy[1] = 0.25f;
            pyramid_rotate_x[1] += 1.0f;

            pyramid_dx[2] = -0.25f;
            pyramid_dy[2] = 0.25f;
            pyramid_rotate_z[2] -= 1.0f;

            pyramid_dz[3] = 0.25f;
            pyramid_dy[3] = 0.25f;
            pyramid_rotate_x[3] -= 1.0f;
        }
    }
    else if (!pyramid2_flag) {
        if (pyramid_rotate_z[0] > 0 && pyramid_rotate_x[1] > 0 && pyramid_rotate_z[2] < 0 && pyramid_rotate_x[3] < 0) {
            pyramid_rotate_z[0] -= 1.0f;
            pyramid_rotate_x[1] -= 1.0f;
            pyramid_rotate_z[2] += 1.0f;
            pyramid_rotate_x[3] += 1.0f;
        }
    }
    if (pyramid2_flag) {
        if (pyramid_rotate_z[0] < 90) {
            pyramid_dx[0] = 0.25f;
            pyramid_dy[0] = 0.25f;
            pyramid_rotate_z[0] += 1.0f;
        }
        if (pyramid_rotate_z[0] > 85 && pyramid_rotate_x[1] < 90) {
            pyramid_dz[1] = -0.25f;
            pyramid_dy[1] = 0.25f;
            pyramid_rotate_x[1] += 1.0f;
        }
        if (pyramid_rotate_x[1] > 85 && pyramid_rotate_z[2] > -90) {
            pyramid_dx[2] = -0.25f;
            pyramid_dy[2] = 0.25f;
            pyramid_rotate_z[2] -= 1.0f;
        }
        if (pyramid_rotate_z[2] < -85 && pyramid_rotate_x[3] > -90) {
            pyramid_dz[3] = 0.25f;
            pyramid_dy[3] = 0.25f;
            pyramid_rotate_x[3] -= 1.0f;
        }
    }
    else if (!pyramid1_flag) {
        if (pyramid_rotate_z[0] > 0) {
            pyramid_rotate_z[0] -= 1.0f;
        }
        if (pyramid_rotate_x[1] > 0) {
            pyramid_rotate_x[1] -= 1.0f;
        }
        if (pyramid_rotate_z[2] < 0) {
            pyramid_rotate_z[2] += 1.0f;
        }
        if (pyramid_rotate_x[3] < 0) {
            pyramid_rotate_x[3] += 1.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);
}
