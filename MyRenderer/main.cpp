// TODO check if x64 is bettter

// TODO create camera class
// TODO create renderer class


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include <Globals.h>
#include <Mesh.h>
#include <shaders.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

Mesh* monkey;
GLuint phong_shader_program;

struct Camera
{
	vec3 eye;
	vec3 up;
	vec3 center;

	mat4 projection;
	mat4 view;

	GLuint loc_projection;
	GLuint loc_view;
}camera;

void init();
void init_phong_shaders(const char* vs, const char* fs);
void init_camera(float fovy, float aspect, float near, float far);
void render();

// Initial function
int main(int argc, char* argv[])
{	
	// Initialize GLUT
	glutInit(&argc, argv);

	// Create context
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// Create window
	glutCreateWindow("Renderer Window");
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(Globals::WIDTH, Globals::HEIGHT);
	glutReshapeWindow(Globals::WIDTH, Globals::HEIGHT);

	// Initialize Glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Unable to initalize Glew ! " << std::endl;
		return 1;
	}

	init();

	// Function bindings
	glutDisplayFunc(render);

	glutMainLoop();

	return 0;
}


// Initialize the parameters
void init()
{
	// Load monkey mesh
	monkey = new Mesh("monkey/monkey.obj");
	Globals::Log("Monkey mesh has been loaded.");

	// Initialize shaders
	init_phong_shaders("shaders/vs.glsl", "shaders/fs.glsl");
	Globals::Log("Phong shaders has been compiled.");

	// Set camera parameters
	init_camera(
		60.f,
		Globals::WIDTH / Globals::HEIGHT,
		0.01f,
		100.f
	);
	Globals::Log("Camera parameters has been set.");
}

// Initialize the phong shading shaders
void init_phong_shaders(const char* vs, const char* fs)
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, vs);
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, fs);
	phong_shader_program = initprogram(vertex_shader, fragment_shader);
}

// Initialize camera parameters
// fovy is degree (not radians)
void init_camera(float fovy, float aspect, float _near, float _far)
{
	camera.eye = vec3(0.f, 0.f, -5.f);
	camera.up = vec3(0.f, 1.f, 0.f);
	camera.center = vec3(0.f, 0.f, 0.f);

	camera.projection = glm::perspective(glm::radians(fovy), aspect, _near, _far);
	camera.view = glm::lookAt(camera.eye, camera.center, camera.up);
}

// Renders the scene
/*
* TODO There will be a renderer class instead of this spaghetti code
*/
void render()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.9f, 0.9f, 0.9f, 1.f);

	// Phong shading shader
	glUseProgram(phong_shader_program);

	// Draw camera
	GLuint loc_proj = glGetUniformLocation(phong_shader_program, "projection");
	GLuint loc_view = glGetUniformLocation(phong_shader_program, "view");
	glUniformMatrix4fv(loc_proj, 1, GL_FALSE, &(camera.projection)[0][0]);
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, &(camera.view)[0][0]);
	
	// Draw_model
	GLuint loc_model = glGetUniformLocation(phong_shader_program, "model");
	glUniformMatrix4fv(loc_model, 1, GL_FALSE, &(mat4(1.f))[0][0]);
	glBindVertexArray(monkey->get_VAO());
	glDrawArrays(GL_TRIANGLES, 0, monkey->get_triangle_count() * 3);
	glBindVertexArray(0);

	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));

	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
