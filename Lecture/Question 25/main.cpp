#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <fstream>

#define WINDOWX 800
#define WINDOWY 800

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0.0f, 1.0f);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();
GLchar* filetobuf(const char* file);
void LoadOBJ(const char* filename, vector<glm::vec3>& out_vertex, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals);

GLuint shaderID;
GLint width, height;

GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO[2];

class Plane {
public:
    vector<glm::vec3> vertex;
    vector<glm::vec3> color;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(glm::vec3), vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertex.size());
    }
};
Plane sphere;
float sphere_x[3] = { 0.0, -0.3f, -0.5f };
float sphere_scale[3] = { 0.0003, 0.0002, 0.0001 };
float sphere_color[3][3] = {
    {0.941, 0.133, 0.133},
    {0.133, 0.941, 0.172},
    {0.133, 0.172, 0.941}
};

float BackGround[] = { 0.0, 0.0, 0.0 };

glm::vec4* vertex;
glm::vec4* face;
glm::vec3* outColor;

glm::mat4 TR = glm::mat4(1.0f);

FILE* FL;
int faceNum = 0;
bool start = true;

void ReadObj(FILE* objFile);
void keyboard(unsigned char, int, int);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void make_vertexShaders()
{

    GLchar* vertexShaderSource;

    vertexShaderSource = filetobuf("vertex.glsl");

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    const GLchar* fragmentShaderSource = filetobuf("fragment.glsl");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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
    ShaderProgramID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(ShaderProgramID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(ShaderProgramID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(ShaderProgramID); //--- 세이더 프로그램 링크하기

    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
        exit(-1);
    }
    glUseProgram(ShaderProgramID); //--- 만들어진 세이더 프로그램 사용하기
    //--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
    //--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
    //--- 사용하기 직전에 호출할 수 있다.
    return ShaderProgramID;
}
void InitShader()
{
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderID = make_shaderProgram(); //--- 세이더 프로그램 만들기
}
GLvoid InitBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glBindVertexArray(VAO);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv); // glut 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
    glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
    glutInitWindowSize(WINDOWX, WINDOWY); // 윈도우의 크기 지정
    glutCreateWindow("실습 26");// 윈도우 생성   (윈도우 이름)
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, TimerFunction, 1);
    glutDisplayFunc(drawScene); //--- 출력 콜백 함수
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutMainLoop();
}
float camera_z = 5.0f, camera_x = 0.0f;
int camera_rotate = 0;
float camera_rotate_y = 0.0f;
float rotate_y = 0.0f;

float light_x = 0.0f, light_z = 2.0f;
float light_angle = 0.0;
bool light_cw_move = false, light_ccw_move = false;

// 변경된 부분: 조명 색상 변수 이름 및 초기값 수정
bool basic_color = true, skyblue_color = false, pink_color = false;

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수 
{
    if (start) {
        start = FALSE;
        LoadOBJ("sphere.obj", sphere.vertex, sphere.uvs, sphere.normals);
        glEnable(GL_DEPTH_TEST);

    } // 초기화할 데이터

    glViewport(0, 0, 800, 800);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
    unsigned int projLocation = glGetUniformLocation(shaderID, "projection");


    unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos");
    unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor");
    unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor");
    glm::mat4 Vw = glm::mat4(1.0f);
    glm::mat4 Pj = glm::mat4(1.0f);
    glm::vec3 cameraPos;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;

    cameraPos = glm::vec4(0.0f, 0.0f, 2.0f, 1.0f); //--- 카메라 위치
    cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    Vw = glm::mat4(1.0f);
    Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);

    glUniform3f(lightPosLocation, light_x, 0.0f, light_z); //--- lightPos 값 전달: (0.0, 0.0, 5.0);

    // 변경된 부분: 조명 색상 설정
    if (basic_color)
        glUniform3f(lightColorLocation, 1.0, 1.0, 1.0); // 기본 색상 (백색)
    else if (skyblue_color)
        glUniform3f(lightColorLocation, 0.529, 0.808, 0.922); // 하늘색
    else if (pink_color)
        glUniform3f(lightColorLocation, 1.0, 0.753, 0.796); // 분홍색
    else
        glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);

    //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
   // 그리기 코드 
   // 부모행렬 -> 공전 -> 이동 -> 자전 -> 스케일
    for (int i = 0; i < 3; ++i) {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(sphere_x[i], 0.0f, 0.0f));
        TR = glm::scale(TR, glm::vec3(sphere_scale[i], sphere_scale[i], sphere_scale[i]));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        glUniform3f(objColorLocation, sphere_color[i][0], sphere_color[i][1], sphere_color[i][2]); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색

        sphere.Bind();
        sphere.Draw();
    }


    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{

}

// 변경된 부분: keyboard 함수 수정
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        basic_color = true; skyblue_color = false; pink_color = false;
        break;
    case '2':
        basic_color = false; skyblue_color = true; pink_color = false;
        break;
    case '3':
        basic_color = false; skyblue_color = false; pink_color = true;
        break;
    case 'c':
    case 'C': { // 'c' 또는 'C' 키 모두 처리
        // 1, 2, 3 중 랜덤 선택
        uniform_int_distribution<int> dist(1, 3);
        int random_choice = dist(dre);
        switch (random_choice) {
        case 1:
            basic_color = true; skyblue_color = false; pink_color = false;
            cout << "랜덤 선택: 1 (기본 색상)" << endl;
            break;
        case 2:
            basic_color = false; skyblue_color = true; pink_color = false;
            cout << "랜덤 선택: 2 (하늘색)" << endl;
            break;
        case 3:
            basic_color = false; skyblue_color = false; pink_color = true;
            cout << "랜덤 선택: 3 (분홍색)" << endl;
            break;
        }
        break;
    }
    case 'r':
        if (light_cw_move) {
            light_cw_move = false;
        }
        else {
            light_cw_move = true; light_ccw_move = false;
        }
        break;
    case 'R':
        if (light_ccw_move) {
            light_ccw_move = false;
        }
        else {
            light_cw_move = false; light_ccw_move = true;
        }
        break;
    case 'q':
        glutDestroyWindow(1);
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    if (light_cw_move) {
        light_angle += 0.01f;
    }
    else if (light_ccw_move) {
        light_angle -= 0.01f;
    }
    light_x = 2.0 * sin(light_angle);
    light_z = 2.0 * cos(light_angle);

    glutPostRedisplay();
    glutTimerFunc(10, TimerFunction, 1);
}

double mx, my;
void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));


}
void Motion(int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));

}

GLchar* filetobuf(const char* file) {
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

void LoadOBJ(const char* filename, vector<glm::vec3>& out_vertex, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals)
{
    vector<int> vertexindices, uvindices, normalindices;
    vector<GLfloat> temp_vertex;
    vector<GLfloat> temp_uvs;
    vector<GLfloat> temp_normals;
    ifstream in(filename, ios::in);
    if (in.fail()) {
        cout << "Impossible to open file" << endl;
        return;
    }
    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") {
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            temp_vertex.push_back(vertex.x);
            temp_vertex.push_back(vertex.y);
            temp_vertex.push_back(vertex.z);
        }
        else if (lineHeader == "vt") {
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            temp_uvs.push_back(uv.x);
            temp_uvs.push_back(uv.y);
        }
        else if (lineHeader == "vn") {
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal.x);
            temp_normals.push_back(normal.y);
            temp_normals.push_back(normal.z);
        }
        else if (lineHeader == "f") {
            string vertex1, vertex2, vertex3;
            unsigned int vertexindex[3], uvindex[3], normalindex[3];
            for (int k = 0; k < 3; ++k) {
                string temp, temp2;
                int cnt{ 0 }, cnt2{ 0 };
                in >> temp;
                while (1) {
                    while ((int)temp[cnt] != 47 && cnt < temp.size()) {
                        temp2 += (int)temp[cnt];
                        cnt++;
                    }
                    if ((int)temp[cnt] == 47 && cnt2 == 0) {
                        vertexindex[k] = atoi(temp2.c_str());
                        vertexindices.push_back(vertexindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if ((int)temp[cnt] == 47 && cnt2 == 1) {
                        uvindex[k] = atoi(temp2.c_str());
                        uvindices.push_back(uvindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if (temp[cnt] = '\n' && cnt2 == 2) {
                        normalindex[k] = atoi(temp2.c_str());
                        normalindices.push_back(normalindex[k]);
                        break;
                    }
                }
            }
        }
        else {
            continue;
        }
    }
    for (int i = 0; i < vertexindices.size(); ++i) {
        unsigned int vertexIndex = vertexindices[i];
        vertexIndex = (vertexIndex - 1) * 3;
        glm::vec3 vertex = { temp_vertex[vertexIndex], temp_vertex[vertexIndex + 1], temp_vertex[vertexIndex + 2] };
        out_vertex.push_back(vertex);
    }
    for (unsigned int i = 0; i < uvindices.size(); ++i) {
        unsigned int uvIndex = uvindices[i];
        uvIndex = (uvIndex - 1) * 2;
        glm::vec2 uv = { temp_uvs[uvIndex], temp_uvs[uvIndex + 1] };
        out_uvs.push_back(uv);
    }
    for (unsigned int i = 0; i < normalindices.size(); ++i) {
        unsigned int normalIndex = normalindices[i];
        normalIndex = (normalIndex - 1) * 3;
        glm::vec3 normal = { temp_normals[normalIndex], temp_normals[normalIndex + 1], temp_normals[normalIndex + 2] };
        out_normals.push_back(normal);
    }
}
