#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#define M_PI 3.14159265358979323846
#define MAX_FRIENDLY 1
#define MAX_ENEMY 3
#define MAX_ROBOTS 3

#define ROBOT_HP 1
#define ROBOT_X 10
#define ROBOT_Y -2
#define ROBOT_Z -40
#define ROBOT_MOVE_INCREMENT 0.03

#define LAZER_LEN 60
#define LAZER_Y -2.0

#define HIP_R 1.5
#define KNEE_R 1.5

#define ROBOT_MIN_SIZE 0.2
#define ROBOT_SIZE_DECREMENT 0.03

#define CANNON_HP 3
#define HIT_CHANCE 15 //prob of getting hit = 1/hitchance, never set to 0
#define CANNON_DOWN_INCREMENT 0.01

#define MAX_LOOKX 3.0
#define MIN_LOOKX -3.0
#define MAX_LOOKY 1.5
#define MIN_LOOKY -1.5

#define INIT_LOOKX 0.0
#define INIT_LOOKY -1.0
#define INIT_LOOKZ -2.0

#define MAX_STAR 25
#define STAR_MINX -100.0
#define STAR_MAXX 100.0
#define STAR_MINY 0.0
#define STAR_MAXY 100.0
#define STAR_Z -100.0
#define STAR_SIZE 0.2


typedef struct Star {
	double x, y, z;
} Star;

typedef struct Bullet{
	int active = 0;
	double x,y,z,rx,ry,move;
} Bullet;

typedef struct EnemyFire {
	int active = 0;
	double x, y, z, dx, dy, dz, tx, ty, tz;
} EnemyFire;

Bullet friendlyFire[MAX_FRIENDLY];
int friendlyFireIdx = 0;

EnemyFire enemyFire[MAX_ENEMY];
int enemyFireIdx = 0;

Star stars[MAX_STAR];

int fireTime = 60;

int cannonHp = CANNON_HP;
boolean lost = false;

typedef struct Robot {
	int active = 0;
	int hp = ROBOT_HP; //# frames the bullet has to be in contact
	double x, y, z, move, initX;
	double size = 1.0;
	double bodyAngle = 0.0;
	double cannonAngle = 0.0;
	double rightHipAngle = 0.0;
	double rightKneeAngle = 0.0;
	double leftHipAngle = 0.0;
	double leftKneeAngle = 0.0;
	double leftShoulderAngle = 0.0;
	double leftElbowAngle = 0.0;
	double rightShoulderAngle = 0.0;
	double rightElbowAngle = 0.0;
	boolean right = true;
	double LhipR = HIP_R;
	double LkneeR = -KNEE_R;
	double RhipR = HIP_R;
	double RkneeR = -KNEE_R;
	boolean dead = false;
} Robot;

Robot robots[MAX_ROBOTS];

boolean NewGame = true;

//void cannonFireAnimation(int);

void mouseMotionHandler3D(int x, int y);

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// screen window identifiers
int window3D;

int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;


// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodySize = 1.0;
float cannonLength = 0.5 * robotBodySize;
float cannonWidth = 0.2 * robotBodySize;
float notchSize = 0.8 * cannonWidth;
float notchLength = 0.5 * cannonLength;
float hipRad = 0.5 * robotBodySize;
float hipLength = 0.5 * robotBodySize;
float upperLegLength = robotBodySize;
float upperLegHeight = 0.2 * robotBodySize;
float upperLegWidth = 0.3 * robotBodySize;
float lowerLegLength = 1.2 * upperLegLength;
float lowerLegHeight = upperLegHeight;
float lowerLegWidth = upperLegWidth;
float footLength = robotBodySize;
float footHeight = 0.5 * robotBodySize;
float footDepth = robotBodySize;
float shoulderRad = hipRad;
float shoulderLength = 2.0 * hipLength;
float upperArmLength = 1.3 * upperLegLength;
float upperArmHeight = upperLegHeight;
float upperArmWidth = upperLegWidth;
float armGunLength = 1.2 * upperArmLength;
float armGunRad = 0.5 * upperArmWidth;


GLfloat star_ambient[] = { 0.99f,0.99f,0.99f,1.0f };
GLfloat star_specular[] = { 0.99f,0.99f,0.99f,1.0f };
GLfloat star_diffuse[] = { 0.99f,0.99f,0.99f,1.0f };
GLfloat star_shininess[] = { 1.0F };

GLfloat robotLeg_mat_ambient[] = { 0.25f,0.25f,0.25f,1.0f };
GLfloat robotLeg_mat_specular[] = { 0.7746f,0.7746f,0.7746f,1.0f };
GLfloat robotLeg_mat_diffuse[] = { 0.05f,0.05f,0.05f,1.0f };
GLfloat robotLeg_mat_shininess[] = { 100.0F };


GLfloat robotBody_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotBody_mat_diffuse[] = { 0.9f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotBody_mat_shininess[] = { 100.0F };

void drawRobot(Robot* robot);
void drawBody(Robot* robot);
void drawRobotCannon(Robot* robot);
void drawLeftLeg(Robot* robot);
void drawRightLeg(Robot* robot);
void drawLeftArm(Robot* robot);
void drawRightArm(Robot* robot);

GLUquadric* sphere;
GLUquadric* cylinder;


GLuint tex[5];

GLubyte yellow[256][256][3];
GLubyte red[256][256][3];
GLubyte white[256][256][3];

void makeTextures()
{
	glGenTextures(5, tex);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	int width, height, nrChannels;
	unsigned char* tile = stbi_load("tiles.bmp", &width, &height, &nrChannels, 0);

	if (tile)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tile);
		glGenerateMipmap(GL_TEXTURE_2D); //prolly not needed
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(tile);

	for (int i = 0; i < 256;i++) {
		for (int j = 0; j < 256;j++) {
			red[i][j][0] = 255;
			red[i][j][1] = 0;
			red[i][j][2] = 0;

			yellow[i][j][0] = 255;
			yellow[i][j][1] = 255;
			yellow[i][j][2] = 0;

			white[i][j][0] = 255;
			white[i][j][1] = 255;
			white[i][j][2] = 255;
		}
	}

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, yellow);
	

	glBindTexture(GL_TEXTURE_2D, tex[2]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, red);

	glBindTexture(GL_TEXTURE_2D, tex[3]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, white);

	glBindTexture(GL_TEXTURE_2D, tex[4]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	int width2, height2, nrChannels2;
	unsigned char* text = stbi_load("clover.bmp", &width2, &height2, &nrChannels2, 0);

	if (text)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, text);
		glGenerateMipmap(GL_TEXTURE_2D); //prolly not needed
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(text);
	
}

double randomDouble(double low, double high)
{
	return ((double)rand() * (high - low)) / (double)RAND_MAX + low;
}


void drawTextureCube(GLfloat size)
{
	static GLfloat n[6][3] =
	{
	  {-1.0, 0.0, 0.0},
	  {0.0, 1.0, 0.0},
	  {1.0, 0.0, 0.0},
	  {0.0, -1.0, 0.0},
	  {0.0, 0.0, 1.0},
	  {0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
	  {0, 1, 2, 3},
	  {3, 2, 6, 7},
	  {7, 6, 5, 4},
	  {4, 5, 1, 0},
	  {5, 6, 2, 1},
	  {7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	glBindTexture(GL_TEXTURE_2D, tex[0]); //REPLACE: rust texture
	for (i = 5; i >= 0; i--) {
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		glVertex3fv(&v[faces[i][0]][0]);
		glTexCoord2f(0.0, 0.0);
		glVertex3fv(&v[faces[i][1]][0]);
		glTexCoord2f(1.0, 0.0);
		glVertex3fv(&v[faces[i][2]][0]);
		glTexCoord2f(1.0, 1.0);
		glVertex3fv(&v[faces[i][3]][0]);
		glTexCoord2f(0.0, 1.0);
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint progID;
GLuint vertID;
GLuint fragID;

void printShaderInfoLog(GLuint obj) {
	int infoLogLength = 0;
	int returnLength = 0;
	char* infoLog;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		infoLog = (char*)malloc(infoLogLength);
		glGetShaderInfoLog(obj, infoLogLength, &returnLength, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj) {
	int infoLogLength = 0;
	int returnLength = 0;
	char* infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		infoLog = (char*)malloc(infoLogLength);
		glGetProgramInfoLog(obj, infoLogLength, &returnLength, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

std::string loadShaderSrc(const std::string& filename) {
	std::ifstream is(filename);
	if (is.is_open()) {
		std::stringstream buffer;
		buffer << is.rdbuf();
		return buffer.str();
	}
	cerr << "Unable to open file " << filename << endl;
	exit(1);
}

void setupShaders() {
	GLenum err = glewInit();
	// create shader
	vertID = glCreateShader(GL_VERTEX_SHADER);
	fragID = glCreateShader(GL_FRAGMENT_SHADER);

	// load shader source from file
	std::string vs = loadShaderSrc("vertexShader.vert");
	const char* vss = vs.c_str();
	std::string fs = loadShaderSrc("fragmentShader.frag");
	const char* fss = fs.c_str();

	// specify shader source
	glShaderSource(vertID, 1, &(vss), NULL);
	glShaderSource(fragID, 1, &(fss), NULL);

	// compile the shader
	glCompileShader(vertID);
	glCompileShader(fragID);

	// check for errors
	printShaderInfoLog(vertID);
	printShaderInfoLog(fragID);

	// create program and attach shaders
	progID = glCreateProgram();
	glAttachShader(progID, vertID);
	glAttachShader(progID, fragID);


	// link the program
	glLinkProgram(progID);
	// output error messages
	printProgramInfoLog(progID);


}


double* vertices;
double* normals;
int numIndices;
int numVertices;
GLuint* indices;

int numTexCoord;
double* texCoord;

//build VBOs with imported file outputMesh.txt
void buildVBOs() {
	FILE* fp;

	//printf("Opening file outputMesh.txt for mesh viewing!\n");
	if (fopen_s(&fp, "cannonMesh.txt", "r") != 0) {
		printf("Error opening file cannonMesh.txt or doesn't exist!\n");
		exit(1);
	}

	fscanf_s(fp, "%d", &numVertices);

	vertices = (double*)malloc(numVertices * sizeof(double));
	normals = (double*)malloc(numVertices * sizeof(double));

	for (int i = 0; i < numVertices; i += 3) {
		fscanf_s(fp, "%lf %lf %lf", &vertices[i], &vertices[i + 1], &vertices[i + 2]);
	}
	for (int i = 0; i < numVertices; i += 3) {
		fscanf_s(fp, "%lf %lf %lf", &normals[i], &normals[i + 1], &normals[i + 2]);
	}

	fscanf_s(fp, "%d", &numIndices);

	indices = (GLuint*)malloc(numIndices * sizeof(GLuint));

	for (int i = 0; i < numIndices; i += 4) {
		fscanf_s(fp, "%u %u %u %u", &indices[i], &indices[i + 1], &indices[i + 2], &indices[i + 3]);
	}

	fclose(fp);

	numTexCoord = numVertices / 3 * 2;
	texCoord = (double*)malloc(numTexCoord * sizeof(double));
	double s = 0.0, t = 0.0, ds = 1.0 / 16.0, dt = 1.0 / 33.0;
	//verticies are stored as a normal table so increase s(x) till 1, then increase t(y)
	for (int i = 0; i < numTexCoord; i += 2) {
		texCoord[i] = s;
		texCoord[i + 1] = t;
		s += ds;
		if (s >= 1.0) {
			t += dt;
			s = 0.0;
		}
	}
}


// Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

GLfloat lazer_ambient[] = { 1.0, 0.94, 0.12, 1.0 }; //neon yellow
GLfloat lazer_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat lazer_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat lazer_shininess[] = { 1.0 };

GLfloat light_position0[] = { 4.0, 18.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 18.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

//
// Surface of Revolution consists of vertices and quads
//
// Set up lighting/shading and material properties for quadMesh
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


GLdouble fov = 60.0;

int lastMouseX;
int lastMouseY;

GLdouble eyeX = 0.0, eyeY = 0.0, eyeZ = 0.0;
GLdouble lookX = INIT_LOOKX, lookY = INIT_LOOKY, lookZ = INIT_LOOKZ;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 150.0;

// (eyeY-lookY)/(eyeZ-lookZ)

GLdouble anglex = 180 / M_PI * atan((eyeY - lookY) / (eyeZ - lookZ));
GLdouble angley = angley = 180 / M_PI * atan((eyeX - lookX) / (eyeZ - lookZ));


void init3DSurfaceWindow()
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.1F, 0.1F, 0.2F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);
}


void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);
}

//double zt = 0.0;

// robot y = -2, z = -30, x = 5

void drawRobot(Robot* robot)
{


	glPushMatrix();

	

	glTranslatef(0.0, 0.0, robot->move);
	glTranslatef(0.0, ROBOT_Y, 0.0);
	glTranslatef(robot->initX, 0.0, 0.0);
	glTranslatef(0.0, 0.0, ROBOT_Z);
	glScalef(robot->size, robot->size, robot->size); //now size can scale entire robot

	//glPushMatrix();

	//glTranslatef(0.0, 0.0, zt);
	//zt += 0.5;
	//if (zt > 50) zt = 0;
	//gluSphere(gluNewQuadric(), 0.2, 20, 20);
	//glPopMatrix();

	//CTM = IV*R_y*R_x
	glPushMatrix();

	// spin body and cannon. 
	glRotatef(robot->bodyAngle, 1.0, 0.0, 0.0);

	//CTM = I * V * R_y * R_x * R_x(bodyAngle)
	drawBody(robot);
	drawRobotCannon(robot);

	glPopMatrix();

	drawLeftLeg(robot);
	drawRightLeg(robot);
	drawLeftArm(robot);
	drawRightArm(robot);

	glPopMatrix();

}


void drawBody(Robot* robot)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	//CTM = I*V* R_y * R_x * R_x(bodyAngle)
	glPushMatrix();
	//CTM = I*V* R_y * R_x * R_x(bodyAngle)*S
	glScalef(robotBodySize, robotBodySize, robotBodySize);
	gluQuadricDrawStyle(sphere, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[4]);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);

	//get position of robot body in eye coords (using VM)
	GLfloat model[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	robot->x = model[12];
	robot->y = model[13];
	robot->z = model[14];
	//printf("robot position: (%f, %f, %f)\n", model[12], model[13], model[14] );

	glPopMatrix();
}


void drawRobotCannon(Robot* robot)
{
	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = I*V* R_y * R_x * R_x(bodyAngle)
	glPushMatrix();

	// Position cannon with respect to parent (body), place it slightly within the body

	glTranslatef(0.0, 0.0, (robotBodySize - 0.25 * cannonLength));

	//rotate cannon and notch on z-axis
	glRotatef(robot->cannonAngle, 0.0, 0.0, 1.0);

	//CTM = I * V * R_y * R_x * R_x(bodyAngle) * T1 * R1
	glPushMatrix();
	// Build cannon
	glScalef(cannonWidth, cannonWidth, cannonLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 50, 50);
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();

	// position notch above cylinder
	glTranslatef(0.0, (cannonWidth + 0.5 * notchSize), (cannonLength - 0.5 * notchLength));

	glPushMatrix();
	glScalef(notchSize, notchSize, notchLength);
	drawTextureCube(1.0);
	glPopMatrix();

	glPopMatrix();


}

void drawLeftLeg(Robot* robot)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV* R_y * R_x 
	glPushMatrix();

	// Position hip with respect to parent body
	glTranslatef(robotBodySize, (-1.5 * hipRad), 0.0);

	glRotatef(90, 0.0, 1.0, 0.0);

	// rotate hip and sub-parts (legs etc)
	glRotatef(robot->leftHipAngle, 0.0, 0.0, 1.0);


	// build hip
	glPushMatrix();
	glScalef(hipRad, hipRad, hipLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	//rotate 45 degrees about the hip joint
	glRotatef(45, 0.0, 0.0, 1.0);

	//position upperleg w/ resp. to hip
	glTranslatef(0.0, -(hipRad + 0.5 * upperLegLength), 0.5 * hipLength);

	//place upperleg on hip
	glPushMatrix();
	glScalef(upperLegHeight, upperLegLength, upperLegWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	glTranslatef(0.0, -0.5 * upperLegLength, 0.0);

	//rotates will occur at the knee joint now
	glRotatef(robot->leftKneeAngle, 0.0, 0.0, 1.0);

	//translate first to change pivot point to knee
	glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

	glPushMatrix();
	glScalef(lowerLegLength, lowerLegHeight, lowerLegWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	//position foot w/ resp. to lower leg
	glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

	//rotate foot so that it is flat
	glRotatef(-45, 0.0, 0.0, 1.0);


	glPushMatrix();
	glScalef(footLength, footHeight, footDepth);
	drawTextureCube(1.0);
	glPopMatrix();


	glPopMatrix();
}

void drawRightLeg(Robot* robot)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV* R_y * R_x 
	glPushMatrix();

	// Position hip with respect to parent body
	glTranslatef(-(robotBodySize + hipLength), (-1.5 * hipRad), 0.0);


	glRotatef(90, 0.0, 1.0, 0.0);

	// rotate hip and sub-parts (legs etc)
	glRotatef(robot->rightHipAngle, 0.0, 0.0, 1.0);

	// build hip
	glPushMatrix();
	glScalef(hipRad, hipRad, hipLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	//rotate 45 degrees about the hip joint
	glRotatef(45, 0.0, 0.0, 1.0);

	//position upperleg w/ resp. to hip
	glTranslatef(0.0, -(hipRad + 0.5 * upperLegLength), 0.5 * hipLength);

	//place upperleg on hip
	glPushMatrix();
	glScalef(upperLegHeight, upperLegLength, upperLegWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	glTranslatef(0.0, -0.5 * upperLegLength, 0.0);

	//rotates will occur at the knee joint now
	glRotatef(robot->rightKneeAngle, 0.0, 0.0, 1.0);

	//translate first to change pivot point to knee
	glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);
	glPushMatrix();
	glScalef(lowerLegLength, lowerLegHeight, lowerLegWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	//position foot w/ resp. to lower leg
	glTranslatef(-0.5 * lowerLegLength, 0.0, 0.0);

	//rotate foot so that it is flat
	glRotatef(-45, 0.0, 0.0, 1.0);

	glPushMatrix();
	glScalef(footLength, footHeight, footDepth);
	drawTextureCube(1.0);
	glPopMatrix();


	glPopMatrix();

}

void drawLeftArm(Robot* robot)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV*R_y*R_x
	glPushMatrix();

	// Position shoulder with respect to parent body
	glTranslatef(robotBodySize, 1.5 * shoulderRad, 0.0);
	glRotatef(90, 0.0, 1.0, 0.0);

	// rotate shoulder and sub-parts (arms etc)
	glRotatef(robot->leftShoulderAngle, 0.0, 0.0, 1.0);

	// build shoulder
	glPushMatrix();
	glScalef(shoulderRad, shoulderRad, shoulderLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	//rotate -45 degrees about the shoulder joint
	glRotatef(-45, 0.0, 0.0, 1.0);

	//position upperarm w/ resp. to shoulder
	glTranslatef(0.0, -(shoulderRad + 0.5 * upperArmLength), shoulderLength - 0.5 * upperArmWidth);

	//place upperArm on shoulder
	glPushMatrix();
	glScalef(upperArmHeight, upperArmLength, upperArmWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	glTranslatef(0.0, -0.5 * upperArmLength, 0.0);

	//rotates will occur at the elbow joint (base of cylinder)
	glRotatef(robot->leftElbowAngle, 0.0, 0.0, 1.0);

	glRotatef(-90, 0.0, 1.0, 0.0);

	glPushMatrix();
	glScalef(armGunRad, armGunRad, armGunLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	glPopMatrix();
}

void drawRightArm(Robot* robot)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	//CTM = IV*R_y*R_x
	glPushMatrix();

	// Position shoulder with respect to parent body
	glTranslatef(-(robotBodySize + shoulderLength), 1.5 * shoulderRad, 0.0);
	glRotatef(90, 0.0, 1.0, 0.0);

	// rotate shoulder and sub-parts (arms etc)
	glRotatef(robot->rightShoulderAngle, 0.0, 0.0, 1.0);

	// build shoulder
	glPushMatrix();
	glScalef(shoulderRad, shoulderRad, shoulderLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	//rotate -45 degrees about the shoulder joint
	glRotatef(-45, 0.0, 0.0, 1.0);

	//position upperarm w/ resp. to shoulder
	glTranslatef(0.0, -(shoulderRad + 0.5 * upperArmLength), 0.5 * upperArmWidth);

	//place upperArm on shoulder
	glPushMatrix();
	glScalef(upperArmHeight, upperArmLength, upperArmWidth);
	drawTextureCube(1.0);
	glPopMatrix();

	glTranslatef(0.0, -0.5 * upperArmLength, 0.0);

	//rotates will occur at the elbow joint (base of cylinder)
	glRotatef(robot->rightElbowAngle, 0.0, 0.0, 1.0);
	glRotatef(-90, 0.0, 1.0, 0.0);

	glPushMatrix();
	glScalef(armGunRad, armGunRad, armGunLength);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 1.0, 1.0, 1.0, 100, 100);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	glPopMatrix();

}

void drawGround()
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);

	double x = -50.0, z = -190.0, dx = 100.0/4.0, dz = 200.0/4.0;

	glBindTexture(GL_TEXTURE_2D, tex[0]);

	while (z < 10) {
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);

		glVertex3f(x, -12.0f, z);
		//glTexCoord2f(s, t);
		glTexCoord2f(0.0, 0.0);

		glVertex3f(x, -12.0f, z + dz);
		//glTexCoord2f(s, t + dt);
		glTexCoord2f(0.0, 1.0);

		glVertex3f(x + dx, -12.0f, z + dz);
		//glTexCoord2f(s + ds, t + dt);
		glTexCoord2f(1.0, 1.0);

		glVertex3f(x + dx, -12.0f, z);
		//glTexCoord2f(s + ds, t);
		glTexCoord2f(1.0, 0.0);
		glEnd();

		x += dx;
		if (x >= 50.0) {
			z += dz;
			x = -50.0;
		}
	}
	

	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
}



void drawCannon()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glPushMatrix();

	glTranslatef(0, -3.0, 0);
	glTranslatef(eyeX, eyeY, eyeZ);

	glRotatef(angley, 0.0, 1.0, 0.0);
	glRotatef(-anglex, 1.0, 0.0, 0.0);
	glRotatef(10, 1.0, 0.0, 0.0);
	glRotatef(-90, 1.0, 0.0, 0.0);

		int stride = 3 * sizeof(double);
		glBindTexture(GL_TEXTURE_2D, tex[0]);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_DOUBLE, stride, vertices);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_DOUBLE, stride, normals);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_DOUBLE, 0, texCoord);

		glDrawElements(GL_QUADS, numIndices, GL_UNSIGNED_INT, indices);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();

}


void drawCannonBullet(Bullet* bullet) {

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, lazer_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, lazer_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, lazer_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, lazer_shininess);

	glTranslatef(0.0, LAZER_Y, 0.0);
	glRotatef(bullet->ry, 0.0, 1.0, 0.0);
	glRotatef(bullet->rx, 1.0, 0.0, 0.0);
	//glTranslatef(0.0, 0.0, bullet->move);
	glTranslatef(0.0, 0.0, -LAZER_LEN + bullet->move);

	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluCylinder(cylinder, 0.2, 0.2, LAZER_LEN, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);


	GLfloat model[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	bullet->x = model[12];
	bullet->y = model[13];
	bullet->z = model[14];
	//printf("Bullet position: (%f, %f, %f)\n", model[12], model[13], model[14] );

	glPopMatrix();

}

void drawEnemyBullet(EnemyFire* bullet) {

	glPushMatrix();
	
	//make dark and shiny like robot leg
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);

	glTranslatef(bullet->tx, bullet->ty, bullet->tz);
	glScalef(0.2, 0.2, 0.2);

	
	gluQuadricDrawStyle(sphere, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[2]);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, 1.0, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	//gluSphere(gluNewQuadric(), 1.0, 20, 20);

	GLfloat model[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	bullet->x = model[12];
	bullet->y = model[13];
	bullet->z = model[14];
	//printf("Bullet position: (%f, %f, %f)\n", model[12], model[13], model[14] );

	glPopMatrix();

}

void createEnemyBullet(Robot* robot) {
	double x, y, z;
	int r = rand() % HIT_CHANCE; //10% of time shoot directly at cannon
	if (r == 0) {
		x = eyeX;
		y = eyeY;
		z = eyeZ;
	}
	else {
		x = randomDouble(-5.0, 5.0);
		y = randomDouble(-2.5, 5.0);
		z = 0.0;
	}
	//double zfactor = abs( 2 * (z - (ROBOT_Z + robot->move + robotBodySize)) );
	enemyFire[enemyFireIdx].active = 1;
	enemyFire[enemyFireIdx].tx = robot->initX;
	enemyFire[enemyFireIdx].ty = ROBOT_Y;
	enemyFire[enemyFireIdx].tz = ROBOT_Z + robot->move + robotBodySize; //bullet origin is just infront of robot
	
	double sfactor = sqrt(pow((x - (robot->initX)), 2) + pow((y - (ROBOT_Y)), 2) + pow((z - (ROBOT_Z + robot->move + robotBodySize)), 2));;
	
	enemyFire[enemyFireIdx].dx = (x-(robot->initX)) / sfactor;
	enemyFire[enemyFireIdx].dy = (y-(ROBOT_Y)) / sfactor;
	enemyFire[enemyFireIdx].dz = (z - (ROBOT_Z + robot->move + robotBodySize)) / sfactor; // divide by distance, make bullets move more uniformly

	enemyFireIdx++;
	enemyFireIdx = enemyFireIdx % MAX_ENEMY;
}

void drawStar(Star* star) {

	glPushMatrix();

	glMaterialfv(GL_FRONT, GL_AMBIENT, star_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, star_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, star_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, star_shininess);

	glTranslatef(star->x,star->y,star->z);

	gluQuadricDrawStyle(sphere, GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, tex[3]);
	gluQuadricTexture(sphere, GL_TRUE);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluSphere(sphere, STAR_SIZE, 5, 5);
	glBindTexture(GL_TEXTURE_2D, 0);

	//gluSphere(sphere, 1.0, 10, 10);

	glPopMatrix();

}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Set up the Viewing Transformation (V matrix)	
	gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

	drawGround();


	// Draw quad mesh
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	for (int i = 0; i < MAX_FRIENDLY; i++) {
		if (friendlyFire[i].active == 1) {
			drawCannonBullet(&friendlyFire[i]);
		}
	}

	drawCannon();



	for (int i = 0; i < MAX_ROBOTS; i++) {
		if (robots[i].active == 1) {
			drawRobot(&robots[i]);
		}
	}

	if (fireTime <= 0) {
		boolean flag = false;
		for (int i = 0; i < MAX_ROBOTS; i++) {
			if (robots[i].active == 1) {  //ensure there is at least 1 active robot
				flag = true;
				break;
			}
		}
		if (flag) {
			int r = rand() % MAX_ROBOTS;
			while (robots[r].active != 1) {
				r = rand() % MAX_ROBOTS;
			}
			createEnemyBullet(&robots[r]); //bullet from random active robot
		}
		fireTime = 60; //fire once a second
	}

	// draw enemy bullets
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemyFire[i].active == 1) {
			drawEnemyBullet(&enemyFire[i]);
		}
	}

	//draw stars
	for (int i = 0; i < MAX_STAR; i++) {
		drawStar(&stars[i]);
	}

	glPopMatrix();
	glutSwapBuffers();
}


int currentButton;

void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	switch (button)
	{
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
		{
			// Used scroll wheel
		}
		break;
	default:
		break;
	}
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse)
{
	
	glutPostRedisplay();

}



void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;

		if (dx > 0) {
			lookX += 0.03;
			if (lookX > MAX_LOOKX)
				lookX = MAX_LOOKX;
		}
		else if (dx < 0) {
			lookX -= 0.03;
			if (lookX < MIN_LOOKX)
				lookX = MIN_LOOKX;
		}

		if (dy > 0) {
			lookY -= 0.03;
			if (lookY < MIN_LOOKY)
				lookY = MIN_LOOKY;
		}
		else if (dy < 0) {
			lookY += 0.03;
			if (lookY > MAX_LOOKY)
				lookY = MAX_LOOKY;
		}
		

	anglex = 180 / M_PI * atan((eyeY - lookY) / (eyeZ - lookZ));
	angley = 180 / M_PI * atan((eyeX - lookX) / (eyeZ - lookZ));
	
	lastMouseX = x;
	lastMouseY = y;
	glutPostRedisplay();
}

void keyboardHandler3D(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 32: //ascii for spacebar
		friendlyFire[friendlyFireIdx].active = 1;
		friendlyFire[friendlyFireIdx].rx = -anglex;
		friendlyFire[friendlyFireIdx].ry = angley;
		friendlyFire[friendlyFireIdx].move = 0.0;
		friendlyFireIdx++;
		friendlyFireIdx = friendlyFireIdx % MAX_FRIENDLY;
		//printf("space\n");
		break;
	case 'r':
	case 'R':
		NewGame = true;
		break;
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void keyboardUpHandler3D(unsigned char key, int x, int y) {
	switch (key)
	{
	case 32: //on space up, deactivate lazer
		for (int i = 0; i < MAX_FRIENDLY; i++) {
			friendlyFire[i].active = 0;
		}
		//printf("up\n");
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void moveBullets() {
	// move friendly bullets, dont need for lazer
	//for (int i = 0; i < MAX_FRIENDLY; i++) {
	//	if(friendlyFire[i].active == 1)
	//		friendlyFire[i].move -= 3.0;
	//}
	for (int i = 0; i < MAX_FRIENDLY; i++) {
		if (friendlyFire[i].active == 1) {
			friendlyFire[i].rx = -anglex;
			friendlyFire[i].ry = angley;
		}
	}
	// move enemy bullets
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemyFire[i].active == 1) {
			enemyFire[i].tx += enemyFire[i].dx;
			enemyFire[i].ty += enemyFire[i].dy;
			enemyFire[i].tz += enemyFire[i].dz;
		}
	}
}

void stepRightCheck(Robot * robot)
{
	//starts at 0, increments till >40
	//decrements till <-10
	//stops when <-20
	if (robot->rightHipAngle >= 40.0)
	{
		robot->RhipR = -1.5 * HIP_R;
		robot->RkneeR = 0.25 * KNEE_R;
	}
	else if (robot->rightHipAngle <= -20.0)
	{
		//stop animating and reset increments and angles
		robot->rightHipAngle = 0.0;
		robot->rightKneeAngle = 0.0;
		robot->RhipR = HIP_R;
		robot->RkneeR = -KNEE_R;

		robot->right = false;
		return;
	}
	else if (robot->rightHipAngle <= -10.0)
	{
		robot->RhipR = -0.5 * HIP_R;
		robot->RkneeR = 2.5 * KNEE_R;
	}
}

void stepLeftCheck(Robot* robot)
{
	//starts at 0, increments till >40
	//decrements till <-10
	//stops when <-20
	if (robot->leftHipAngle >= 40.0)
	{
		robot->LhipR = -1.5 * HIP_R;
		robot->LkneeR = 0.25 * KNEE_R;
	}
	else if (robot->leftHipAngle <= -20.0)
	{
		//stop animating and reset increments and angles
		robot->leftHipAngle = 0.0;
		robot->leftKneeAngle = 0.0;
		robot->LhipR = 1.0 * HIP_R;
		robot->LkneeR = -1.0 * KNEE_R;

		robot->right = true;
		return;
	}
	else if (robot->leftHipAngle <= -10.0)
	{
		robot->LhipR = -0.5 * HIP_R;
		robot->LkneeR = 2.5 * KNEE_R;
	}
}

void moveRobots() {
	for (int i = 0; i < MAX_ROBOTS; i++) {
		if (robots[i].active == 1 && !robots[i].dead) {
			robots[i].move += ROBOT_MOVE_INCREMENT; //move robots foreward

			//if robot moves past cannon, lose
			if (robots[i].move > abs(ROBOT_Z)) {
				printf("A Robot got past you!\n");
				lost = true;
				robots[i].active = 0; //deactivate robot
			}

			robots[i].cannonAngle += 3.0; //rotate cannon
			if(robots[i].cannonAngle > 360.0) 
				robots[i].cannonAngle -= 360.0;

			//animate walk
			if (robots[i].right) {
				//animate right, add angle increments
				robots[i].rightHipAngle += robots[i].RhipR;
				robots[i].rightKneeAngle += robots[i].RkneeR;
				stepRightCheck(&robots[i]);
			}
			else {
				//animate left
				robots[i].leftHipAngle += robots[i].LhipR;
				robots[i].leftKneeAngle += robots[i].LkneeR;
				stepLeftCheck(&robots[i]);
			}
		}

		if (robots[i].dead) {
			//shrink them
			robots[i].size -= ROBOT_SIZE_DECREMENT;
			if (robots[i].size < ROBOT_MIN_SIZE) { //unactivate robot if shrink too small
				robots[i].active = 0;
			}
		}
	}
}

void checkCollision() {
	// friendy fire to robot collision
	for (int i = 0; i < MAX_FRIENDLY; i++) {
		if (friendlyFire[i].active == 1) {
			for (int j = 0; j < MAX_ROBOTS; j++) {
				if (robots[j].active == 1 && !robots[j].dead) { //ensure robot not already hit
					double sfactor = 2.0; //check more often
					double dx, dy, dz;
					dx = (lookX - eyeX) / sfactor; 
					dy = (lookY - eyeY) / sfactor; 
					dz = (lookZ - eyeZ) / sfactor;
					double x = eyeX, y = eyeY+LAZER_Y, z = eyeZ;
					double dist = sqrt(pow( (x - robots[j].x) , 2) + pow( (y - robots[j].y) , 2) + pow( (z - robots[j].z) , 2));
					x += dx;
					y += dy;
					z += dz;
					double newdist = sqrt(pow((x - robots[j].x), 2) + pow((y - robots[j].y), 2) + pow((z - robots[j].z), 2));
					//now this is world coords, need robot world coords as well
					while (newdist < dist) {
						x += dx; 
						y += dy;
						z += dz;
						//printf("x:%lf, y:%lf, z:%lf\n",dx,dy,dz);
						dist = newdist;
						newdist = sqrt(pow((x - robots[j].initX), 2) + pow((y -	ROBOT_Y), 2) + pow((z - (ROBOT_Z + robots[j].move)), 2));
					}
					if (dist < 1.5) {
						//printf("Bullet %d hit robot %d\n", i, j);
						robots[j].hp -= 1;
						if (robots[j].hp <= 0) {
							robots[j].dead = true;
						}
					}
				}
			}
		}
	}

	// enemy fire to cannon collision
	for (int i = 0; i < MAX_ENEMY; i++) {
		if (enemyFire[i].active == 1 && !lost) { //stop collision detection when lost
			double dist = sqrt(pow((enemyFire[i].x - eyeX), 2) + pow((enemyFire[i].y - eyeY), 2) + pow((enemyFire[i].z - eyeZ), 2));
			if (dist < 1.0) {
				//printf("Bullet %d hit cannon\n", i);
				enemyFire[i].active = 0; //make bullet unactive to prevent further hits
				cannonHp--;
				printf("HIT! Remaining HP: %d\n",cannonHp);
				if (cannonHp <= 0) {
					printf("YOU LOST! R to reset!\n");
					lost = true;
				}
			}
		}
	}
}

void myTimer(int value) {

	moveRobots();
	moveBullets();

	fireTime -= 1;

	checkCollision();

	if (NewGame) {
		for (int i = 0; i < MAX_ROBOTS; i++) { //create all robots
			robots[i].active = 1;
			robots[i].size = 1.0;
			robots[i].dead = false;
			robots[i].hp = ROBOT_HP;
			robots[i].move = 0.0;
			robots[i].initX = -ROBOT_X + i * ROBOT_X;
		}
		for (int i = 0; i < MAX_ENEMY; i++) {
			enemyFire[i].active = 0;			//deactivate all enemy fire
		}
		cannonHp = CANNON_HP;
		lookX = INIT_LOOKX; //reset camera and angles
		lookY = INIT_LOOKY;
		lookZ = INIT_LOOKZ;
		anglex = 180 / M_PI * atan((eyeY - lookY) / (eyeZ - lookZ));
		angley = 180 / M_PI * atan((eyeX - lookX) / (eyeZ - lookZ));
		lost = false;
		printf("Game Started! HP: %d\n", cannonHp);
		glutPassiveMotionFunc(mouseMotionHandler3D); //re-enable mouse control
		NewGame = false;
	}

	if (lost) {
		//disable camera movement
		glutPassiveMotionFunc(NULL);
		//animate camera and cannon down
		lookY -= CANNON_DOWN_INCREMENT;
		if (lookY < MIN_LOOKY) {
			lookY = MIN_LOOKY;
		}
		//reupdate angles
		anglex = 180 / M_PI * atan((eyeY - lookY) / (eyeZ - lookZ));
		angley = 180 / M_PI * atan((eyeX - lookX) / (eyeZ - lookZ));
	}

	//check if win, maybe not needed

	glutPostRedisplay();
	glutTimerFunc(17, myTimer, 0);

}

void init() {
	//setup random
	srand(time(0));

	for (int i = 0; i < MAX_STAR; i++) {
		stars[i].x = randomDouble(STAR_MINX, STAR_MAXX);
		stars[i].y = randomDouble(STAR_MINY, STAR_MAXY);
		stars[i].z = STAR_Z;
	}

	//setup vertex and fragment shaders
	setupShaders();
	glUseProgram(progID);


	sphere = gluNewQuadric();
	cylinder = gluNewQuadric();

	//textures
	glEnable(GL_TEXTURE_2D);
	makeTextures();

	//build the VBOs from file outputMesh.txt once at the beginning
	buildVBOs();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth, glutWindowHeight);
	glutInitWindowPosition(600, 100);

	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution");
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	//glutMotionFunc(mouseMotionHandler3D);
	glutKeyboardFunc(keyboardHandler3D);
	glutPassiveMotionFunc(mouseMotionHandler3D);
	glutKeyboardUpFunc(keyboardUpHandler3D);

	//timerfunction
	glutTimerFunc(17, myTimer, 0);
	// Initialize the 3D system
	init3DSurfaceWindow();

	init();

	// Annnd... ACTION!!
	glutMainLoop();

	return 0;
}