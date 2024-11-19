#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define WINDOWX 800
#define WINDOWY 600
using namespace std;

#define PI 3.141592

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0, 1);

GLchar* vertexsource, * fragmentsource; //--- 소스코드 저장 변수
GLuint vertexshader, fragmentshader; //--- 세이더 객체

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLUquadricObj* center_qobj;

GLUquadricObj* planet_qobj1;
GLUquadricObj* planet_qobj2;
GLUquadricObj* planet_qobj3;

GLUquadricObj* satellite_qobj1;
GLUquadricObj* satellite_qobj2;
GLUquadricObj* satellite_qobj3;

GLuint VAO, VBO[2];

GLfloat colors[]{
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};

struct Dot {
    float x;
    float y;
    float z;
};
struct Circle {
    vector<Dot>dot;
    float r;
};
Dot new_dot;
Circle circle;
Circle s_circle;

class Line {
public:
    GLfloat p[6];
    GLfloat color[6];

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
    }
};

bool culling = true;

class Plane {
public:
    GLfloat p[9];
    GLfloat color[9];

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glEnable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};

float mx, my, px, py;
float cnt;
int sel;
char comm;

BOOL Time, start = TRUE, TimeLoop;

float BackGround[] = { 1.0, 1.0, 1.0 };
Line xl = {
   0, 1, 0,
   0, -1, 0,

   0, 1, 0,
   0, 1, 0,
};
Line yl = {
   1, 0, 0,
   -1, 0, 0,

   1, 0, 0,
   1, 0, 0,
};
Line zl = {
   0, 0, 1,
   0, 0, -1,

   0, 0, 1,
   0, 0, 1,
};
glm::vec4* vertex;
glm::vec4* face;

Plane* Fvertex[2];

glm::mat4 TR = glm::mat4(1.0f);
glm::mat4 S_TR = glm::mat4(1.0f);
glm::mat4 P_TR1 = glm::mat4(1.0f);
glm::mat4 P_TR2 = glm::mat4(1.0f);
glm::mat4 P_TR3 = glm::mat4(1.0f);
GLUquadricObj* obj;
FILE* FL;
int faceNum = 0;
int Click = 0;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void vectoplane(Plane* p);
void planecolorset(Plane* p, int a);

GLfloat color[3][3]{
    {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 1.0}
};

char* filetobuf(const char* file)
{
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); 
    if (!fptr) 
        return NULL;
    fseek(fptr, 0, SEEK_END); 
    length = ftell(fptr); 
    buf = (char*)malloc(length + 1); 
    fseek(fptr, 0, SEEK_SET); 
    fread(buf, length, 1, fptr); 
    fclose(fptr); 
    buf[length] = 0; 
    return buf; 
}

void make_vertexShaders()
{
    vertexsource = filetobuf("vertex.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexsource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }
}
void make_fragmentShaders()
{
    fragmentsource = filetobuf("fragment.glsl");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
    glCompileShader(fragmentShader);



    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }

}
GLuint make_shaderProgram()
{
    GLint result;
    GLchar errorLog[512];
    GLuint ShaderProgramID;
    ShaderProgramID = glCreateProgram();
    glAttachShader(ShaderProgramID, vertexShader); 
    glAttachShader(ShaderProgramID, fragmentShader);
    glLinkProgram(ShaderProgramID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
        exit(-1);
    }
    glUseProgram(ShaderProgramID); 
    return ShaderProgramID;
}
void InitShader()
{
    make_vertexShaders();
    make_fragmentShaders();
    shaderID = make_shaderProgram();
}
GLvoid InitBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
}

void main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); 
    glutInitWindowPosition(0, 0); 
    glutInitWindowSize(WINDOWX, WINDOWY);
    glutCreateWindow("실습 19");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, TimerFunction, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutMainLoop();
}

bool Projection = true, model_mode = true;
float center_rotate_y = 0.0f, center_x = 0.0f, center_y = 0.0f, center_z = 0.0f;
float planet_rotate_y = 0.0f, planet_rotate_z = 0.0f;
float planet_rotate_1 = 0.0f, planet_x1 = 2.5f, planet_y1 = 0.0f, planet_z1 = 0.0f;
float planet_rotate_2 = 0.0f, planet_x2 = 0.0f, planet_y2 = 0.0f, planet_z2 = 2.5f;
float planet_rotate_3 = 0.0f, planet_x3 = 0.0f, planet_y3 = 0.0f, planet_z3 = -2.5f;
float satellite_rotate_1 = 0.0f, satellite_x1 = planet_x1 + 1.0f, satellite_y1 = planet_y1, satellite_z1 = planet_z1;
float satellite_rotate_2 = 0.0f, satellite_x2 = planet_x2 , satellite_y2 = planet_y2, satellite_z2 = planet_z2 + 1.0f;
float satellite_rotate_3 = 0.0f, satellite_x3 = planet_x3 , satellite_y3 = planet_y3, satellite_z3 = planet_z3 - 1.0f;

GLvoid drawScene() 
{
    if (start) {
        start = FALSE;
        glEnable(GL_DEPTH_TEST);

        circle.r = 1.0f;
        double radian = 0.0f, degree = 0.0f;
        for (int i = 0; i < 360; ++i) {
            radian = i * PI / 100;
            new_dot.x = circle.r * cos(radian);
            new_dot.y = 0.0f;
            new_dot.z = circle.r * sin(radian);
            circle.dot.push_back(new_dot);
        }

        s_circle.r = 3.5f;
        radian = 0.0f, degree = 0.0f;
        for (int i = 0; i < 360; ++i) {
            radian = i * PI / 100;
            new_dot.x = s_circle.r * cos(radian);
            new_dot.y = 0.0f;
            new_dot.z = s_circle.r * sin(radian);
            s_circle.dot.push_back(new_dot);
        }

    } 
    glEnable(GL_DEPTH_TEST);

    glClearColor(BackGround[0], BackGround[1], BackGround[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(shaderID);
    glBindVertexArray(VAO);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TR = glm::mat4(1.0f);

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); 
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); 
    glm::mat4 view = glm::mat4(1.0f);

    view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view"); 
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    glm::mat4 projection = glm::mat4(1.0f);
    if (Projection) {
        projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
        projection = glm::translate(projection, glm::vec3(0.0f, 0.0f, -1.0f));
    }
    else {
        projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
        projection = glm::translate(projection, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    unsigned int projectionLocation = glGetUniformLocation(shaderID, "projection"); 
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

    TR = glm::mat4(1.0f);
    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    unsigned int vColorLocation = glGetUniformLocation(shaderID, "color");

    TR = glm::mat4(1.0f);

    TR = glm::scale(TR, glm::vec3(0.4f, 0.4f, 0.4f));
    TR = glm::rotate(TR, (float)glm::radians(30.0), glm::vec3(1.0f, 0.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(-30.0), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::rotate(TR, (float)glm::radians(center_rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
    TR = glm::translate(TR, glm::vec3(center_x, center_y, center_z));

    S_TR = glm::mat4(1.0f);
    S_TR *= TR;

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    center_qobj = gluNewQuadric();
    if(model_mode)gluQuadricDrawStyle(center_qobj, GLU_FILL);
    else gluQuadricDrawStyle(center_qobj, GLU_LINE);
    gluQuadricNormals(center_qobj, GLU_SMOOTH);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 1.0f);

    gluSphere(center_qobj, 1.0, 20, 20); 

    // 행성 1 궤도
    TR = glm::mat4(1.0f);
    TR = glm::rotate(S_TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::scale(TR, glm::vec3(2.5f, 2.5f, 2.5f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, circle.dot.size() * sizeof(Dot), circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, circle.dot.size());

    // 행성 2 궤도
    TR = glm::mat4(1.0f);
    TR = glm::rotate(S_TR, (float)glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::rotate(TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::scale(TR, glm::vec3(2.5f, 2.5f, 2.5f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, circle.dot.size() * sizeof(Dot), circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, circle.dot.size());

    // 행성 3 궤도
    TR = glm::mat4(1.0f);
    TR = glm::rotate(S_TR, (float)glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::rotate(TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::scale(TR, glm::vec3(2.5f, 2.5f, 2.5f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, circle.dot.size() * sizeof(Dot), circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, circle.dot.size());


    // 행성 1
    TR = glm::mat4(1.0f);

    TR = glm::rotate(S_TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));

    TR = glm::rotate(TR, (float)glm::radians(planet_rotate_1), glm::vec3(0.0f, 1.0f, 0.0f));

    TR = glm::translate(TR, glm::vec3(planet_x1, planet_y1, planet_z1));
    TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));

    P_TR1 = glm::mat4(1.0f);
    P_TR1 = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    planet_qobj1 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(planet_qobj1, GLU_FILL);
    else gluQuadricDrawStyle(planet_qobj1, GLU_LINE);
    gluQuadricNormals(planet_qobj1, GLU_SMOOTH);

    glUniform3f(vColorLocation, 0.0f, 1.0f, 0.0f);
    gluSphere(planet_qobj1, 1.0, 20, 20); // 반지름 1.0

    // 행성 2
    TR = glm::mat4(1.0f);

    TR = glm::rotate(S_TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));

    TR = glm::rotate(TR, (float)glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::rotate(TR, (float)glm::radians(planet_rotate_2), glm::vec3(1.0f, 0.0f, 0.0f));

    TR = glm::translate(TR, glm::vec3(planet_x2, planet_y2, planet_z2));
    TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));

    P_TR2 = glm::mat4(1.0f);
    P_TR2 = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    planet_qobj2 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(planet_qobj2, GLU_FILL);
    else gluQuadricDrawStyle(planet_qobj2, GLU_LINE);
    gluQuadricNormals(planet_qobj2, GLU_SMOOTH);
    glUniform3f(vColorLocation, 0.0f, 1.0f, 0.0f);
    gluSphere(planet_qobj2, 1.0, 20, 20); // 반지름 1.0

    // 행성 3
    TR = glm::mat4(1.0f);

    TR = glm::rotate(S_TR, (float)glm::radians(planet_rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));

    TR = glm::rotate(TR, (float)glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    TR = glm::rotate(TR, (float)glm::radians(planet_rotate_3), glm::vec3(1.0f, 0.0f, 0.0f));

    TR = glm::translate(TR, glm::vec3(planet_x3, planet_y3, planet_z3));
    TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));

    P_TR3 = glm::mat4(1.0f);
    P_TR3 = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    planet_qobj3 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(planet_qobj3, GLU_FILL);
    else gluQuadricDrawStyle(planet_qobj3, GLU_LINE);
    gluQuadricNormals(planet_qobj3, GLU_SMOOTH);
    glUniform3f(vColorLocation, 0.0f, 1.0f, 0.0f);
    gluSphere(planet_qobj3, 1.0, 20, 20); // 반지름 1.0

    // 빨간색위성 궤도

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(P_TR1));

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, s_circle.dot.size() * sizeof(Dot), s_circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, s_circle.dot.size());

    // 빨간색위성 궤도
    TR = glm::mat4(1.0f);
    TR = glm::rotate(P_TR2, (float)glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, s_circle.dot.size() * sizeof(Dot), s_circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, s_circle.dot.size());

    // 빨간색위성 궤도
    TR = glm::mat4(1.0f);
    TR = glm::rotate(P_TR3, (float)glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, s_circle.dot.size() * sizeof(Dot), s_circle.dot.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform3f(vColorLocation, 0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINE_STRIP, 0, s_circle.dot.size());

    // 빨간색위성 1
    P_TR1 = glm::rotate(P_TR1, (float)glm::radians(satellite_rotate_1), glm::vec3(0.0f, 1.0f, 0.0f));

    P_TR1 = glm::translate(P_TR1, glm::vec3(satellite_x1, satellite_y1, satellite_z1));

    P_TR1 = glm::scale(P_TR1, glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(P_TR1));

    satellite_qobj1 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(satellite_qobj1, GLU_FILL);
    else gluQuadricDrawStyle(satellite_qobj1, GLU_LINE);
    gluQuadricNormals(satellite_qobj1, GLU_SMOOTH);
    glUniform3f(vColorLocation, 1.0f, 0.0f, 0.0f);
    gluSphere(satellite_qobj1, 1.0, 20, 20); // 반지름 1.0

    // 빨간색위성 2
    P_TR2 = glm::rotate(P_TR2, (float)glm::radians(satellite_rotate_2), glm::vec3(1.0f, 0.0f, 0.0f));

    P_TR2 = glm::translate(P_TR2, glm::vec3(satellite_x2, satellite_y2, satellite_z2));

    P_TR2 = glm::scale(P_TR2, glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(P_TR2));

    satellite_qobj2 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(satellite_qobj2, GLU_FILL);
    else gluQuadricDrawStyle(satellite_qobj2, GLU_LINE);
    gluQuadricNormals(satellite_qobj2, GLU_SMOOTH);
    glUniform3f(vColorLocation, 1.0f, 0.0f, 0.0f);
    gluSphere(satellite_qobj2, 1.0, 20, 20); // 반지름 1.0

    // 빨간색위성 3
    P_TR3 = glm::rotate(P_TR3, (float)glm::radians(satellite_rotate_3), glm::vec3(1.0f, 0.0f, 0.0f));

    P_TR3 = glm::translate(P_TR3, glm::vec3(satellite_x3, satellite_y3, satellite_z3));

    P_TR3 = glm::scale(P_TR3, glm::vec3(0.5f, 0.5f, 0.5f));

    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(P_TR3));

    satellite_qobj3 = gluNewQuadric();
    if (model_mode)gluQuadricDrawStyle(satellite_qobj3, GLU_FILL);
    else gluQuadricDrawStyle(satellite_qobj3, GLU_LINE);
    gluQuadricNormals(satellite_qobj3, GLU_SMOOTH);
    glUniform3f(vColorLocation, 1.0f, 0.0f, 0.0f);
    gluSphere(satellite_qobj3, 1.0, 20, 20); // 반지름 1.0

    glutSwapBuffers();
    glutPostRedisplay();
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, w, h); //--- 뷰포트 변환
}

bool Rotate_Y_Plus = false, Rotate_Y_Minus = false;
bool Rotate_Z_Plus = false, Rotate_Z_Minus = false;
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p': // 직각 투영
        Projection = false;
        break;
    case 'P': // 원근 투영
        Projection = true;
        break;
    case 'm': // 솔리드 모델
        model_mode = true;
        break;
    case 'M': // 와이어 모델
        model_mode = false;
        break;
    case 'w': // 상
        center_y += 0.1f;
        break;
    case 'a': // 좌
        center_x -= 0.1f;
        break;
    case 's': // 하
        center_y -= 0.1f;
        break;
    case 'd': // 우
        center_x += 0.1f;
        break;
    case '+': // 앞
        center_z += 0.1f;
        break;
    case '-': // 뒤
        center_z -= 0.1f;
        break;
    case 'y': // y축 양의 방향으로 자전
        Rotate_Y_Plus = true; Rotate_Y_Minus = false;
        break;
    case 'Y': // y축의 음의 방향으로 자전
        Rotate_Y_Plus = false; Rotate_Y_Minus = true;
        break;
    case 'z': // 중심 제외 행성, 달, 궤도가 z축에 대하여 양의 방향으로 회전
        Rotate_Z_Plus = true; Rotate_Z_Minus = false;
        break;
    case 'Z': // 음의 방향으로 회전
        Rotate_Z_Plus = false; Rotate_Z_Minus = true;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    planet_rotate_1 += 1.5f; planet_rotate_2 += 1.0f; planet_rotate_3 += 0.5f;
    if (planet_rotate_1 > 360.0)planet_rotate_1 = 0.0f;
    if (planet_rotate_2 > 360.0)planet_rotate_2 = 0.0f;
    if (planet_rotate_3 > 360.0)planet_rotate_3 = 0.0f;

    satellite_rotate_1 += 1.5f; satellite_rotate_2 += 1.0f; satellite_rotate_3 += 0.5f;
    if (satellite_rotate_1 > 360.0)satellite_rotate_1 = 0.0f;
    if (satellite_rotate_2 > 360.0)satellite_rotate_2 = 0.0f;
    if (satellite_rotate_3 > 360.0)satellite_rotate_3 = 0.0f;

    if (Rotate_Y_Plus) {
        center_rotate_y++; planet_rotate_y++;
        if (center_rotate_y > 360.0)center_rotate_y = 0.0f;
        if (planet_rotate_y > 360.0)planet_rotate_y = 0.0f;
    }
    if (Rotate_Y_Minus) {
        center_rotate_y--; planet_rotate_y--;
        if (center_rotate_y < 0.0)center_rotate_y = 360.f;
        if (planet_rotate_y < 0.0)planet_rotate_y = 360.f;
    }
    if (Rotate_Z_Plus) {
        planet_rotate_z++;
        if (planet_rotate_z > 360.0)planet_rotate_z = 0.0f;
    }
    if (Rotate_Z_Minus) {
        planet_rotate_z--;
        if (planet_rotate_z < 0.0)planet_rotate_z = 360.f;
    }

    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

void ReadObj(FILE* objFile)
{
    faceNum = 0;

    char count[8000];
    char bind[8000];
    int vertexNum = 0;
    while (!feof(objFile)) {
        fscanf(objFile, "%s", count);
        if (count[0] == 'v' && count[1] == '\0')
            vertexNum += 1;
        else if (count[0] == 'f' && count[1] == '\0')
            faceNum += 1;
        memset(count, '\0', sizeof(count));
    }
    int vertIndex = 0;
    int faceIndex = 0;
    vertex = (glm::vec4*)malloc(sizeof(glm::vec4) * vertexNum);
    face = (glm::vec4*)malloc(sizeof(glm::vec4) * faceNum);

    fseek(objFile, 0, 0);
    while (!feof(objFile)) {
        fscanf(objFile, "%s", bind);
        if (bind[0] == 'v' && bind[1] == '\0') {
            fscanf(objFile, "%f %f %f",
                &vertex[vertIndex].x, &vertex[vertIndex].y, &vertex[vertIndex].z);
            vertIndex++;
        }
        else if (bind[0] == 'f' && bind[1] == '\0') {
            fscanf(objFile, "%f %f %f",
                &face[faceIndex].x, &face[faceIndex].y, &face[faceIndex].z);
            int x = face[faceIndex].x - 1, y = face[faceIndex].y - 1, z = face[faceIndex].z - 1;
            faceIndex++;
        }
    }
}

void vectoplane(Plane* p) {
    for (int i = 0; i < faceNum; ++i) {
        int x = face[i].x - 1, y = face[i].y - 1, z = face[i].z - 1;
        p[i].p[0] = vertex[x].x;
        p[i].p[1] = vertex[x].y;
        p[i].p[2] = vertex[x].z;

        p[i].p[3] = vertex[y].x;
        p[i].p[4] = vertex[y].y;
        p[i].p[5] = vertex[y].z;

        p[i].p[6] = vertex[z].x;
        p[i].p[7] = vertex[z].y;
        p[i].p[8] = vertex[z].z;
    }
}

void planecolorset(Plane* p, int a) {
    if (a == 0) {
        for (int i = 0; i < faceNum; i += 2) {
            float R = uid(dre);
            float G = uid(dre);
            float B = uid(dre);
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = R;
                p[i].color[j * 3 + 1] = G;
                p[i].color[j * 3 + 2] = B;

                p[i + 1].color[j * 3] = R;
                p[i + 1].color[j * 3 + 1] = G;
                p[i + 1].color[j * 3 + 2] = B;
            }
        }
    }
    else if (a == 1) {
        for (int i = 0; i < faceNum; ++i) {
            float R = uid(dre);
            float G = uid(dre);
            float B = uid(dre);
            for (int j = 0; j < 3; ++j) {
                p[i].color[j * 3] = R;
                p[i].color[j * 3 + 1] = G;
                p[i].color[j * 3 + 2] = B;
            }
        }
    }
}