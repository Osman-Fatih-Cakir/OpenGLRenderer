
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
#include <Scene.h>
#include <Renderer.h>
#include <Input.h>
#include <Keys.h>
#include <Skybox.h>

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
/*
#define new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define new new
*/
#endif
////////////////

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

// Point lights
const int point_light_count = 3;
const int direct_light_count = 1;

// Window
Window* window = nullptr;

// Input handler
Input* input = nullptr;

// Scene pointer
Scene* scene = nullptr;

// Timer pointer
Timer* timer = nullptr;

// Renderer
Renderer* renderer = nullptr;

// Global variables for test the application
int skybox_id = 0;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
void exit_app();
void keyboard(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);
void mouse_click(int button, int state, int x, int y);
void mouse_passive(int x, int y);
void resize_window(int w, int h);
void init_models();
void init_skyboxes();
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

void GLAPIENTRY MessageCallback(GLenum source, GLenum type,	GLuint id, GLenum severity,
	GLsizei length,	const GLchar* message,	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
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

	if (GLEW_OK != err)
	{
		std::cout << "Unable to initalize Glew ! " << std::endl;
		return;
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Enable linear filtering across the cubemap faces
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Function bindings
	glutReshapeFunc(resize_window);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutMouseFunc(mouse_click);
	glutPassiveMotionFunc(mouse_passive);
	glutDisplayFunc(render);
	//glDebugMessageCallback(MessageCallback, 0);
}

// Initialize the parameters
void init()
{
	std::cout << "*****************Start************************" << std::endl;

	input = new Input();
	timer = new Timer();

	// Initialize scene
	init_scene();
}

// Exit from the application
void exit_app()
{
	// Deallocate all pointers
	delete window;
	delete renderer;
	delete input;
	delete timer;

	// Check for leaks
	_CrtDumpMemoryLeaks();

	exit(0);
}

// Keyboard key press
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		input->add_key(Key::KEY_W);
		break;
	case 's':
		input->add_key(Key::KEY_S);
		break;
	case 'd':
		input->add_key(Key::KEY_D);
		break;
	case 'a':
		input->add_key(Key::KEY_A);
		break;
	case 'q':
		input->add_key(Key::KEY_Q);
		break;
	case 'e':
		input->add_key(Key::KEY_E);
		break;
	case 'z':
		input->add_key(Key::KEY_Z);
		break;
	case 'c':
		input->add_key(Key::KEY_C);
		break;
	case 'x':
		input->add_key(Key::KEY_X);
		break;
	case 'f':
		input->add_key(Key::KEY_F);
		break;
	case 'r':
		input->add_key(Key::KEY_R);
		break;
	}
}

// Keyboard key release 
void keyboard_up(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		input->remove_key(Key::KEY_W);
		break;
	case 's':
		input->remove_key(Key::KEY_S);
		break;
	case 'd':
		input->remove_key(Key::KEY_D);
		break;
	case 'a':
		input->remove_key(Key::KEY_A);
		break;
	case 'q':
		input->remove_key(Key::KEY_Q);
		break;
	case 'e':
		input->remove_key(Key::KEY_E);
		break;
	case 'z':
		input->remove_key(Key::KEY_Z);
		break;
	case 'c':
		input->remove_key(Key::KEY_C);
		break;
	case 'x':
		input->remove_key(Key::KEY_X);
		break;
	case 'f':
		input->remove_key(Key::KEY_F);
		break;
	case 'r':
		input->remove_key(Key::KEY_R);
		break;
	}
}

// Mouse click
void mouse_click(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			input->add_mouse_click(MouseButton::LButton);
		else if (state == GLUT_UP)
			input->remove_mouse_click(MouseButton::LButton);
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
			input->add_mouse_click(MouseButton::MButton);
		else if (state == GLUT_UP)
			input->remove_mouse_click(MouseButton::MButton);
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
			input->add_mouse_click(MouseButton::RButton);
		else if (state == GLUT_UP)
			input->remove_mouse_click(MouseButton::RButton);
		break;
	}
}

// Mouse position
void mouse_passive(int x, int y)
{
	input->set_mouse(x, y);
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
void init_models()
{
	// Scene meshes
	//Model* model = new Model("mesh/test_scene/helmet/DamagedHelmet.gltf", false);
	//model->translate(-15.f, 0.f, 0.f, 1.f);
	//model->scale(10.f, 10.f, 10.f, 1.f);
	//scene->add_model(model);
	//Model* model = new Model("mesh/cornell_box/cornell_box.gltf", false);	
	//model->scale(2.f, 2.f, 2.f, 1.f);
	//scene->add_model(model);
	//Model* model = new Model("mesh/test_scene/sponza/sponza.glb", false);
	//scene->add_model(model);
	//Model* model = new Model("mesh/test_scene/tree/scene.gltf", false);
	//model->rotate(vec3(0.f, 1.f, 0.f), 180.f, 1.f);
	//model->scale(0.5f, 0.5f, 0.5f, 1.f);
	//scene->add_model(model);
	//Model* model = new Model("mesh/test_scene/fuel_glasses/scene.gltf", true);
	//scene->add_model(model);
}

// Initialize skyboxes
void init_skyboxes()
{
	//scene->add_skybox("mesh/ibl_test/hall_2k.hdr", 2, false, 1.0f);
	//scene->add_skybox("mesh/ibl_test/dreifaltigkeitsberg_2k.hdr", 3, false, 1.5f);
	//scene->add_skybox("mesh/ibl_test/dikhololo_night_2k.hdr", 1, false, 0.1f);
	//scene->render_skybox_id(3);
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
	
	//srand((unsigned int)time(NULL));

	vec3 _positions[] = {
		vec3(0.f, 12.f, -5.f),
		vec3(0.f, 12.f, 5.f),
		vec3(3.8f, 8.f, 2.6f)
	};

	// Color sequential
	vec3 _colors[] = {
		vec3(0.9f, 0.9f, 0.3f),
		vec3(0.9f, 0.3f, 0.9f),
		vec3(0.9f, 0.9f, 0.9f)
	};

	// Light positions
	int color_index = 0;
	for (int i = 0; i < point_light_count; i++)
	{
		// Initialize light
		PointLight* light = new PointLight(_positions[i], _colors[i], false);
		light->set_intensity(0.f);
		std::cout << "Radius: " << light->radius << "\n";
		// Draw a mesh for represent a light
		Model* light_model = new Model("mesh/simple/sphere.obj", false);
		light->set_model(light_model);
		//Model* light_debug_model = new Model("mesh/simple/icosphere.obj");
		//light->debug(light_debug_model);
		// Add light to scene
		scene->add_point_light(light);
	}

	//
	//// Directional lights
	//

	for (int i = 0; i < direct_light_count; i++)
	{
		DirectionalLight* light = new DirectionalLight(
			vec3(-1.0f, -10.0f, -1.0f),
			vec3(1.f, 1.f, 0.9f), true);
		light->intensity = 1.f;
		scene->add_direct_light(light);
	}
}

// Initialize the scene
void init_scene()
{
	// Initlialize scene class
	scene = new Scene();
	
	init_skyboxes();

	// Load sphere mesh
	init_models();

	// Set camera parameters
	init_camera(
		vec3(15.f, 11.f, 0.f), // Eye
		vec3(0.f, 1.f, 0.f), // Up
		vec3(0.f, 6.f, 0.f) // Center
	);
	
	// Initialize lights
	init_lights();

	// Initialize renderer
	renderer = new Renderer(scene);
}

// Renders the scene
void render()
{
	// Get delta time
	float delta = timer->get_delta_time();

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	renderer->render(delta);

	// Camera actions
	float camera_speed = 1.f;
	float delta_over_t = delta / 1000;
	float delta_over_h = delta / 100;
	Camera* cam = scene->camera;
	if (input->hold_key(Key::KEY_W))
	{
		cam->translate(camera_speed, 0.f, 0.f, delta_over_t);
	}
	if (input->hold_key(Key::KEY_S))
	{
		cam->translate(-camera_speed, 0.f, 0.f, delta_over_t);
	}
	if (input->hold_key(Key::KEY_A))
	{
		cam->translate(0.f, 0.f, -camera_speed, delta_over_t);
	}
	if (input->hold_key(Key::KEY_D))
	{
		cam->translate(0.f, 0.f, camera_speed, delta_over_t);
	}
	if (input->hold_key(Key::KEY_Q))
	{
		cam->translate(0.f, -camera_speed, 0.f, delta_over_t);
	}
	if (input->hold_key(Key::KEY_E))
	{
		cam->translate(0.f, camera_speed, 0.f, delta_over_t);
	}
	if (input->hold_key(Key::KEY_Z))
	{
		cam->rotate(vec3(0.f, 0.f, -1.f), 2.f*0.1f, delta_over_t);
	}
	if (input->hold_key(Key::KEY_C))
	{
		cam->rotate(vec3(0.f, -1.f, 0.f), 2.f*0.1f, delta_over_h);
	}
	if (input->press_key(Key::KEY_F))
	{
		if (scene->get_render_skybox() != nullptr)
			scene->get_render_skybox()->toggle_IBL();
	}
	if (input->press_key(Key::KEY_X))
	{
		if (scene->get_render_skybox() != nullptr)
		{
			scene->render_skybox_id(skybox_id + 1);
			skybox_id++;
			skybox_id = skybox_id % 3;
		}
	}
	if (input->press_key(Key::KEY_R))
	{
		exit_app();
	}

	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "ERROR: %s\n", gluErrorString(err));

	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
