#include<iostream>
#include<gl/glew.h>
#include<gl/freeglut.h>
#include<gl/freeglut_ext.h>
#include <cstdlib>  // rand() �Լ� ����� ���� �߰�
#include <ctime>    // srand()�� time() ����� ���� �߰�

using namespace std;


GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);


GLclampf r = 1.0f, g = 1.0f, b = 1.0f;


bool start_timer = false;

// Ÿ�̸� �Լ�
void Timer(int value) {
    if (start_timer) {
        // rand() �Լ��� ����Ͽ� ���� ���� ����
        r = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
        g = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
        b = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
    }
    // Ÿ�̸Ӹ� �ٽ� ���� (1�� �� ȣ��)
    glutTimerFunc(1000, Timer, 0);
    // ȭ�� ���� ��û
    glutPostRedisplay();
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW initialized\n";
	}
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000, Timer, 0);
	glutMainLoop();

}

GLvoid drawScene(GLvoid) {

	glClearColor(r, g, b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glutSwapBuffers();
}
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);

}
GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'c':
		r = 0.0f;
		g = 1.0f;
		b = 1.0f;
		break;

	case 'm':
		r = 1.0f;
		g = 0.0f;
		b = 1.0f;
		break;

	case 'y':
		r = 1.0f;
		g = 1.0f;
		b = 0.0f;
		break;

	case 'a':
        r = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
        g = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
        b = static_cast<GLclampf>(rand()) / static_cast<GLclampf>(RAND_MAX);
		break;

	case 'w':
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		break;

	case 'k':
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
		break;

	case 't':
		start_timer = true;
		break;

	case 's':
		start_timer = false;
		break;

	case 'q':
		glutLeaveMainLoop();
		break;
	}

	glutPostRedisplay();
}