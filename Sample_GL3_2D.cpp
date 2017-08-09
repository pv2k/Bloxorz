#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
	float x,y,z;
	int status;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 projection1;
	glm::mat4 model;
	glm::mat4 view;
	float x,y,z;
	int status;
	GLuint MatrixID;
} Matrices;

struct Color {
	float r;
	float g;
	float b;
};
typedef struct Color Color;

struct gameObjects {
	string name;
	VAO *object;
	Color color;
	int x;
	int y;
	int z;
	int val;
	float len,breadth,height;
};
typedef struct gameObjects gameObjects;
map< int, map<int, gameObjects> > marb;
gameObjects cube1 = {};
gameObjects cube2 = {};
int cubx1=0,cuby1=0,cubx2=0,cuby2=6,cubz1=0,cubz2=0;
int flag = 1,lv8_flg = 0,view=0;
bool fall = false;

void createCuboid(int i,int j,Color color,string body,float pf,float nf);
void createRectangle (string comp, string body, Color Color, float l, float b, float x, float y,float angle);
void createCircle (string comp, string body, Color Color, float radius, float x, float y,float parts);
void up();
void down();
void left();
void right();
void init();
void play1();
void play2();
void play3();
void play4();
void play6();
void play7();
void play8();

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

void* play_audio(string audioFile){
	mpg123_handle *mh;
	unsigned char *buffer;
	size_t buffer_size;
	size_t done;
	int err;

	int driver;
	ao_device *dev;

	ao_sample_format format;
	int channels, encoding;
	long rate;

	/* initializations */
	ao_initialize();
	driver = ao_default_driver_id();
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = mpg123_outblock(mh);
	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, &audioFile[0]);
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * 8;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);

	/* decode and play */
	char *p =(char *)buffer;
	while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, p, done);

	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
 int totalscore=0,currentscore=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_O:
				view=0;
				break;
			case GLFW_KEY_T:
				view=1;
				break;
			case GLFW_KEY_B:
				view=2;
				break;
			case GLFW_KEY_F:
				view=3;
				break;
			case GLFW_KEY_H:
				view=4;
				break;
			default:
				break;
		}
	}
	if(key==GLFW_KEY_RIGHT && action==GLFW_PRESS && !fall){
		thread(play_audio,"./star.mp3").detach();
		currentscore++;
		if(lv8_flg==0){
		right();
	}
	else if(lv8_flg==1){
		cubx2+=6;
	}
	else if(lv8_flg==2){
		cubx1+=6;
	}

	}

	else if(key==GLFW_KEY_LEFT && action==GLFW_PRESS && !fall){
		thread(play_audio,"./star.mp3").detach();
		currentscore++;
		if(lv8_flg==0){
		left();
	}
	else if(lv8_flg==1){
		cubx2-=6;
	}
	else if(lv8_flg==2)
		cubx1-=6;
	}

	else if(key==GLFW_KEY_UP && action==GLFW_PRESS && !fall){
		thread(play_audio,"./star.mp3").detach();
		currentscore++;
		if(lv8_flg==0)
		{
			up();
		}
			else if(lv8_flg==1){
				cubz2-=6;
			}
			else if(lv8_flg==2)
				cubz1-=6;

	}
	if(key==GLFW_KEY_DOWN && action==GLFW_PRESS && !fall){
		thread(play_audio,"./star.mp3").detach();
		currentscore++;
		if(lv8_flg==0){
			down();
	}
	else if(lv8_flg==1)
		cubz2+=6;
	else if(lv8_flg==2)
		cubz1+=6;

	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 0.9f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection1 = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.1f, 500.0f);
	Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
}

VAO  *rectangle,*cub1,*cub2,*circle,*cuboid1[10][15],*brow_cub,*circle1,*blu_cub,*red_cub,*blac_cub,*obj;
void right(){
	if(cubx1==cubx2 && cuby2>cuby1){
		cubx1+=6;
		cubx2+=12;
		cuby2-=6;

	}

	else if(cubx1==cubx2 && cuby1>cuby2){
		cubx1+=12;
		cubx2+=6;
		cuby1-=6;
	}
	else if(cubx1>cubx2 && cuby1==cuby2){
		cubx1+=6;
		cubx2+=12;
		cuby2+=6;
	}
	else if(cubx2>cubx1 && cuby1==cuby2){
		cubx2+=6;
		cubx1+=12;
		cuby1+=6;
	}
	else if(cubx2==cubx1 && cuby1==cuby2){
		cubx1+=6;
		cubx2+=6;
		//cubz2+=6;
	}
}
void left(){
	if(cubx1==cubx2 && cuby2>cuby1){
		cubx1-=6;
		cubx2-=12;
		cuby2-=6;

	}

	else if(cubx1==cubx2 && cuby1>cuby2){
		cubx1-=12;
		cubx2-=6;
		cuby1-=6;
	}
	else if(cubx1>cubx2 && cuby1==cuby2){
		cubx1-=12;
		cubx2-=6;
		cuby1+=6;
	}
	else if(cubx2>cubx1 && cuby1==cuby2){
		cubx1-=6;
		cubx2-=12;
		cuby2+=6;
	}
	else if(cubx2==cubx1 && cuby1==cuby2){
		cubx1-=6;
		cubx2-=6;
		//cubz2+=6;
	}
}
void up(){
	if(cubz1==cubz2 && cuby2>cuby1){
		cubz1-=6;
		cubz2-=12;
		cuby2-=6;

	}

	else if(cubz1==cubz2 && cuby1>cuby2){
		cubz1-=12;
		cubz2-=6;
		cuby1-=6;
	}
	else if(cubz1>cubz2 && cuby1==cuby2){
		cubz1-=12;
		cubz2-=6;
		cuby1+=6;
	}
	else if(cubz2>cubz1 && cuby1==cuby2){
		cubz1-=6;
		cubz2-=12;
		cuby2+=6;
	}
	else if(cubz2==cubz1 && cuby1==cuby2){
		cubz1-=6;
		cubz2-=6;
		//cubz2+=6;
	}
}
void down(){
	if(cubz1==cubz2 && cuby2>cuby1){
		cubz1+=6;
		cubz2+=12;
		cuby2-=6;

	}

	else if(cubz1==cubz2 && cuby1>cuby2){
		cubz1+=12;
		cubz2+=6;
		cuby1-=6;
	}
	else if(cubz1>cubz2 && cuby1==cuby2){
		cubz1+=6;
		cubz2+=12;
		cuby2+=6;
	}
	else if(cubz2>cubz1 && cuby1==cuby2){
		cubz2+=6;
		cubz1+=12;
		cuby1+=6;
	}
	else if(cubz2==cubz1 && cuby1==cuby2){
		cubz1+=6;
		cubz2+=6;
		//cubz2+=6;
	}
}
int segment_cnt=0;
VAO* segment[20];
void set_segments(int letter_cnt,char letter)
{
	for(int i=0;i<9;i++)
		segment[9*letter_cnt+i]->status=0;

	//left1
	if(letter=='O' || letter=='M' || letter=='G' || letter=='A' || letter=='U'|| letter=='E' || letter=='0'|| letter=='4'|| letter=='5'|| letter=='6'|| letter=='8'|| letter=='9')
	{
		segment[9*letter_cnt+0]->status=1;
	}
	//middle1
	if(letter=='1'|| letter=='M')
	{
		segment[9*letter_cnt+1]->status=1;
	}
	//right1
	if(letter=='U' || letter=='O' || letter=='M'|| letter=='A'|| letter=='0'|| letter=='2'|| letter=='3'|| letter=='4'|| letter=='8'|| letter=='7'|| letter=='9')
	{
		segment[9*letter_cnt+2]->status=1;
	}
	//left2
	if(letter=='O' || letter=='M'  || letter=='A'|| letter=='G'|| letter=='U' ||letter=='E'|| letter=='0'|| letter=='2'||  letter=='6'|| letter=='8')
	{
		segment[9*letter_cnt+3]->status=1;
	}
	//middle2
	if(letter=='1')
	{
		segment[9*letter_cnt+4]->status=1;
	}
	//right2
	if(letter=='O' ||  letter=='A' || letter=='G' ||letter=='M'|| letter=='U'|| letter=='0'|| letter=='3'|| letter=='5'|| letter=='6'|| letter=='4'|| letter=='8'|| letter=='7'|| letter=='9')
	{
		segment[9*letter_cnt+5]->status=1;
	}
	//top
	if(letter=='O' || letter=='M' || letter=='A' || letter=='G'|| letter=='E'|| letter=='0'|| letter=='2'|| letter=='3'|| letter=='5'|| letter=='6'|| letter=='8' || letter=='7'|| letter=='9')
	{
		segment[9*letter_cnt+6]->status=1;
	}
	//middle
	if(letter=='G'  || letter=='A' || letter=='E'|| letter=='-' || letter=='2'|| letter=='3'|| letter=='4'|| letter=='5'|| letter=='6'|| letter=='8'|| letter=='9')
	{
		segment[9*letter_cnt+7]->status=1;
	}
	//bottom
	if(letter=='O' || letter=='G' || letter=='U' || letter=='E'|| letter=='0'|| letter=='2'|| letter=='3'|| letter=='5'|| letter=='6'|| letter=='8'|| letter=='9')
	{
		segment[9*letter_cnt+8]->status=1;
	}
}
void createRectangle (string comp, string body, Color Color, float l, float b, float x, float y,float angle)
{
	const GLfloat vertex_buffer_data [] = {
		-l,-b,0,
		l,-b,0,
		l,b,0,

		l,b,0,
		-l,b,0,
		-l,-b,0
	};

	const GLfloat color_buffer_data [] = {
	  Color.r,Color.g,Color.b, // color 1
	  Color.r,Color.g,Color.b, // color 2
	  Color.r,Color.g,Color.b, // color 3

	  Color.r,Color.g,Color.b, // color 3
	  Color.r,Color.g,Color.b, // color 4
	  Color.r,Color.g,Color.b  // color 1
	};

	rectangle = create3DObject(GL_TRIANGLES,6,vertex_buffer_data, color_buffer_data, GL_FILL);

	gameObjects gameObject = {};
	gameObject.name = comp;
	gameObject.object = rectangle;
	gameObject.x = x;
	gameObject.y = y;
	gameObject.z = 0;
	gameObject.len = 2*l;
	gameObject.breadth = 2*b;
	gameObject.color =  Color;
}

void createCircle (string comp, string body, Color Color, float radius, float x, float y,float parts)
{
	GLfloat vertex_buffer_data [360*9];
	GLfloat color_buffer_data [360*9];
	for(int i=0;i<360;i++){
		vertex_buffer_data [9*i]=0;
		vertex_buffer_data [9*i+1]=0;
		vertex_buffer_data [9*i+2]=0;
		vertex_buffer_data [9*i+3]=radius*cos(i*M_PI/180);
		vertex_buffer_data [9*i+4]=radius*sin(i*M_PI/180);
		vertex_buffer_data [9*i+5]=0;
		vertex_buffer_data [9*i+6]=radius*cos((i+1)*M_PI/180);
		vertex_buffer_data [9*i+7]=radius*sin((i+1)*M_PI/180);
		vertex_buffer_data [9*i+8]=0;
	}
	for (int i = 0; i<360*9; i+=3){
		color_buffer_data [i]=Color.r;
		color_buffer_data [i+1]=Color.g;
		color_buffer_data [i+2]=Color.b;
	}
	circle = create3DObject(GL_TRIANGLES, (360*3)*parts, vertex_buffer_data, color_buffer_data, GL_FILL);

	gameObjects gameObject = {};
	gameObject.name = comp;
	gameObject.object = circle;
	gameObject.x = x;
	gameObject.y = y;
	gameObject.color =  Color;
}
void createSegment (GLfloat h,GLfloat w,float x,float y,float z)
{
	GLfloat vertex_buffer_data [] = {
		-w/2,h/2,0, // vertex 1
		w/2,h/2,0, // vertex 2
		w/2,-h/2,0, // vertex 3

		w/2,-h/2,0, // vertex 3
		-w/2,-h/2,0, // vertex 4
		-w/2,h/2,0  // vertex 1
	};
	GLfloat color_buffer_data [] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	segment[segment_cnt] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	segment[segment_cnt]->x = x;
	segment[segment_cnt]->y = y;
	segment[segment_cnt]->z = z;
	segment[segment_cnt]->status=0;
	segment_cnt++;
}

void createCuboid(int i,int j,Color color,string body,float pf,float nf){
	GLfloat vertex_buffer_data [] = {
		nf,nf,nf,
		nf,nf, pf,
		nf, pf, pf,
		pf, pf,nf,
		nf,nf,nf,
		nf, pf,nf,
		pf,nf, pf,
		nf,nf,nf,
		pf,nf,nf,
		pf, pf,nf,
		pf,nf,nf,
		nf,nf,nf,
		nf,nf,nf,
		nf, pf, pf,
		nf, pf,nf,
		pf,nf, pf,
		nf,nf, pf,
		nf,nf,nf,
		nf, pf, pf,
		nf,nf, pf,
		pf,nf, pf,
		pf, pf, pf,
		pf,nf,nf,
		pf, pf,nf,
		pf,nf,nf,
		pf, pf, pf,
		pf,nf, pf,
		pf, pf, pf,
		pf, pf,nf,
		nf, pf,nf,
		pf, pf, pf,
		nf, pf,nf,
		nf, pf, pf,
		pf, pf, pf,
		nf, pf, pf,
		pf,nf, pf
	};
	GLfloat color_buffer_data [12*3*3];
	int v=0;
	while(v < 12*3){
		color_buffer_data [3*v+0] = color.r;
		color_buffer_data [3*v+1] = color.g;
		color_buffer_data [3*v+2] = color.b;
		v++;
	}
			obj = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
			if(i!=-1 && j!=-1){
				gameObjects gobj = {};
				gobj.object = obj;
				marb[i][j] = gobj;

			}
			else{
				if(body == "cube1"){
					cube1.object = obj;
					cube1.x = cubx1;cube1.y = cuby1;cube1.z = cubz1;
				}
				else if(body == "cube2"){
					cube2.object = obj;
					cube2.x = cubx2;cube2.y = cuby2;cube2.z = cubz2;
				}
				else if(body == "brow_cub")
					brow_cub = obj;
				else if(body == "blu_cub")
					blu_cub = obj;
				else if(body == "red_cub")
					red_cub = obj;
				else if(body == "blac_cub")
					blac_cub = obj;
			}

	init();
	play1();
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void play1(){
	for(int i=0;i<10;i++)
		for(int j=0;j<10;j++)
			marb[i][j].val=1;
	for(int i=0;i<2;i++)
		for(int j=0;j<10;j++)
			marb[i][j].val=0;
	for(int i=9;i>=8;i--)
		for(int  j=0;j<10;j++)
			marb[i][j].val=0;
	marb[6][7].val=4;
	marb[6][0].val=0;
	marb[6][2].val=0;
	marb[6][1].val=0;
	marb[6][3].val=0;
	marb[6][4].val=0;
	int i=7;
	for(int j=0;j<6;j++)
		marb[i][j].val=0;
	marb[i][9].val=0;
	marb[5][0].val=0;
	i=2;
	for(int j=3;j<10;j++)
		marb[i][j].val=0;
	i=3;
	for(int j=6;j<10;j++)
		marb[i][j].val=0;
	marb[4][9].val=0;
	for(int i=0;i<10;i++)
		for(int j=10;j<15;j++)
			marb[i][j].val=0;
}
int l2tog=0,l2f=0;
int l2togl=0,l2r=0;
int l3=0,r3=0;
int r4=0;

void play2(){
	for(int i=0;i<10;i++)
		for(int j=0;j<15;j++)
			marb[i][j].val=1;
	for(int i=0;i<3;i++)
		for(int j=0;j<15;j++)
			marb[i][j].val=0;
	for(int i=8;i<10;i++)
		for(int j=0;j<15;j++)
			marb[i][j].val=0;
	for(int i=3;i<8;i++){
		marb[i][4].val=0;
		marb[i][5].val=0;
	}
	int i=2;
	for(int j=6;j<15;j++)
		marb[i][j].val=1;
	marb[2][10].val=0;
	marb[2][11].val=0;
	for(int i=3;i<8;i++)
		for(int j=10;j<12;j++)
			marb[i][j].val=0;
	marb[7][12].val=0;
	marb[7][13].val=0;
	marb[7][14].val=0;
	marb[3][13].val=4;
	marb[4][2].val=2;
	l2tog=0;
	l2f=0;
	marb[3][8].val=3;
	l2r=0;
	l2togl=0;

}
void play3(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			marb[i][j].val=1;
	}
	for(int i=0;i<4;i++)
		for(int j=0;j<15;j++)
			marb[i][j].val=0;
	for(int i=8;i<10;i++)
		for(int j=0;j<15;j++)
			marb[i][j].val=0;
		for(int i=6;i<8;i++)
		for(int j=4;j<11;j++)
			marb[i][j].val=0;
		marb[7][11].val=0;
		int i=3;
		for(int j=6;j<15;j++)
			marb[i][j].val=1;
		marb[4][4].val=0;
		marb[4][5].val=0;
		marb[4][9].val=0;
		marb[4][10].val=0;
		marb[5][9].val=0;
		marb[5][10].val=0;
		marb[3][13].val=0;
		marb[3][14].val=0;
		marb[4][13].val=0;
		marb[4][14].val=0;
		marb[6][13].val=4;
}
void play4(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			marb[i][j].val=1;
	}
	int i=0;
	for(int j=0;j<15;j++)
		marb[i][j].val=0;
	for(int i=1;i<3;i++){
		for(int j=0;j<3;j++)
			marb[i][j].val=0;
		for(int j=13;j<15;j++)
			marb[i][j].val=0;
	}
	for(int i=3;i<6;i++)
		for(int j=4;j<9;j++)
			marb[i][j].val=0;
		marb[4][3].val=0;
		marb[4][9].val=0;
		marb[5][3].val=0;
		marb[5][9].val=0;
		marb[6][3].val=0;
		marb[6][4].val=0;
		marb[7][3].val=0;
		marb[7][4].val=0;

		for(int i=8;i<10;i++)
		for(int j=0;j<5;j++)
			marb[i][j].val=0;
		for(int i=8;i<10;i++)
		for(int j=8;j<10;j++)
			marb[i][j].val=0;

		marb[8][6].val=4;
		marb[8][13].val=5;
		marb[6][8].val=6;
		marb[6][9].val=6;
		marb[7][8].val=6;
		marb[7][9].val=6;

}
void play6(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			marb[i][j].val=1;
	}
	int i;
	//int i=0;
	//for(int j=0;j<15;j++)
	//	marb[i][j].val=0;
	for(int i=0;i<3;i++){
		for(int j=0;j<5;j++)
			marb[i][j].val=0;
	}
	marb[1][6].val=0;
	marb[1][7].val=0;
	marb[2][6].val=0;
	marb[2][7].val=0;
	marb[3][6].val=0;
	for(int i=3;i<6;i++){
		for(int j=7;j<11;j++)
			marb[i][j].val=0;
	}
	marb[6][8].val=0;
	marb[6][7].val=0;
	marb[5][11].val=0;
	for(i=6;i<9;i++)
	for(int j=11;j<15;j++)
		marb[i][j].val=0;
	for(i=0;i<2;i++)
	for(int j=11;j<15;j++)
		marb[i][j].val=0;
	marb[2][13].val=0,marb[2][14].val=0;
	for(i=4;i<9;i++)
	for(int j=0;j<4;j++)
		marb[i][j].val=0;
	for(i=6;i<9;i++)
	for(int j=4;j<6;j++)
		marb[i][j].val=0;
	marb[9][6].val=0;
	marb[9][10].val=0;
	marb[4][13].val=4;
	for(int j=0;j<6;j++)
		marb[9][j].val=0;
	for(int j=10;j<15;j++)
		marb[9][j].val=0;
}
void play7(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			marb[i][j].val=1;
	}
	int i=0;
	for(int j=0;j<15;j++)
		marb[i][j].val=0;
	i=9;
	for(int j=0;j<15;j++)
		marb[i][j].val=0;
	for(int i=1;i<3;i++)
		for(int j=0;j<8;j++)
		marb[i][j].val=0;
	for(int i=1;i<3;i++)
		for(int j=12;j<15;j++)
		marb[i][j].val=0;
	i=3;
		for(int j=3;j<8;j++)
		marb[i][j].val=0;
	marb[3][9].val=0;
	marb[3][10].val=0;
	marb[4][9].val=0;
	for(int i=4;i<7;i++)
		for(int j=10;j<12;j++)
		marb[i][j].val=0;
	for(int i=5;i<8;i++)
		for(int j=3;j<7;j++)
		marb[i][j].val=0;
	for(int i=7;i<9;i++)
		for(int j=9;j<15;j++)
		marb[i][j].val=0;
	marb[4][13].val=4;
	marb[5][9].val=2;

}
void play8(){
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++)
			marb[i][j].val=0;
	}
	for(int i=4;i<7;i++){
		for(int j=0;j<6;j++)
			marb[i][j].val=1;
	}
	for(int i=1;i<10;i++){
		for(int j=9;j<12;j++)
			marb[i][j].val=1;
	}
	for(int i=4;i<7;i++){
		for(int j=12;j<15;j++)
			marb[i][j].val=1;
	}
	marb[5][13].val=4;
	marb[5][4].val=7;
lv8_flg=0;
}

float spo;
int l6=0,r6=0,l7=0,r7=0,r8=0,r9=0;
void init(){
	for(int i=0;i<15;i++)
		for(int j=0;j<10;j++)
			marb[i][j].y=-60;
	currentscore=0;

	//cuby[0][0]=0;
	spo=60;
	cubx1=0;
	cubx2=0;
	cubz1=0;
	cubz2=0;
	cuby1=0;
	cuby2=6;
	fall=false;

}

double current_time,utime=glfwGetTime();
int flagdown=0;

void draw ()
{

	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
		if(view==0){
	Matrices.view = glm::lookAt(glm::vec3(-30,70,60), glm::vec3(0,0,0), glm::vec3(0,1,0));
	// Matrices.projection = glm::ortho((float)(-100.0f/zoom), (float)(100.0f/zoom), (float)(-50.0f/zoom), (float)(50.0f/zoom), 0.1f, 500.0f);
	}
	else if(view==1)
	{
		Matrices.view = glm::lookAt(glm::vec3(0,150,1), glm::vec3(0,0,0), glm::vec3(0,1,0));
	}
	else if(view==2){
	//Matrices.projection = glm::perspective(0.9f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	Matrices.projection = glm::perspective(0.9f+0.6f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(-8+cube1.x+l3+l6+l7,15,-4+cube1.z+r3+r4+r6+r7+r8+r9), glm::vec3(30,0,10), glm::vec3(0,1,0));

	}
	else if(view==3){
	Matrices.projection = glm::perspective(0.9f+0.3f, (GLfloat) 1500 / (GLfloat) 800, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(-33+cube1.x+l3+l6+l7,24,-8+cube1.z+r3+r4+r6+r7+r8+r9), glm::vec3(30,0,10), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	}
    if(view==4){
	Matrices.projection = glm::ortho(-100.0f,100.0f,-50.0f,50.0f,0.1f, 500.0f);

	Matrices.view = glm::lookAt(glm::vec3(-30*cos(camera_rotation_angle*M_PI/180),70,60*sin(camera_rotation_angle*M_PI/180)), glm::vec3(0,0,0), glm::vec3(0,1,0));
	} // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	int i=0;
	while(i<10){
		int j=0;
		while(j<15)
		{
			if(marb[i][j].val==1 ||marb[i][j].val==2 || marb[i][j].val==3 ||marb[i][j].val==6){


				Matrices.model = glm::mat4(1.0f);
				marb[i][j].y+=((i+j)/1.5);
				if(marb[i][j].y>0)
					marb[i][j].y=0;
				marb[i][j].x = (j+1)*6-30;
				marb[i][j].z = (i+1)*6-30;
				glm::mat4 translateTriangle = glm::translate (glm::vec3(marb[i][j].x,marb[i][j].y ,marb[i][j].z )); // glTranslatef
				glm::mat4 rotateTriangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scaleTriangle = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 triangleTransform = translateTriangle * rotateTriangle*scaleTriangle;
				Matrices.model *= triangleTransform;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(marb[i][j].object);

				if(flag==4 ){
					Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(13+1)*6-30, 0.0f+marb[i][j].y, 0.0f+(8+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslate
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(brow_cub);

				}
	/*			Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(j+1)*6-30, 0.0f+cuby[i][j], 0.0f+(i+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(rect[i][j]);
*/

			}
			j++;
		}
		i++;
	}
	spo-=2;
	if(spo<0)
		spo=0;
	int l1=(-18+cubx1+l3+l6+l7)/6 +4;
	int l2=(-18+cubx2+l3+l6+l7)/6+4;
	int r1=(-6+cubz1+r3+r6+r7+r8+r9)/6+4;
	int r2=(-6+cubz2+r3+r6+r7+r8+r9)/6+4;

	Matrices.model = glm::mat4(1.0f);
	cube1.x = cubx1;cube1.y = cuby1;cube1.z = cubz1;
	glm::mat4 transTri1 = glm::translate (glm::vec3(-18.0f+cube1.x+l3+l6+l7, 3.0f+cube1.y+spo, -6.0f+cube1.z+r3+r4+r6+r7+r8+r9)); // glTranslatef

	glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,-3));

	glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 1.5f, 1.5f)); // glTranslatef
	// rotate about vector (1,0,0)
	glm::mat4 tri_prod1 = transTri1*rotTri1 * scalTri1;
	Matrices.model *= tri_prod1;
	MVP = VP * Matrices.model; // MVP = p * V * M

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(cube1.object);

	Matrices.model = glm::mat4(1.0f);
	cube2.x = cubx2;cube2.y = cuby2;cube2.z = cubz2;
	glm::mat4 translateTriangle2 = glm::translate (glm::vec3(-18.0f+cube2.x+l3+l6+l7, 3.0f+cube2.y+spo, -6.0f+cube2.z+r3+r4+r6+r7+r8+r9)); // glTranslatef

	glm::mat4 rotateTriangle2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,-3));

	glm::mat4 scaleTriangle2 = glm::scale (glm::vec3(1.5f, 1.5f, 1.5f)); // glTranslatef
	// rotate about vector (1,0,0)
	glm::mat4 triangleTransform2 = translateTriangle2*rotateTriangle2 * scaleTriangle2;
	Matrices.model *= triangleTransform2;
	MVP = VP * Matrices.model; // MVP = p * V * M

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	draw3DObject(cube2.object);
	if(marb[r1][l1].val==0 || marb[r2][l2].val==0 || r1<0 ||l1<0||r2<0||l2<0){
		double ctime=glfwGetTime();
		if(ctime-utime>0.05){
			utime=glfwGetTime();
			cuby1-=2;
			cuby2-=2;
			fall=true;
		}
		if(cuby1<-15){
		init();
		if(flag==1){
			play1();
		}
		if(flag==2)
			play2();
		if(flag==3)
			play3();
		if(flag==4)
			play4();
		if(flag==5)
			play6();
		if(flag==6)
			play7();
		if(flag==7)
			play8();
	}

	}
	if(flag==4 && marb[6][8].val==0){
		marb[6][8].val=6;
		marb[6][9].val=6;
		marb[7][8].val=6;
		marb[7][9].val=6;

	}
	if(marb[r1][l1].val==4 && marb[r2][l2].val==4){
		double ctime=glfwGetTime();
		if(ctime-utime>0.05){
			utime=glfwGetTime();
			cuby1-=2;
			cuby2-=2;
		}
		if(cuby1<-20){
		totalscore+=currentscore;
		init();
		flag++;
		if(flag==2)
		play2();
		if(flag==3){
			play3();
			l3=0;
			r3=18;
		}
		if(flag==4){
			play4();
		}
		if(flag==5){
			play6();
			l6=-6;
			r6=-18;
		}
		if(flag==6){
			play7();
			l7=6;
			r7=6;
		}
		if(flag==7){
			play8();
			r8=6;
		}
		}
	}

	if(flag==2){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(2+1)*6-30, 0.0f, 0.0f+(4+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(red_cub);

				Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri2 = glm::translate (glm::vec3(0.0f+(8+1)*6-30, 0.0f, 0.0f+(3+1)*6-30)); // glTranslatef
				glm::mat4 tri_prod2 = transTri2;
				glm::mat4 rotTri2 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				tri_prod2 *= rotTri2;
				glm::mat4 scalTri2 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				tri_prod2 *= scalTri2;
				// rotate about vector (1,0,0)
				Matrices.model *= tri_prod2;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(blac_cub);

		if(marb[r1][l1].val==2 || marb[r2][l2].val==2){
		if(marb[6][4].val==0 && l2tog==0){
			marb[6][4].val=1;
			marb[6][5].val=1;
			l2f=1;
		}
		else if(marb[6][4].val==1 && l2tog==1){
			marb[6][4].val=0;
			marb[6][5].val=0;
			l2f=0;
		}
		}
		else if(l2f==1){
			l2tog=1;
		}
		else if(l2f==0){
			l2tog=0;
		}
		if(marb[r1][l1].val==3 && marb[r2][l2].val==3){
		if(marb[6][10].val==0 && l2togl==0){
			marb[6][10].val=1;
			marb[6][11].val=1;
			l2r=1;
		}
		else if(marb[6][10].val==1  && l2togl==1){
			marb[6][10].val=0;
			marb[6][11].val=0;
			l2r=0;
		}
		}
		else if(l2r==1){
			l2togl=1;
		}
		else if(l2r==0){
			l2togl=0;
		}
	}
	if(flag==4){
		if(marb[r1][l1].val==5 && marb[r2][l2].val==5){
		marb[6][8].val=1;
		marb[6][9].val=1;
		marb[7][8].val=1;
		marb[7][9].val=1;
		}
		else if((marb[r1][l1].val==6 || marb[r2][l2].val==6)){
		marb[6][8].val=0;
		marb[6][9].val=0;
		marb[7][8].val=0;
		marb[7][9].val=0;

	}

	}
if(flag==6){

Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(9+1)*6-30, 0.0f, 0.0f+(5+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(brow_cub);
		if(marb[r1][l1].val==2 && marb[r2][l2].val==2){
			marb[7][3].val=1;
	}
}
	if(flag==7){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(4+1)*6-30, 0.0f, 0.0f+(5+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(blu_cub);
				if(marb[r1][l1].val==7 && marb[r2][l2].val==7){
			cubx1+=36;
			cubx2+=36;
			cuby1-=6;
			cubz1+=18;
			cubz2-=18;
			lv8_flg=1;
	}
	if(r1==5 && l1==11 && r2==5 && l2==12)
		lv8_flg=0;
	else if(r2==5 && l2==12)
		lv8_flg=2;
	}

	if(flag==8){
		Matrices.model = glm::mat4(1.0f);

glm::mat4 transTri1 = glm::translate (glm::vec3(0.0f+(13+1)*6-30, 0.0f, 0.0f+(4+1)*6-30)); // glTranslatef
				glm::mat4 rotTri1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));
				glm::mat4 scalTri1 = glm::scale (glm::vec3(1.5f, 0.4f, 1.5f)); // glTranslatef
				// rotate about vector (1,0,0)
				glm::mat4 tri_prod1 = transTri1 * rotTri1*scalTri1;
				Matrices.model *= tri_prod1;
				MVP = VP * Matrices.model; // MVP = p * V * M

				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(blu_cub);
				if(marb[r1][l1].val==7 && marb[r2][l2].val==7){
			cubx1-=6;
			cubx2-=66;
			cuby2-=6;
			//cubz1+=18;
			//cubz2-=18;
			lv8_flg=1;
	}
	if(r1==5 && l1==7 && r2==4 && l2==7)
		lv8_flg=0;
	else if(r2==4 && l2==7)
		lv8_flg=2;
	}


	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();


	// Increment angles
	float increments = 1;

	stringstream ss1;
	ss1 << flag;
	string str3 = ss1.str();
	int digits = str3.length();
	float seg_x=-7;

	for(int i=0;i<digits;i++){
		set_segments(0,str3[i]);
		for(int j=0;j<9;j++){
			if(segment[j]->status){
				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateSegment = glm::translate (glm::vec3(seg_x+segment[j]->x,-7+segment[j]->y,0));
				Matrices.model *= translateSegment;
				glm::mat4 VP1 = Matrices.projection1 * glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
				MVP = VP1* Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(segment[j]);
			}
		}
		seg_x+=1;
	}

	stringstream ss2;
	ss2 << totalscore+currentscore;
	str3 = ss2.str();
	digits = str3.length();
	seg_x=+6;

	for(int i=0;i<digits;i++){
		set_segments(0,str3[i]);
		for(int j=0;j<9;j++){
			if(segment[j]->status){
				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateSegment = glm::translate (glm::vec3(seg_x+segment[j]->x,-7+segment[j]->y,0));
				Matrices.model *= translateSegment;
				glm::mat4 VP1 = Matrices.projection1 * glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
				MVP = VP1* Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				draw3DObject(segment[j]);
			}
		}
		seg_x+=1;
	}


	//camera_rotation_angle++; // Simulating camera rotation
	// triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	Color blue = {0,0,1};
	Color dblue = {0.258824,0.258824,0.435294};
	Color green = {0,1,0};
	Color dgreen = {0.184314,0.309804,0.184314};
	Color red = {1,0,0};
	Color purple = {1.0,0.0,1.0};
	Color grey = {168.0/255.0,168.0/255.0,168.0/255.0};
	Color gold = {218.0/255.0,165.0/255.0,32.0/255.0};
	Color silver = {0.90,0.91,0.98};
	Color orange = {1,0.5,0.0};
	Color yellow = {1,1,0};
	Color skyblue = {132/255.0,217/255.0,245/255.0};
	Color black = {0,0,0};
	Color white = {1,1,1};
	Color hyu = {0.8,0.8,0.6};
	Color brown = {0.6,0.36,0.26};
	Color darkwood = {0.52,0.37,0.26};
	Color maroon = {0.556863,0.137255,0.419608};
	Color scarlet = {0.55,0.09,0.09};
	Color chocolate = {0.36,0.20,0.09};

	createCuboid(-1,-1,darkwood,"cube1",2.0,-2.0);
	createCuboid(-1,-1,darkwood,"cube2",2.0,-2.0);
	createCuboid(-1,-1,brown,"brow_cub",2.0,-2.0);
	createCuboid(-1,-1,blue,"blu_cub",2.0,-2.0);
	createCuboid(-1,-1,gold,"red_cub",2.0,-2.0);
	createCuboid(-1,-1,scarlet,"blac_cub",2.0,-2.0);
	for(int i=0;i<10;i++){
		for(int j=0;j<15;j++){
			if((i+j)%2==0)
			   createCuboid(i,j,orange,"marbles",2.0,-2.0);
			else
			   createCuboid(i,j,white,"marbles",2.0,-2.0);
		}
}
		float hh,ww,x_offset,y_offset;
		for(int j=0;j<18;j++){
			int i=j%9;
			x_offset=0;
			if(i>=0 && i<=5){
				hh=0.5;
				ww=0.1;
			}
			else{
				hh=0.1;
				ww=0.7;
			}
			if(i>=0 && i<=2)
				y_offset=0.25;
			if(i>=3 && i<=5)
				y_offset=-0.25;
			if(i==6)
				y_offset=0.5;
			if(i==7)
				y_offset=0;
			if(i==8)
				y_offset=-0.5;
			if(i==0 || i==3)
				x_offset=-0.35;
			if(i==2 || i==5)
				x_offset=0.35;

			createSegment(hh,ww,x_offset,y_offset,0);
		}



	// createCircle();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.90,0.91,0.98, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1500;
	int height = 800;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
	}

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
