
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <Window.h>
#include <Camera.h>
#include <Model.h>
#include <init_shaders.h>
#include <DirectionalLight.h>
#include <PointLight.h>
#include <InputHandler.h>
#include <Scene.h>
#include <Renderer.h>

////////////////
// Debugging memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define DBG_NEW new
#endif
#ifdef _DEBUG
#define new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define new new
#endif
////////////////

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

// Point lights
const int point_light_count = 1;
const int direct_light_count = 1;

// Window
Window* window = nullptr;

// Input handler
InputHandler* input_handler = nullptr;

// Scene pointer
Scene* scene = nullptr;

// Renderer
Renderer* renderer = nullptr;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
void exit_app();
void keyboard(unsigned char key, int x, int y);
void resize_window(int w, int h);
void init_meshes();
void init_camera(vec3 eye, vec3 center, vec3 up);
void init_lights();
void init_scene();

void render();

// Initial function
int main(int argc, char* argv[])
{	
	Init_Glut_and_Glew(argc, argv);

	// Main function that prepares the scene and program
	init();

	// Start update loop
	glutMainLoop();

	return 0;
}

// Initialize glut and glew
void Init_Glut_and_Glew(int argc, char* argv[])
{
	// Initialize GLUT
	glutInit(&argc, argv);

	// Create context
	glutInitContextVersion(4, 5);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// Create window
	window = new Window();
	window->create_window(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	// Initialize Glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	// TODO After updating glew version, delete this
	GLuint errr = glGetError(); if (errr) fprintf(stderr, "%s\n", gluErrorString(errr));
	if (GLEW_OK != err)
	{
		std::cout << "Unable to initalize Glew ! " << std::endl;
		return;
	}
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Function bindings
	glutReshapeFunc(resize_window);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(render);
}

// Initialize the parameters
void init()
{
	std::cout << "*****************Start************************" << std::endl;

	input_handler = new InputHandler();

	// Initialize scene
	init_scene();
}

// Exit from the application
void exit_app()
{
	// Destroy window
	window->destroy_window();

	// Deallocate all pointers
	delete window;
	delete renderer;

	// Check for leaks
	_CrtDumpMemoryLeaks();

	exit(0);
}

// Keyboard inputs
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		scene->camera->camera_translate(vec3(0.f, 0.f, -0.1f));
		break;
	case 's':
		scene->camera->camera_translate(vec3(0.f, 0.f, 0.1f));
		break;
	case 'd':
		scene->camera->camera_translate(vec3(0.1f, 0.f, 0.f));
		break;
	case 'a':
		scene->camera->camera_translate(vec3(-0.1f, 0.f, 0.f));
		break;
	case 'q':
		scene->camera->camera_translate(vec3(0.f, -0.1f, 0.f));
		break;
	case 'e':
		scene->camera->camera_translate(vec3(0.f, 0.1f, 0.f));
		break;
	case 'z':
		scene->camera->camera_rotate(vec3(0.1f, 0.f, 0.f), -5.f);
		break;
	case 'c':
		scene->camera->camera_rotate(vec3(0.1f, 0.f, 0.f), 5.f);
		break;
	case 'r':
		exit_app();
		break;
	}
}

// Resize window
void resize_window(int w, int h)
{
	// Prevent from zero division
	if (h == 0) h = 1;

	// Recalculate camera projection matrix
	scene->camera->set_camera_projection(PERSPECTIVE);
	glViewport(0, 0, w, h); // Set the viewport
}

// Initailize spheres
void init_meshes()
{
	// Scene meshes
	Model* model = new Model("mesh/Mandalorian_Helmet/Mandalorian_Helmet.obj");
	model->translate(2.f, 0.f, 0.f);
	model->scale(0.03f, 0.03f, 0.03f);
	scene->add_model(model);

	model = new Model("mesh/Mandalorian_Helmet/Mandalorian_Helmet.obj");
	model->translate(-2.f, 0.f, 0.f);
	model->scale(0.03f, 0.03f, 0.03f);
	scene->add_model(model);
}

// Initialize camera
void init_camera(vec3 eye, vec3 up, vec3 center)
{
	Camera* camera = new Camera(eye, up, center, PERSPECTIVE);
	scene->camera = camera;
}

// Initialize light paramters
void init_lights()
{
	//
	//// Point lights 
	//
	srand((unsigned int)time(NULL));

	// Light positions
	for (int i = 0; i < point_light_count; i++)
	{
		vec3 _position = vec3(2.f, 3.f, 3.f);
		vec3 _color = vec3(1.f, 1.f, 1.f);

		// Initialize light
		PointLight* light = new PointLight(_position, _color);
		light->intensity = 20.f;
		// Draw a mesh for represent a light
		Model* light_model = new Model("mesh/white_cube/cube.obj");
		light_model->translate(light->position);
		light->model = light_model;
	
		// Create depth map framebuffer for each light
		light->create_depth_map_framebuffer();
		scene->add_point_light(light);
	}

	//
	//// Directional lights
	//

	for (int i = 0; i < direct_light_count; i++)
	{
		DirectionalLight* light = new DirectionalLight(vec3(-1.0f, -1.0f, -1.0f), vec3(1.f, 1.f, 1.f));
		mat4 proj_mat = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 1000.f);
		mat4 view_mat = glm::lookAt(vec3(5, 5, 5), vec3(0, 0, 0), vec3(-1, 1, -1));
		light->calculate_space_matrix(proj_mat, view_mat);
		// Create depth map framebuffer
		light->create_depth_map_framebuffer();
		scene->add_direct_light(light);
	}
}

// Initialize the scene
void init_scene()
{
	// Initlialize scene class
	scene = new Scene();
	
	// Load sphere mesh
	init_meshes();

	// Set camera parameters
	init_camera(
		vec3(0.f, 0.f, 7.f), // Eye
		vec3(0.f, 1.f, 0.f), // Up
		vec3(0.f, 0.f, 0.f) // Center
	);

	// Initialize lights
	init_lights();

	// Initialize renderer
	renderer = new Renderer(scene);
}

// Renders the scene
void render()
{
	renderer->render();
	//input_handler->update();
}
