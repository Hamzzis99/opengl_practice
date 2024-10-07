#include<iostream>
#include<gl/glew.h>
#include<gl/freeglut.h>
#include<gl/freeglut_ext.h>
#include<random>
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid change_xy(int x, int y);

std::random_device rd;
std::mt19937 g(rd());


GLclampf base_r = 1.0f;
GLclampf base_g = 1.0f;
GLclampf base_b = 1.0f;

void generate_rect();
bool is_overlap(int, int);

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

random_rect rectangle_list[10];



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
	glutMotionFunc(change_xy);
	glutMainLoop();

}


GLvoid drawScene(GLvoid) {

	glClearColor(base_r, base_g, base_b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (random_rect temp : rectangle_list) {
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
	case 'a':
		generate_rect();

		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}

	glutPostRedisplay();
}



void generate_rect() {
	for (random_rect& temp : rectangle_list) {
		if (g() % 10 > 3) {
			temp.is_creat = true;
			temp.x1 = ((float)(g() % 2000) - 1000.0) / 1000.0;
			temp.y1 = ((float)(g() % 2000) - 1000.0) / 1000.0;
			temp.x2 = temp.x1 + 0.3;
			temp.y2 = temp.y1 - 0.3;
			temp.r = (float(g() % 1000)) / 1000.0;
			temp.g = (float(g() % 1000)) / 1000.0;
			temp.b = (float(g() % 1000)) / 1000.0;
		}

	}
}

bool is_drag = false;
int selected_rect = -1;
int merge_target = -1;

GLclampf offset_x, offset_y;


GLvoid Mouse(int button, int state, int x, int y) {
	GLclampf mouse_x = (float)(x - (float)800 / 2.0) * (float)(1.0 / (float)(800 / 2.0));
	GLclampf mouse_y = -(float)(y - (float)600 / 2.0) * (float)(1.0 / (float)(600 / 2.0));
	bool is_merged = false;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int cur = 0; cur < 10; ++cur) {
			if (rectangle_list[cur].x1 < mouse_x && mouse_x < rectangle_list[cur].x2 && rectangle_list[cur].y2 < mouse_y && mouse_y < rectangle_list[cur].y1) {
				is_drag = true;
				selected_rect = cur;
				offset_x = mouse_x - rectangle_list[cur].x1;
				offset_y = mouse_y - rectangle_list[cur].y1;
				break;

			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		for (int i = 0; i < 10; ++i) {
			if (rectangle_list[selected_rect].is_creat && is_overlap(i, selected_rect)) {
				merge_target = i;
				is_merged = true;
				break;

			}
		}
		if (is_merged) {

			rectangle_list[merge_target].x1 = std::min(rectangle_list[merge_target].x1, rectangle_list[selected_rect].x1);
			rectangle_list[merge_target].y1 = std::max(rectangle_list[merge_target].y1, rectangle_list[selected_rect].y1);
			rectangle_list[merge_target].x2 = std::max(rectangle_list[merge_target].x2, rectangle_list[selected_rect].x2);
			rectangle_list[merge_target].y2 = std::min(rectangle_list[merge_target].y2, rectangle_list[selected_rect].y2);
			rectangle_list[merge_target].r = (float(g() % 1000)) / 1000.0;
			rectangle_list[merge_target].g = (float(g() % 1000)) / 1000.0;
			rectangle_list[merge_target].b = (float(g() % 1000)) / 1000.0;
			rectangle_list[selected_rect].is_creat = false;

		}
		merge_target = -1;
		is_merged = false;
		is_drag = false;
		selected_rect = -1;
	}

	glutPostRedisplay();

}

GLvoid change_xy(int x, int y) {
	GLclampf width{ rectangle_list[selected_rect].x2 - rectangle_list[selected_rect].x1 };
	GLclampf height{ rectangle_list[selected_rect].y1 - rectangle_list[selected_rect].y2 };
	if (is_drag && selected_rect != -1) {
		GLclampf mouse_x = (float)(x - (float)800 / 2.0) * (float)(1.0 / (float)(800 / 2.0));
		GLclampf mouse_y = -(float)(y - (float)600 / 2.0) * (float)(1.0 / (float)(600 / 2.0));
		rectangle_list[selected_rect].x1 = mouse_x - offset_x;
		rectangle_list[selected_rect].y1 = mouse_y - offset_y;
		rectangle_list[selected_rect].x2 = rectangle_list[selected_rect].x1 + width;
		rectangle_list[selected_rect].y2 = rectangle_list[selected_rect].y1 - height;


	}
	glutPostRedisplay();
}

bool is_overlap(int target, int select) {
	if (rectangle_list[target].x1 < rectangle_list[select].x1 && rectangle_list[select].x1 < rectangle_list[target].x2) {

	}
}