#include<iostream>
#include<gl/glew.h>
#include<gl/freeglut.h>
#include<gl/freeglut_ext.h>
#include<random>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;

typedef struct random_rect {
	bool is_creat = false;
	GLclampf x1{};
	GLclampf y1{};
	GLclampf x2{};
	GLclampf y2{};
	GLclampf r{};
	GLclampf g{};
	GLclampf b{};
};

std::random_device rd;
std::mt19937 g(rd());

random_rect rectangle_list[41];

void reset();
void merge(int);

bool is_reset = false;

GLclampf erase_width{};
GLclampf erase_heigh{};


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
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	if (not is_reset) {
		reset();
		is_reset = true;
	}
	glutMainLoop();

}


GLvoid drawScene(GLvoid) {

	glClearColor(base_r, base_g, base_b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	for (const random_rect temp : rectangle_list) {
		if (temp.is_creat) {
			glColor3f(temp.r, temp.g, temp.b);
			glRectf(temp.x1, temp.y1, temp.x2, temp.y2);

		}
	}
	glutSwapBuffers();
}
GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);

}
GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		is_reset = false;
		break;

	case 'q':
		glutLeaveMainLoop();
		break;
	}

	glutPostRedisplay();
}

void reset() {
	for (int i = 0; i < 20; i++) {
		GLclampf temp_x{ ((float(g() % 2000)) / 1000.0f) - 1.0f };
		GLclampf temp_y{ ((float(g() % 2000)) / 1000.0f) - 1.0f };
		rectangle_list[i] = { true, temp_x, temp_y, temp_x + 0.025f, temp_y - 0.025f,(float(g() % 1000)) / 1000.0f, (float(g() % 1000)) / 1000.0f, (float(g() % 1000)) / 1000.0f };
	}
	for (int j = 20; j < 40; ++j) {
		GLclampf temp_x{ ((float(g() % 2000)) / 1000.0f) - 1.0f };
		GLclampf temp_y{ ((float(g() % 2000)) / 1000.0f) - 1.0f };
		if (g() % 2 == 0) {
			rectangle_list[j] = { true, temp_x, temp_y, temp_x + 0.025f, temp_y - 0.025f,(float(g() % 1000)) / 1000.0f, (float(g() % 1000)) / 1000.0f, (float(g() % 1000)) / 1000.0f };
		}
	}
	erase_width = 0.025f;
	erase_heigh = 0.025f;
	rectangle_list[40].r = 0.0f;
	rectangle_list[40].g = 0.0f;
	rectangle_list[40].b = 0.0f;

}
bool is_drag = false;
GLvoid Mouse(int button, int state, int x, int y) {
	GLclampf mouse_x = (float)(x - (float)800 / 2.0) * (float)(1.0 / (float)(800 / 2.0));
	GLclampf mouse_y = -(float)(y - (float)600 / 2.0) * (float)(1.0 / (float)(600 / 2.0));
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		rectangle_list[40] = { true, mouse_x - erase_width, mouse_y + erase_heigh, mouse_x + erase_width, mouse_y - erase_heigh };
		is_drag = true;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		rectangle_list[40].is_creat = false;
		is_drag = false;
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y) {
	GLclampf mouse_x = (float)(x - (float)800 / 2.0) * (float)(1.0 / (float)(800 / 2.0));
	GLclampf mouse_y = -(float)(y - (float)600 / 2.0) * (float)(1.0 / (float)(600 / 2.0));
	if (is_drag) {
		rectangle_list[40].x1 = mouse_x - erase_width;
		rectangle_list[40].y1 = mouse_y + erase_heigh;
		rectangle_list[40].x2 = mouse_x + erase_width;
		rectangle_list[40].y2 = mouse_y - erase_heigh;
		for (int i = 0; i < 10; ++i) {
			if (rectangle_list[40].is_creat && rectangle_list[i].is_creat && ((rectangle_list[i].x1 < rectangle_list[40].x1 && rectangle_list[40].x1 < rectangle_list[i].x2)
				|| (rectangle_list[i].x1 < rectangle_list[40].x1 && rectangle_list[40].x2 < rectangle_list[i].x2)
				|| (rectangle_list[i].y1 < rectangle_list[40].y1 && rectangle_list[40].y1 < rectangle_list[i].y2)
				|| (rectangle_list[i].y1 < rectangle_list[40].y2 && rectangle_list[40].y2 < rectangle_list[i].y2))) {
				merge(i);
				break;
			}
		}

	}
	glutPostRedisplay();
}

void merge(int target_index) {
	/*rectangle_list[target_index].is_creat = false;
	erase_width += 0.025f;
	erase_heigh += 0.025f;
	rectangle_list[40].r = rectangle_list[target_index].r;
	rectangle_list[40].g = rectangle_list[target_index].g;
	rectangle_list[40].b = rectangle_list[target_index].b;
	*/
	std::cout << "merge" << target_index << std::endl;
}