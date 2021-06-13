
// TODO implement deferred shading
// TODO After that, check what I need


// TODO create uniform classes
// TODO create shader classes
// TODO create renderer class


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include <Camera.h>
#include <Globals.h>
#include <Mesh.h>
#include <init_shaders.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

Camera* camera;

Mesh* monkey;
GLuint shader_program;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
void init_shaders(const char* vs, const char* fs);
void init_camera(vec3 eye, vec3 center, vec3 up, GLuint program);
void render();

// Initial function
int main(int argc, char* argv[])
{	
	Init_Glut_and_Glew(argc, argv);

	//
	//// Main function that prepares the scene and program
	//
	init();

	//
	//// Start update loop
	//
	glutMainLoop();

	return 0;
}

// Initialize glut and glew
void Init_Glut_and_Glew(int argc, char* argv[])
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
		return;
	}

	// Function bindings
	glutDisplayFunc(render);
}

// Initialize the parameters
void init()
{
	// Load monkey mesh
	monkey = new Mesh("monkey/monkey.obj");
	Globals::Log("Monkey mesh has been loaded.");

	// Initialize shaders
	init_shaders("shaders/vs.glsl", "shaders/fs.glsl");
	Globals::Log("Shaders has been compiled.");

	// Set camera parameters
	init_camera(
		vec3(0.f, 2.f, -5.f), // Eye
		vec3(0.f, 1.f, 0.f), // Up
		vec3(0.f, 0.f, 0.f), // Center
		shader_program
	);
	Globals::Log("Camera parameters has been set.");
}

// Initialize the shading shaders
void init_shaders(const char* vs, const char* fs)
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, vs);
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, fs);
	shader_program = initprogram(vertex_shader, fragment_shader);
}

// Initialize camera
void init_camera(vec3 eye, vec3 up, vec3 center, GLuint program)
{
	camera = new Camera(eye, up, center, Globals::PERSPECTIVE);
}

// Renders the scene
void render()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.9f, 0.9f, 0.9f, 1.f);

	// Shading shader
	glUseProgram(shader_program);

	// Draw camera
	GLuint loc_proj = glGetUniformLocation(shader_program, "projection");
	GLuint loc_view = glGetUniformLocation(shader_program, "view");
	glUniformMatrix4fv(loc_proj, 1, GL_FALSE, &(camera->get_projection_matrix())[0][0]);
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, &(camera->get_view_matrix())[0][0]);
	
	// Draw_model
	GLuint loc_model = glGetUniformLocation(shader_program, "model");
	glUniformMatrix4fv(loc_model, 1, GL_FALSE, &(mat4(1.f))[0][0]);
	glBindVertexArray(monkey->get_VAO());
	glDrawArrays(GL_TRIANGLES, 0, monkey->get_triangle_count() * 3);
	glBindVertexArray(0);

	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));

	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
