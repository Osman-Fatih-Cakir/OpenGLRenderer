
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <Window.h>
#include <Camera.h>
#include <Globals.h>
#include <Mesh.h>
#include <init_shaders.h>
#include <DirectionalLight.h>
#include <PointLight.h>
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
const int point_light_count = 8;
const int direct_light_count = 1;
const int sphere_count = 16;

// Window
Window* window = nullptr;

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
	window->create_window(Globals::WIDTH, Globals::HEIGHT);
	
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
	Globals::Log("*****************Start************************");

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
	case 'q':
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
	scene->camera->set_camera_projection(Globals::PERSPECTIVE);
	glViewport(0, 0, w, h); // Set the viewport
}

// Initailize spheres
void init_meshes()
{
	// Scene meshes
	vec3 cords[16] = {
		vec3(6.f, 0.f, 6.f), vec3(6.f, 0.f, 2.f), vec3(6.f, 0.f, -2.f), vec3(6.f, 0.f, -6.f),
		vec3(2.f, 0.f, 6.f), vec3(2.f, 0.f, 2.f), vec3(2.f, 0.f, -2.f), vec3(2.f, 0.f, -6.f),
		vec3(-2.f, 0.f, 6.f), vec3(-2.f, 0.f, 2.f), vec3(-2.f, 0.f, -2.f), vec3(-2.f, 0.f, -6.f),
		vec3(-6.f, 0.f, 6.f), vec3(-6.f, 0.f, 2.f), vec3(-6.f, 0.f, -2.f), vec3(-6.f, 0.f, -6.f)
	};
	for (int i = 0; i < sphere_count; i++)
	{
		Mesh* mesh = new Mesh("mesh/cube.obj", "mesh/brick.jpg");
		mesh->translate_mesh(cords[i]);
		scene->add_mesh(mesh);
	}

	// Floor
	Mesh* plane = new Mesh("mesh/plane.obj", "mesh/wood.png");
	plane->translate_mesh(vec3(0.f, -1.f, 0.f));
	plane->scale_mesh(vec3(10.f, 1.f, 10.f));
	scene->add_mesh(plane);
}

// Initialize camera
void init_camera(vec3 eye, vec3 up, vec3 center)
{
	Camera* camera = new Camera(eye, up, center, Globals::PERSPECTIVE);
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
		// Set positions of the lights
		vec3 _position = vec3(
			((rand() % 100) / 100.f) * 20.f - 10.f,
			((rand() % 100) / 100.f) * 3.f + 1.f,
			((rand() % 100) / 100.f) * 20.f - 10.f
		);

		// Set colors of the lights
		vec3 _color = vec3(
			((rand() % 100) / 200.f) + 0.5f,
			((rand() % 100) / 200.f) + 0.5f,
			((rand() % 100) / 200.f) + 0.5f
		);

		// Initialize light
		PointLight* light = new PointLight(_position, _color);

		// Draw a mesh for represent a light
		Mesh* light_mesh = new Mesh("mesh/cube.obj", "NONE");
		light_mesh->translate_mesh(light->position);
		light_mesh->scale_mesh(vec3(0.1f, 0.1f, 0.1f));
		
		light->mesh = light_mesh;

		// Space matrices
		
		// Projection matrix
		light->shadow_projection_far = 500.f;
		mat4 pointlight_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, light->shadow_projection_far);
		// View matrices (for each cube plane)
		light->space_matrices[0] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
		light->space_matrices[1] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(-1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
		light->space_matrices[2] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));
		light->space_matrices[3] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
		light->space_matrices[4] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(0.f, 0.f, 1.f), vec3(0.f, -1.f, 0.f));
		light->space_matrices[5] = pointlight_projection * glm::lookAt(light->position, light->position + vec3(0.f, 0.f, -1.f), vec3(0.f, -1.f, 0.f));
		// Create depth map framebuffer for each light
		light->create_depth_map_framebuffer();
		scene->add_point_light(light);
	}

	//
	//// Directional lights
	//

	for (int i = 0; i < direct_light_count; i++)
	{
		DirectionalLight* light = new DirectionalLight(vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f));
		light->intensity = 0.5f;
		mat4 proj_mat = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 1000.f);
		mat4 view_mat = glm::lookAt(vec3(16, 20, 16), vec3(0, 0, 0), vec3(0, 1, 0));
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
		vec3(11.f, 6.f, 11.f), // Eye
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
}
