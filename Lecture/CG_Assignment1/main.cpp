#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "open_file.h" // 쉐이더 파일 로딩 함수 포함

#define WINDOWX 800
#define WINDOWY 800

using namespace std;

// 랜덤 엔진 및 분포 설정
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> uid(0.0f, 1.0f);
uniform_real_distribution<float> uid_lean(0.2f, 1.0f);
uniform_int_distribution<int> uidd(0, 3);
uniform_int_distribution<int> uid_x(0, 1);

// 구조체 정의
struct Point {
    glm::vec3 xyz;
};

struct Color {
    GLfloat r, g, b;
};

struct Shape {
    int vertex_num; // 도형 꼭짓점 개수
    int s; // 도형 종류 (삼각형, 사각형 등)
    vector<Point> point; // 꼭짓점 벡터
    vector<Color> color; // 도형 색
    float x, y; // 도형 중심 좌표
    float tx, ty; // 도형 이동 거리
    float rotate_r; // 자전 각도
    float lean; // 이동 경로 기울기
    bool split; // 도형 분할 여부
    bool on_box; // 바구니에 담겼는지 여부
    bool draw_route; // 경로 출력 여부

    void bind();
    void draw();
};

// 함수 선언 (Shape 정의 이후)
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void InitBuffer();
void TimerFunc(int value);
GLvoid Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void make_newShape();
void split_shape(Shape s);
void Collide_shape(Shape s);
bool collide_box(Shape s);
bool crossing_vertex();

// 쉐이더 소스 및 객체
GLchar* vertexSource = nullptr;
GLchar* fragmentSource = nullptr;
GLuint vertexShader, fragmentShader;
GLuint shaderProgramID;
GLuint VAO, VBO[2];
glm::mat4 TR = glm::mat4(1.0f);
glm::vec4 transformedVertex;
glm::vec3 finalVertex;

// 구조체 인스턴스
Point p;
Color c;

// 도형 그리기 모드
bool shape_mode = true;
bool route_mode = false;

// 전역 변수
vector<Shape> shape;
Shape temp_shape;
bool mouse_click = false;
int timer_speed = 30;

// 타이머 변수 선언 (전역 변수로 선언)
int new_shape_time = 0;

// 라인 및 색상
GLfloat line[2][3];
GLfloat route_line[2][3];
GLfloat line_color[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f
};

// 바구니 정의
GLfloat box[4][3] = {
    {-0.2f, -0.85f, 1.0f},
    {-0.2f, -0.95f, 1.0f},
    {0.2f, -0.95f, 1.0f},
    {0.2f, -0.85f, 1.0f}
};

GLfloat box_color[4][3] = {
    {0.0f, 0.0f, 0.8f},
    {0.0f, 0.0f, 0.8f},
    {0.0f, 0.0f, 0.8f},
    {0.0f, 0.0f, 0.8f}
};
float box_xmove = 0.01f;
float box_x = 0.0f;

// 분할 관련 변수
vector<Point> cross_point;
vector<Point> left_shape;
vector<Point> right_shape;

float X1, X2, X3, X4;
float Y1, Y2, Y3, Y4;
float m1, m2;
int cross_num = 0;

// Shape 멤버 함수 정의
void Shape::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, point.size() * sizeof(Point), point.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(Color), color.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
}

void Shape::draw() {
    glBindVertexArray(VAO);
    if (!shape_mode) {
        glLineWidth(3.0f);
        glPolygonMode(GL_FRONT, GL_LINE);
        glDrawArrays(GL_TRIANGLE_FAN, 0, point.size());
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLE_FAN, 0, point.size());
    }
}

// 교차점 검사 함수
bool crossing_vertex() {
    float cx, cy;
    m1 = (Y2 - Y1) / (X2 - X1);
    m2 = (Y4 - Y3) / (X4 - X3);
    if (m1 == m2) return false;
    cx = (m1 * X1 - m2 * X3 - Y1 + Y3) / (m1 - m2);
    cy = m1 * cx - m1 * X1 + Y1;
    if (((cy >= Y3 && cy <= Y4) || (cy <= Y3 && cy >= Y4))
        && ((cy >= Y1 && cy <= Y2) || (cy <= Y1 && cy >= Y2))) { // 교차 !
        p.xyz = { cx, cy, 1.0f };
        cross_point.push_back(p);
        return true;
    }
    return false;
}

// 도형 충돌 검사 함수
void Collide_shape(Shape s) {
    cross_num = 0;
    cross_point.clear();
    left_shape.clear(); right_shape.clear();
    X1 = line[0][0]; Y1 = line[0][1];
    X2 = line[1][0]; Y2 = line[1][1];

    std::cout << "도형 검사 시작: 도형 종류 = " << s.s << ", 중심 좌표 = (" << s.x << ", " << s.y << ")\n";

    for (size_t i = 0; i < s.point.size(); ++i) {
        X3 = s.point[i].xyz.x; Y3 = s.point[i].xyz.y;
        if (i == s.point.size() - 1) {
            X4 = s.point[0].xyz.x; Y4 = s.point[0].xyz.y;
        }
        else {
            X4 = s.point[i + 1].xyz.x; Y4 = s.point[i + 1].xyz.y;
        }

        if (crossing_vertex()) {
            std::cout << "교차점 발견: (" << p.xyz.x << ", " << p.xyz.y << ")\n";
            if (cross_num == 0) {
                p.xyz = { s.point[i].xyz.x, s.point[i].xyz.y, 1.0f };
                left_shape.push_back(p);
                p.xyz = cross_point[0].xyz;
                left_shape.push_back(p); right_shape.push_back(p);
            }
            else if (cross_num == 1) {
                p.xyz = { s.point[i].xyz.x, s.point[i].xyz.y, 1.0f };
                right_shape.push_back(p);
                p.xyz = cross_point[1].xyz;
                left_shape.push_back(p); right_shape.push_back(p);
            }
            cross_num++;
        }
        else {
            if (cross_num == 0) {
                p.xyz = { s.point[i].xyz.x, s.point[i].xyz.y, 1.0f };
                left_shape.push_back(p);
            }
            else if (cross_num == 1) {
                p.xyz = { s.point[i].xyz.x, s.point[i].xyz.y, 1.0f };
                right_shape.push_back(p);
            }
            else if (cross_num == 2) {
                p.xyz = { s.point[i].xyz.x, s.point[i].xyz.y, 1.0f };
                left_shape.push_back(p);
            }
        }
    }

    std::cout << "도형 검사 완료: 교차점 개수 = " << cross_num << "\n";
}

// 도형 분할 함수
void split_shape(Shape s) {
    std::cout << "도형 분할 시작: 도형 종류 = " << s.s << ", 중심 좌표 = (" << s.x << ", " << s.y << ")\n";

    // 왼쪽 도형 생성
    temp_shape.point.clear(); temp_shape.color.clear();

    temp_shape.vertex_num = static_cast<int>(left_shape.size());

    c.r = s.color[0].r; c.g = s.color[0].g; c.b = s.color[0].b;

    for (size_t i = 0; i < left_shape.size(); ++i) {
        p.xyz = left_shape[i].xyz;
        temp_shape.point.push_back(p);
        temp_shape.color.push_back(c);
    }
    temp_shape.x = s.x; temp_shape.y = s.y;
    temp_shape.tx = -0.005f;
    temp_shape.ty = -0.01f;
    temp_shape.rotate_r = 0.03f;
    temp_shape.split = false; temp_shape.on_box = false; temp_shape.draw_route = false;
    shape.push_back(temp_shape);

    std::cout << "왼쪽 도형 추가됨: vertex_num = " << temp_shape.vertex_num << ", 이동 벡터 = (" << temp_shape.tx << ", " << temp_shape.ty << ")\n";

    // 오른쪽 도형 생성
    temp_shape.point.clear(); temp_shape.color.clear();
    temp_shape.vertex_num = static_cast<int>(right_shape.size());
    c.r = s.color[0].r; c.g = s.color[0].g; c.b = s.color[0].b;
    for (size_t i = 0; i < right_shape.size(); ++i) {
        p.xyz = right_shape[i].xyz;
        temp_shape.point.push_back(p);
        temp_shape.color.push_back(c);
    }
    temp_shape.x = s.x; temp_shape.y = s.y;
    temp_shape.tx = 0.005f;
    temp_shape.ty = -0.01f;
    temp_shape.rotate_r = 0.03f;
    temp_shape.split = false; temp_shape.on_box = false; temp_shape.draw_route = false;
    shape.push_back(temp_shape);

    std::cout << "오른쪽 도형 추가됨: vertex_num = " << temp_shape.vertex_num << ", 이동 벡터 = (" << temp_shape.tx << ", " << temp_shape.ty << ")\n";

    // 파티클 생성 코드 제거
    // make_particle(s.x, s.y, s.color[0].r, s.color[0].g, s.color[0].b);

    std::cout << "도형 분할 완료: 새로운 도형 두 개 추가됨\n";
}

// 바구니와 도형 충돌 검사 함수
bool collide_box(Shape s) {
    if (s.x >= box[0][0] && s.x <= box[3][0]) {
        for (size_t i = 0; i < s.point.size(); ++i) {
            if (s.point[i].xyz.y <= box[0][1]) return true;
        }
    }
    return false;
}

// 새로운 도형 생성 함수
void make_newShape() {
    temp_shape.s = uidd(dre); // 0부터 3까지의 랜덤 정수 선택
    temp_shape.vertex_num = 0;
    temp_shape.point.clear(); temp_shape.color.clear();
    if (temp_shape.s == 0) { // 삼각형
        p.xyz = { 0.0f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;

        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 3; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 1) { // 사각형
        p.xyz = { -0.5f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 4; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 2) { // 오각형
        p.xyz = { 0.0f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, 0.15f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.3f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.3f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, 0.15f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 5; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 3) { // 육각형
        p.xyz = { -0.25f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.55f, 0.0f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.25f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.25f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.55f, 0.0f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.25f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 6; ++i) temp_shape.color.push_back(c);
    }
    temp_shape.rotate_r = 0.03f;
    temp_shape.split = false;
    temp_shape.on_box = false;
    temp_shape.draw_route = true;
    int rand_x = uid_x(dre);
    if (rand_x == 0) {
        temp_shape.x = 1.2f;
        temp_shape.lean = -uid_lean(dre);
        temp_shape.tx = -0.01f;
        temp_shape.ty = temp_shape.lean * temp_shape.tx;
    }
    else if (rand_x == 1) {
        temp_shape.x = -1.2f;
        temp_shape.lean = uid_lean(dre);
        temp_shape.tx = 0.01f;
        temp_shape.ty = temp_shape.lean * temp_shape.tx;
    }
    temp_shape.y = 0.0f;

    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(temp_shape.x, temp_shape.y, 0.0f));
    TR = glm::scale(TR, glm::vec3(0.3f, 0.3f, 0.3f));

    for (size_t i = 0; i < temp_shape.point.size(); ++i) {
        transformedVertex = TR * glm::vec4(temp_shape.point[i].xyz, 1.0f);
        temp_shape.point[i].xyz = glm::vec3(transformedVertex);
    }

    shape.push_back(temp_shape);

    std::cout << "새로운 도형 생성: 도형 종류 = " << temp_shape.s << ", 중심 좌표 = (" << temp_shape.x << ", " << temp_shape.y << ")\n";
}

// 버텍스 쉐이더 생성 함수
void make_vertexShaders()
{
    vertexSource = loadShaderSource("vertex.glsl"); // 쉐이더 파일 로딩
    if (!vertexSource) {
        exit(EXIT_FAILURE); // 쉐이더 로드 실패 시 종료
    }

    // 버텍스 세이더 객체 생성
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // 쉐이더 코드 소스 설정
    const GLchar* vShaderSource[] = { vertexSource };
    glShaderSource(vertexShader, 1, vShaderSource, NULL);

    // 버텍스 세이더 컴파일
    glCompileShader(vertexShader);

    // 컴파일 상태 확인
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        free(vertexSource); // 메모리 해제
        exit(EXIT_FAILURE);
    }

    free(vertexSource); // 컴파일 후 메모리 해제
}

// 프래그먼트 쉐이더 생성 함수
void make_fragmentShaders()
{
    fragmentSource = loadShaderSource("fragment.glsl"); // 쉐이더 파일 로딩
    if (!fragmentSource) {
        exit(EXIT_FAILURE); // 쉐이더 로드 실패 시 종료
    }

    // 프래그먼트 세이더 객체 생성
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // 쉐이더 코드 소스 설정
    const GLchar* fShaderSource[] = { fragmentSource };
    glShaderSource(fragmentShader, 1, fShaderSource, NULL);

    // 프래그먼트 세이더 컴파일
    glCompileShader(fragmentShader);

    // 컴파일 상태 확인
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
        free(fragmentSource); // 메모리 해제
        exit(EXIT_FAILURE);
    }

    free(fragmentSource); // 컴파일 후 메모리 해제
}

// 쉐이더 프로그램 생성 함수
void make_shaderProgram()
{
    make_vertexShaders(); // 버텍스 세이더 생성 및 컴파일
    make_fragmentShaders(); // 프래그먼트 세이더 생성 및 컴파일

    // 쉐이더 프로그램 생성
    shaderProgramID = glCreateProgram();

    // 세이더 프로그램에 세이더 첨부
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    // 링크 상태 확인
    GLint success;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgramID, 512, NULL, infoLog);
        std::cout << "ERROR: Shader program 링크 실패\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }

    // 세이더 삭제 (프로그램에 이미 첨부되었으므로)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 쉐이더 프로그램 사용
    glUseProgram(shaderProgramID);
}

// 버퍼 초기화 함수
void InitBuffer()
{
    glGenVertexArrays(1, &VAO); // VAO 생성
    glBindVertexArray(VAO); // VAO 바인딩
    glGenBuffers(2, VBO); // VBO 생성
}

// 타이머 함수
void TimerFunc(int value)
{
    // 새로운 도형 생성
    new_shape_time++;
    if (new_shape_time >= 100) {
        make_newShape();
        new_shape_time = 0;
    }

    // 도형 이동
    for (size_t i = 0; i < shape.size(); ++i) {
        if (shape[i].on_box) shape[i].tx = box_xmove;

        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(shape[i].x, shape[i].y, 0.0f));
        TR = glm::rotate(TR, shape[i].rotate_r, glm::vec3(0.0f, 0.0f, 1.0f));
        TR = glm::translate(TR, glm::vec3(-shape[i].x, -shape[i].y, 0.0f));

        TR = glm::translate(TR, glm::vec3(shape[i].tx, shape[i].ty, 0.0f));
        shape[i].x += shape[i].tx;
        shape[i].y += shape[i].ty;
        for (size_t j = 0; j < shape[i].point.size(); ++j) {
            transformedVertex = TR * glm::vec4(shape[i].point[j].xyz, 1.0f);
            shape[i].point[j].xyz = glm::vec3(transformedVertex);
        }
    }

    // 바구니 이동
    TR = glm::mat4(1.0f);
    TR = glm::translate(TR, glm::vec3(box_xmove, 0.0f, 0.0f));
    box_x += box_xmove;
    if (box_x >= 0.75f) box_xmove = -box_xmove;
    else if (box_x <= -0.75f) box_xmove = -box_xmove;
    for (int i = 0; i < 4; ++i) {
        transformedVertex = TR * glm::vec4(box[i][0], box[i][1], 1.0f, 1.0f);
        p.xyz = glm::vec3(transformedVertex);
        box[i][0] = p.xyz.x; box[i][1] = p.xyz.y;
    }

    // 바구니와 도형 충돌 처리
    for (size_t i = 0; i < shape.size(); ++i) {
        if (collide_box(shape[i])) {
            shape[i].tx = box_xmove; shape[i].ty = 0.0f;
            shape[i].rotate_r = 0.0f;
        }
    }

    // 범위를 벗어나거나 쪼개진 도형 삭제
    for (size_t i = 0; i < shape.size();) {
        if (shape[i].x <= -1.3f || shape[i].x >= 1.3f || shape[i].split) {
            std::cout << "도형 삭제: 인덱스 " << i << "\n";
            shape.erase(shape.begin() + i);
        }
        else {
            ++i;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(timer_speed, TimerFunc, 1);
}

// 키보드 입력 처리 함수
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'l': // LINE/FILL 모드 전환
        shape_mode = !shape_mode;
        std::cout << "모드 전환: " << (shape_mode ? "FILL" : "LINE") << "\n";
        break;
    case 'r': // 경로 출력 on/off 전환
        route_mode = !route_mode;
        std::cout << "경로 출력 " << (route_mode ? "ON" : "OFF") << "\n";
        break;
    case '+':
        if (timer_speed > 5) timer_speed--;
        std::cout << "타이머 속도 증가: " << timer_speed << "ms\n";
        break;
    case '-':
        timer_speed++;
        std::cout << "타이머 속도 감소: " << timer_speed << "ms\n";
        break;
    case 'q':
        exit(EXIT_SUCCESS); // 프로그램 종료
        break;
    }
    glutPostRedisplay();
}

double mx, my;
int shape_size = 0;

// 마우스 클릭 처리 함수
void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouse_click = true;
        line[0][0] = static_cast<GLfloat>(mx);
        line[0][1] = static_cast<GLfloat>(my);
        line[0][2] = 1.0f;
        line[1][0] = static_cast<GLfloat>(mx);
        line[1][1] = static_cast<GLfloat>(my);
        line[1][2] = 1.0f;
        std::cout << "마우스 클릭: 슬라이스 시작점 = (" << mx << ", " << my << ")\n";
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mouse_click = false;
        std::cout << "마우스 버튼 해제: 슬라이스 종료점 = (" << line[1][0] << ", " << line[1][1] << ")\n";
        shape_size = static_cast<int>(shape.size());
        for (int i = 0; i < shape_size; ++i) {
            if (!shape[i].split) {
                Collide_shape(shape[i]);
                if (cross_num == 2) {
                    split_shape(shape[i]);
                    shape[i].split = true; // 원래 도형의 split 플래그 설정
                    std::cout << "도형 분할 처리 완료: 인덱스 " << i << "\n";
                }
                else {
                    std::cout << "도형 분할 실패: 교차점 개수 = " << cross_num << "\n";
                }
            }
        }
    }
}

// 마우스 움직임 처리 함수
void Motion(int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));
    if (mouse_click) {
        line[1][0] = static_cast<GLfloat>(mx);
        line[1][1] = static_cast<GLfloat>(my);
        line[1][2] = 1.0f;
        std::cout << "슬라이스 라인 업데이트: (" << mx << ", " << my << ")\n";
    }
}

// 그리기 콜백 함수
GLvoid drawScene()
{
    glClearColor(0.69f, 0.83f, 0.94f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 배경

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO); // 쉐이더, 버퍼 배열 사용

    // 도형 및 경로 그리기
    for (size_t i = 0; i < shape.size(); ++i) {
        if (route_mode && shape[i].draw_route && !shape[i].split) {
            route_line[0][0] = shape[i].x; route_line[0][1] = shape[i].y;
            if (shape[i].tx >= 0.0f) {
                route_line[1][0] = shape[i].x + 2.0f;
                route_line[1][1] = shape[i].y + 2.0f * shape[i].lean;
            }
            else {
                route_line[1][0] = shape[i].x - 2.0f;
                route_line[1][1] = shape[i].y - 2.0f * shape[i].lean;
            }
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), route_line, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), line_color, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glBindVertexArray(VAO);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, 2);
        }

        if (!shape[i].split) {
            shape[i].bind();
            shape[i].draw();
        }
    }
    // 슬라이스 라인 그리기
    if (mouse_click) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), line, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), line_color, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(VAO);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }

    // 파티클 그리기 코드 제거
    /*
    for (size_t i = 0; i < particle.size(); ++i) {
        particle[i].bind();
        particle[i].draw();
    }
    */

    // 바구니 그리기
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), box, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), box_color, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glutSwapBuffers(); // 화면에 출력
}

// 창 크기 변경 콜백 함수
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

// 메인 함수
int main(int argc, char** argv)
{
    // 윈도우 생성
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WINDOWX, WINDOWY);
    glutCreateWindow("Let's SP!");

    // GLEW 초기화
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cout << "GLEW 초기화 실패: " << glewGetErrorString(glewStatus) << std::endl;
        return EXIT_FAILURE;
    }

    make_shaderProgram();
    InitBuffer();
    make_newShape();
    glutTimerFunc(timer_speed, TimerFunc, 1);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);

    glutMainLoop();

    return EXIT_SUCCESS;
}
