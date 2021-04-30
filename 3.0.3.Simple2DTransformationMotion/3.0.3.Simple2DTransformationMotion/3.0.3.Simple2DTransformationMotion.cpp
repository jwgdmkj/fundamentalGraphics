#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <time.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 0, win_height = 0; 
float centerx = 0.0f, centery = 0.0f;

//비행기 각도
float planeAnglePos = 0.0f;

//날아가는 칼 수
std::vector<std::pair<float, unsigned int>> swordV;

// 2D 물체 정의 부분은 objects.h 파일로 분리
// 새로운 물체 추가 시 prepare_scene() 함수에서 해당 물체에 대한 prepare_***() 함수를 수행함.
// (필수는 아니나 올바른 코딩을 위하여) cleanup() 함수에서 해당 resource를 free 시킴.
#include "objects.h"

unsigned int timestamp = 0;
void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	glutPostRedisplay();
	glutTimerFunc(10, timer, 0);
}

typedef struct randomCake{
	int scale;
	int totaldropy;
	int startX;
	int toYaxis;
};
std::vector<randomCake> dropCakeV;
std::vector<int> dropCoctailV;

unsigned int dropCount = 0;
void display(void) {
	glm::mat4 ModelMatrix;
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();
    draw_house(); // in MC

	//다가오는 행성들
	if (timestamp % 500 == 0) {

	}
	
	int house_clock = (timestamp % 1442) / 2 - 360; // -360 <= house_clock <= 360 
	float rotation_angle_house = atanf(100.0f*TO_RADIAN*cosf(house_clock * TO_RADIAN)); 
	float scaling_factor_house = 5.0f*(1.0f - fabs(cosf(house_clock * TO_RADIAN)));

	//centerx =  x - win_width/2.0f, centery = (win_height - y) - win_height/2.0f;
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(centerx, centery, 0.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3((float)house_clock, 
		                                                100.0f * sinf(house_clock * TO_RADIAN), 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaling_factor_house, scaling_factor_house, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rotation_angle_house, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house(); // in WC

	int airplane_clock = timestamp % 720; // 0 <= airplane_clock <= 719 
	if (airplane_clock <= 360) { 
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, airplane_clock*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
	}
	else {
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, -(airplane_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));
		
		if (airplane_clock <= 540)
			airplane_s_factor = (airplane_clock - 360.0f) / 180.0f + 1.0f;
		else 
			airplane_s_factor = -(airplane_clock - 540.0f) / 180.0f + 2.0f;
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(airplane_s_factor, airplane_s_factor, 1.0f));
	}
	
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
 	draw_airplane();  

	//방향키로 방향계 비행기
//	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(centerx, centery, 0.0f));
	ModelMatrix = glm::rotate(glm::mat4(1.0f), 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
//	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, win_height/2 , 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, planeAnglePos * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -win_height/6, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();


	for (auto a : swordV) {
		ModelMatrix = glm::rotate(glm::mat4(1.0f), 180.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, win_height / 2, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix,
			a.first * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
		//전진시키기
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -win_height / 8
			- (float)(timestamp - a.second), 0.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sword();
		
		//오래된 건 삭제
		if (swordV.size() > 1) {
			if (timestamp - swordV.front().second > 2 * win_height) {
				swordV.erase(swordV.begin());
			}
		}
	}
	
	int dropcake_clock = timestamp % 360;
	if (dropcake_clock % 180 == 0) {
		dropCount++;
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,
		 -(float)timestamp + win_height/4, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, dropcake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,
		 10.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cake();


	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f,
		-((float)dropCount * 20) + win_height / 4, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, dropcake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	if (dropcake_clock <= 180) {
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,
			40.0f, 0.0f));
	}
	else {
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,
			20.0f, 0.0f));
	}
	//ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(win_width/4, win_height/4, 1.0f));
	//if (airplane_clock <= 180) {
	//	ModelMatrix = glm::rotate(ModelMatrix, dropcake_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	//	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, win_height/10, 0.0f));
	//}
	//else {
	//	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, win_height/20, 0.0f));
	//	ModelMatrix = glm::rotate(ModelMatrix, -(airplane_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	//	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(AIRPLANE_ROTATION_RADIUS, 0.0f, 0.0f));

	//	if (airplane_clock <= 540)
	//		airplane_s_factor = (airplane_clock - 360.0f) / 180.0f + 1.0f;
	//	else
	//		airplane_s_factor = -(airplane_clock - 540.0f) / 180.0f + 2.0f;
	//	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(airplane_s_factor, airplane_s_factor, 1.0f));
	//}
	//	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, win_height / 2, 0.0f));
//	ModelMatrix = glm::rotate(ModelMatrix, planeAnglePos * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
//	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -win_height / 6, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_cake();

	//랜덤반지름 케이크 무지성투하
	srand(time(0));
	if (timestamp % 500 == 0) {
		randomCake newcake;
		newcake.scale = rand() % 3;
		newcake.scale++;
		newcake.totaldropy = 0;
		if (rand() % 2 == 0) {
			newcake.startX = rand() % (win_width / 4);
		}
		else {
			newcake.startX = rand() % (win_width / 4);
			newcake.startX = -newcake.startX;
		}
		newcake.toYaxis = (rand()%5)+1;
		newcake.toYaxis *= 10;

		dropCakeV.push_back(newcake);
	}

	for (int i = 0; i < dropCakeV.size(); i++) {
		ModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3((float)dropCakeV[i].scale,
			(float)dropCakeV[i].scale, 1.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(dropCakeV[i].startX,
			-((float)dropCakeV[i].totaldropy) + win_height / 3, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, dropcake_clock * TO_RADIAN, 
			glm::vec3(0.0f, 0.0f, 1.0f));

		if (dropcake_clock % 180 == 0) {
			int tmp = dropCakeV[i].toYaxis;
			if (dropcake_clock % 360 == 0) {
				dropCakeV[i].toYaxis = ((rand() % 5) + 1) * 10;
				dropCakeV[i].totaldropy += (dropCakeV[i].toYaxis - tmp);
			}
			else {
				dropCakeV[i].toYaxis = ((rand() % 5) + 1) * 5;
				dropCakeV[i].totaldropy += (tmp - dropCakeV[i].toYaxis);
			}
		//	printf("%d\n", dropCakeV[i].toYaxis);
		}

		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,
				(float)dropCakeV[i].toYaxis, 0.0f));

		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cake();
	}

	glFlush();	
}   


int leftkeyboardpressed = 0, rightkeyboardpressed = 0;
void keyboard(unsigned char key, int x, int y) {
	glm::mat4 ModelMatrix;
	switch (key) {
		//키가 왼쪽일 때마다 2도씩, 오른쪽일 때마다 -2도씩 구부리기
	case 'a':
		if (planeAnglePos >= 90.0f)
			break;
		planeAnglePos += 2.0f;
		glutPostRedisplay();
		break;
	case 'd':
		if (planeAnglePos <= -90.0f)
			break;
		planeAnglePos -= 2.0f;
		glutPostRedisplay();
		break;
		
		//z는 칼 쏘기, x는 궁(차 쏘기)
	case 'z':
		swordV.push_back(std::make_pair(planeAnglePos, timestamp));
		break;

	case 'x' :
		break;

	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

int leftbuttonpressed = 0;
void mouse(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		leftbuttonpressed = 1;
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
		leftbuttonpressed = 0;
}

void motion(int x, int y) {
	if (leftbuttonpressed) {
		centerx =  x - win_width/2.0f, centery = (win_height - y) - win_height/2.0f;
		glutPostRedisplay();
	}
//	if (leftkeyboardpressed || rightkeyboardpressed) {
//		printf("qw");
//		glutPostRedisplay();
//	}
} 
	
void reshape(int width, int height) {
	win_width = width, win_height = height;
	
  	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0, 
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer, 0);
	glutCloseFunc(cleanup);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glClearColor(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_airplane();
	prepare_house();
	prepare_cake();
	prepare_sword();
	prepare_car();
	prepare_cocktail();
	prepare_shirt();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program(); 
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

    error = glewInit();
	if (error != GLEW_OK) { 
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
int main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 Simple2DTransformationMotion_GLSL_3.0.3";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC'"
		"    - Mouse used: L-click and move"
	};

	glutInit (&argc, argv);
 	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize (1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


