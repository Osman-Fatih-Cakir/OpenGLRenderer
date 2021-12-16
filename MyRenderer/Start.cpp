
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
const int point_light_count = 2;
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

// Global variables for application
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
void init_camera(vec3 eye, vec3 center, vec3 up);
void init_lights();
void init_scene();

void render();

// Initial function
int main(int argc, char* argv[])
{
	_CrtDumpMemoryLeaks();
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
	glutKeyboardUpFunc(keyboard_up);
	glutMouseFunc(mouse_click);
	glutPassiveMotionFunc(mouse_passive);
	glutDisplayFunc(render);
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
	Model* model = new Model("mesh/Mandalorian_Helmet/Mandalorian_Helmet.obj");
	model->translate(0.f, 1.4f, 0.f, 1.0f);
	model->scale(0.075f, 0.075f, 0.075f, 1.0f);
	scene->add_model(model);
	/*
	model = new Model("mesh/floor/floor.obj");
	model->translate(0.f, -2.f, 0.f, 1.0f);
	model->scale(20.f, 1.f, 20.f, 1.0f);
	scene->add_model(model);
	*/
	/*
	Model* model = new Model("mesh/ibl_test/sphere.obj");
	model->scale(2.f, 2.f, 2.f, 1.0f);
	scene->add_model(model);
	*/
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
		vec3(6.f, 6.f, -4.f),
		vec3(-7.f, 6.f, -3.f)
	};

	vec3 _colors[] = {
		vec3(1.0f, 0.5f, 0.0f),
		vec3(0.0f, 0.5f, 1.0f)
	};

	// Light positions
	for (int i = 0; i < point_light_count; i++)
	{
		// Initialize light
		PointLight* light = new PointLight(_positions[i], _colors[i]);
		light->intensity = 10.f;
		// Draw a mesh for represent a light
		Model* light_model = new Model("mesh/white_cube/cube.obj"); 
		light_model->scale(1.f, 1.f, 1.f, 1.0f);
		//light_model->scale(5.f, 5.f, 5.f, 1.0f); // This cube is 1:1:1 after scaling it
		light->model = light_model;
		// Create depth map framebuffer for each light
		light->create_shadow();
		// Add light to scene
		scene->add_point_light(light);
	}

	//
	//// Directional lights
	//

	for (int i = 0; i < direct_light_count; i++)
	{
		DirectionalLight* light = new DirectionalLight(vec3(-1.0f, -1.0f, -1.0f), vec3(1.f, 1.f, 1.f));
		light->intensity = 0.9f;
		light->create_shadow(
			-20.f, 20.f, -20.f, 20.f, 0.01f, 1000.f,
			vec3(10.f, 10.f, 10.f), vec3(0.f, 0.f, 0.f), vec3(-1.f, 1.f, -1.f)
		);
		scene->add_direct_light(light);
	}
}

// Initialize the scene
void init_scene()
{
	// Initlialize scene class
	scene = new Scene();
	
	Skybox* skybox = new Skybox("mesh/ibl_test/museum_2k.hdr");
	scene->skybox = skybox;

	// Load sphere mesh
	init_models();

	// Set camera parameters
	init_camera(
		vec3(0.f, 3.f, 8.f), // Eye
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
	// Get delta time
	float delta = timer->get_delta_time();

	renderer->render(delta);
	
	float camera_speed = 7.5f;
	if (input->hold_key(Key::KEY_W))
	{
		scene->camera->translate(0.f, 0.f, camera_speed, delta / 1000);
	}
	if (input->hold_key(Key::KEY_S))
	{
		scene->camera->translate(0.f, 0.f, -camera_speed, delta / 1000);
	}
	if (input->hold_key(Key::KEY_A))
	{
		scene->camera->translate(camera_speed, 0.f, 0.f, delta / 1000);
	}
	if (input->hold_key(Key::KEY_D))
	{
		scene->camera->translate(-camera_speed, 0.f, 0.f, delta / 1000);
	}
	if (input->hold_key(Key::KEY_Q))
	{
		scene->camera->translate(0.f, camera_speed, 0.f, delta / 1000);
	}
	if (input->hold_key(Key::KEY_E))
	{
		scene->camera->translate(0.f, -camera_speed, 0.f, delta / 1000);
	}
	if (input->hold_key(Key::KEY_Z))
	{
		scene->camera->rotate(vec3(0.f, 1.f, 0.f), 2.f, delta / 100);
	}
	if (input->hold_key(Key::KEY_C))
	{
		scene->camera->rotate(vec3(0.f, -1.f, 0.f), 2.f, delta / 100);
	}
	if (input->press_key(Key::KEY_R))
	{
		Skybox* _skybox;
		// TODO use X for this, re assign R for exit_app()
		switch (skybox_id)
		{
		case 0:
			_skybox = new Skybox("mesh/ibl_test/hall_2k.hdr");
			scene->skybox = _skybox;
			break;
		case 1:
			_skybox = new Skybox("mesh/ibl_test/museum_2k.hdr");
			scene->skybox = _skybox;
			break;
		case 2:
			_skybox = new Skybox("mesh/ibl_test/veranda_2k.hdr");
			scene->skybox = _skybox;
			break;
		default:
			break;
		}
		skybox_id++;
		skybox_id = skybox_id % 3;
		//exit_app();
	}
}
