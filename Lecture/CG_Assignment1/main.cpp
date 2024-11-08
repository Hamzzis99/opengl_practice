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
#include "open_file.h" // ���̴� ���� �ε� �Լ� ����

#define WINDOWX 800
#define WINDOWY 800

using namespace std;

// ���� ���� �� ���� ����
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> uid(0.0f, 1.0f);
uniform_real_distribution<float> uid_lean(0.2f, 1.0f);
uniform_int_distribution<int> uidd(0, 3);
uniform_int_distribution<int> uid_x(0, 1);

// ����ü ����
struct Point {
    glm::vec3 xyz;
};

struct Color {
    GLfloat r, g, b;
};

struct Shape {
    int vertex_num; // ���� ������ ����
    int s; // ���� ���� (�ﰢ��, �簢�� ��)
    vector<Point> point; // ������ ����
    vector<Color> color; // ���� ��
    float x, y; // ���� �߽� ��ǥ
    float tx, ty; // ���� �̵� �Ÿ�
    float rotate_r; // ���� ����
    float lean; // �̵� ��� ����
    bool split; // ���� ���� ����
    bool on_box; // �ٱ��Ͽ� ������ ����
    bool draw_route; // ��� ��� ����

    void bind();
    void draw();
};

// �Լ� ���� (Shape ���� ����)
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

// ���̴� �ҽ� �� ��ü
GLchar* vertexSource = nullptr;
GLchar* fragmentSource = nullptr;
GLuint vertexShader, fragmentShader;
GLuint shaderProgramID;
GLuint VAO, VBO[2];
glm::mat4 TR = glm::mat4(1.0f);
glm::vec4 transformedVertex;
glm::vec3 finalVertex;

// ����ü �ν��Ͻ�
Point p;
Color c;

// ���� �׸��� ���
bool shape_mode = true;
bool route_mode = false;

// ���� ����
vector<Shape> shape;
Shape temp_shape;
bool mouse_click = false;
int timer_speed = 30;

// Ÿ�̸� ���� ���� (���� ������ ����)
int new_shape_time = 0;

// ���� �� ����
GLfloat line[2][3];
GLfloat route_line[2][3];
GLfloat line_color[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f
};

// �ٱ��� ����
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

// ���� ���� ����
vector<Point> cross_point;
vector<Point> left_shape;
vector<Point> right_shape;

float X1, X2, X3, X4;
float Y1, Y2, Y3, Y4;
float m1, m2;
int cross_num = 0;

// Shape ��� �Լ� ����
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

// ������ �˻� �Լ�
bool crossing_vertex() {
    float cx, cy;
    m1 = (Y2 - Y1) / (X2 - X1);
    m2 = (Y4 - Y3) / (X4 - X3);
    if (m1 == m2) return false;
    cx = (m1 * X1 - m2 * X3 - Y1 + Y3) / (m1 - m2);
    cy = m1 * cx - m1 * X1 + Y1;
    if (((cy >= Y3 && cy <= Y4) || (cy <= Y3 && cy >= Y4))
        && ((cy >= Y1 && cy <= Y2) || (cy <= Y1 && cy >= Y2))) { // ���� !
        p.xyz = { cx, cy, 1.0f };
        cross_point.push_back(p);
        return true;
    }
    return false;
}

// ���� �浹 �˻� �Լ�
void Collide_shape(Shape s) {
    cross_num = 0;
    cross_point.clear();
    left_shape.clear(); right_shape.clear();
    X1 = line[0][0]; Y1 = line[0][1];
    X2 = line[1][0]; Y2 = line[1][1];

    std::cout << "���� �˻� ����: ���� ���� = " << s.s << ", �߽� ��ǥ = (" << s.x << ", " << s.y << ")\n";

    for (size_t i = 0; i < s.point.size(); ++i) {
        X3 = s.point[i].xyz.x; Y3 = s.point[i].xyz.y;
        if (i == s.point.size() - 1) {
            X4 = s.point[0].xyz.x; Y4 = s.point[0].xyz.y;
        }
        else {
            X4 = s.point[i + 1].xyz.x; Y4 = s.point[i + 1].xyz.y;
        }

        if (crossing_vertex()) {
            std::cout << "������ �߰�: (" << p.xyz.x << ", " << p.xyz.y << ")\n";
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

    std::cout << "���� �˻� �Ϸ�: ������ ���� = " << cross_num << "\n";
}

// ���� ���� �Լ�
void split_shape(Shape s) {
    std::cout << "���� ���� ����: ���� ���� = " << s.s << ", �߽� ��ǥ = (" << s.x << ", " << s.y << ")\n";

    // ���� ���� ����
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

    std::cout << "���� ���� �߰���: vertex_num = " << temp_shape.vertex_num << ", �̵� ���� = (" << temp_shape.tx << ", " << temp_shape.ty << ")\n";

    // ������ ���� ����
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

    std::cout << "������ ���� �߰���: vertex_num = " << temp_shape.vertex_num << ", �̵� ���� = (" << temp_shape.tx << ", " << temp_shape.ty << ")\n";

    // ��ƼŬ ���� �ڵ� ����
    // make_particle(s.x, s.y, s.color[0].r, s.color[0].g, s.color[0].b);

    std::cout << "���� ���� �Ϸ�: ���ο� ���� �� �� �߰���\n";
}

// �ٱ��Ͽ� ���� �浹 �˻� �Լ�
bool collide_box(Shape s) {
    if (s.x >= box[0][0] && s.x <= box[3][0]) {
        for (size_t i = 0; i < s.point.size(); ++i) {
            if (s.point[i].xyz.y <= box[0][1]) return true;
        }
    }
    return false;
}

// ���ο� ���� ���� �Լ�
void make_newShape() {
    temp_shape.s = uidd(dre); // 0���� 3������ ���� ���� ����
    temp_shape.vertex_num = 0;
    temp_shape.point.clear(); temp_shape.color.clear();
    if (temp_shape.s == 0) { // �ﰢ��
        p.xyz = { 0.0f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;

        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 3; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 1) { // �簢��
        p.xyz = { -0.5f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 4; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 2) { // ������
        p.xyz = { 0.0f, 0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.5f, 0.15f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { -0.3f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.3f, -0.5f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        p.xyz = { 0.5f, 0.15f, 1.0f }; temp_shape.point.push_back(p); temp_shape.vertex_num++;
        c.r = uid(dre); c.g = uid(dre); c.b = uid(dre);
        for (int i = 0; i < 5; ++i) temp_shape.color.push_back(c);
    }
    else if (temp_shape.s == 3) { // ������
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

    std::cout << "���ο� ���� ����: ���� ���� = " << temp_shape.s << ", �߽� ��ǥ = (" << temp_shape.x << ", " << temp_shape.y << ")\n";
}

// ���ؽ� ���̴� ���� �Լ�
void make_vertexShaders()
{
    vertexSource = loadShaderSource("vertex.glsl"); // ���̴� ���� �ε�
    if (!vertexSource) {
        exit(EXIT_FAILURE); // ���̴� �ε� ���� �� ����
    }

    // ���ؽ� ���̴� ��ü ����
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // ���̴� �ڵ� �ҽ� ����
    const GLchar* vShaderSource[] = { vertexSource };
    glShaderSource(vertexShader, 1, vShaderSource, NULL);

    // ���ؽ� ���̴� ������
    glCompileShader(vertexShader);

    // ������ ���� Ȯ��
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cout << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
        free(vertexSource); // �޸� ����
        exit(EXIT_FAILURE);
    }

    free(vertexSource); // ������ �� �޸� ����
}

// �����׸�Ʈ ���̴� ���� �Լ�
void make_fragmentShaders()
{
    fragmentSource = loadShaderSource("fragment.glsl"); // ���̴� ���� �ε�
    if (!fragmentSource) {
        exit(EXIT_FAILURE); // ���̴� �ε� ���� �� ����
    }

    // �����׸�Ʈ ���̴� ��ü ����
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // ���̴� �ڵ� �ҽ� ����
    const GLchar* fShaderSource[] = { fragmentSource };
    glShaderSource(fragmentShader, 1, fShaderSource, NULL);

    // �����׸�Ʈ ���̴� ������
    glCompileShader(fragmentShader);

    // ������ ���� Ȯ��
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cout << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
        free(fragmentSource); // �޸� ����
        exit(EXIT_FAILURE);
    }

    free(fragmentSource); // ������ �� �޸� ����
}

// ���̴� ���α׷� ���� �Լ�
void make_shaderProgram()
{
    make_vertexShaders(); // ���ؽ� ���̴� ���� �� ������
    make_fragmentShaders(); // �����׸�Ʈ ���̴� ���� �� ������

    // ���̴� ���α׷� ����
    shaderProgramID = glCreateProgram();

    // ���̴� ���α׷��� ���̴� ÷��
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    // ��ũ ���� Ȯ��
    GLint success;
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgramID, 512, NULL, infoLog);
        std::cout << "ERROR: Shader program ��ũ ����\n" << infoLog << std::endl;
        exit(EXIT_FAILURE);
    }

    // ���̴� ���� (���α׷��� �̹� ÷�εǾ����Ƿ�)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ���̴� ���α׷� ���
    glUseProgram(shaderProgramID);
}

// ���� �ʱ�ȭ �Լ�
void InitBuffer()
{
    glGenVertexArrays(1, &VAO); // VAO ����
    glBindVertexArray(VAO); // VAO ���ε�
    glGenBuffers(2, VBO); // VBO ����
}

// Ÿ�̸� �Լ�
void TimerFunc(int value)
{
    // ���ο� ���� ����
    new_shape_time++;
    if (new_shape_time >= 100) {
        make_newShape();
        new_shape_time = 0;
    }

    // ���� �̵�
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

    // �ٱ��� �̵�
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

    // �ٱ��Ͽ� ���� �浹 ó��
    for (size_t i = 0; i < shape.size(); ++i) {
        if (collide_box(shape[i])) {
            shape[i].tx = box_xmove; shape[i].ty = 0.0f;
            shape[i].rotate_r = 0.0f;
        }
    }

    // ������ ����ų� �ɰ��� ���� ����
    for (size_t i = 0; i < shape.size();) {
        if (shape[i].x <= -1.3f || shape[i].x >= 1.3f || shape[i].split) {
            std::cout << "���� ����: �ε��� " << i << "\n";
            shape.erase(shape.begin() + i);
        }
        else {
            ++i;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(timer_speed, TimerFunc, 1);
}

// Ű���� �Է� ó�� �Լ�
GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'l': // LINE/FILL ��� ��ȯ
        shape_mode = !shape_mode;
        std::cout << "��� ��ȯ: " << (shape_mode ? "FILL" : "LINE") << "\n";
        break;
    case 'r': // ��� ��� on/off ��ȯ
        route_mode = !route_mode;
        std::cout << "��� ��� " << (route_mode ? "ON" : "OFF") << "\n";
        break;
    case '+':
        if (timer_speed > 5) timer_speed--;
        std::cout << "Ÿ�̸� �ӵ� ����: " << timer_speed << "ms\n";
        break;
    case '-':
        timer_speed++;
        std::cout << "Ÿ�̸� �ӵ� ����: " << timer_speed << "ms\n";
        break;
    case 'q':
        exit(EXIT_SUCCESS); // ���α׷� ����
        break;
    }
    glutPostRedisplay();
}

double mx, my;
int shape_size = 0;

// ���콺 Ŭ�� ó�� �Լ�
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
        std::cout << "���콺 Ŭ��: �����̽� ������ = (" << mx << ", " << my << ")\n";
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        mouse_click = false;
        std::cout << "���콺 ��ư ����: �����̽� ������ = (" << line[1][0] << ", " << line[1][1] << ")\n";
        shape_size = static_cast<int>(shape.size());
        for (int i = 0; i < shape_size; ++i) {
            if (!shape[i].split) {
                Collide_shape(shape[i]);
                if (cross_num == 2) {
                    split_shape(shape[i]);
                    shape[i].split = true; // ���� ������ split �÷��� ����
                    std::cout << "���� ���� ó�� �Ϸ�: �ε��� " << i << "\n";
                }
                else {
                    std::cout << "���� ���� ����: ������ ���� = " << cross_num << "\n";
                }
            }
        }
    }
}

// ���콺 ������ ó�� �Լ�
void Motion(int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));
    if (mouse_click) {
        line[1][0] = static_cast<GLfloat>(mx);
        line[1][1] = static_cast<GLfloat>(my);
        line[1][2] = 1.0f;
        std::cout << "�����̽� ���� ������Ʈ: (" << mx << ", " << my << ")\n";
    }
}

// �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
    glClearColor(0.69f, 0.83f, 0.94f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ���

    glUseProgram(shaderProgramID);
    glBindVertexArray(VAO); // ���̴�, ���� �迭 ���

    // ���� �� ��� �׸���
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
    // �����̽� ���� �׸���
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

    // ��ƼŬ �׸��� �ڵ� ����
    /*
    for (size_t i = 0; i < particle.size(); ++i) {
        particle[i].bind();
        particle[i].draw();
    }
    */

    // �ٱ��� �׸���
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

    glutSwapBuffers(); // ȭ�鿡 ���
}

// â ũ�� ���� �ݹ� �Լ�
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

// ���� �Լ�
int main(int argc, char** argv)
{
    // ������ ����
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WINDOWX, WINDOWY);
    glutCreateWindow("Let's SP!");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cout << "GLEW �ʱ�ȭ ����: " << glewGetErrorString(glewStatus) << std::endl;
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
