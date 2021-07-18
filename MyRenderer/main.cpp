
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include <Camera.h>
#include <Globals.h>
#include <Mesh.h>
#include <init_shaders.h>
#include <DirectionalLight.h>
#include <PointLight.h>
#include <gBuffer.h>
#include <DirectionalDepth.h>
#include <PointDepth.h>
#include <ForwardRender.h>

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

// Timing
float old_time = 0.f;
float cur_time = 0.f;

// Point lights
const int point_light_count = 8;
std::vector<PointLight*> point_lights;

const int direct_light_count = 1;
std::vector<DirectionalLight*> direct_lights;

// Camera
Camera* camera;

// Meshes
int sphere_count = 16;
std::vector<Mesh*> spheres;
std::vector<Mesh*> planes;

// VAOs
GLuint quad_VAO;

// Shader programs
GLuint deferred_shading_program;

gBuffer* GBuffer = nullptr;
DirectionalDepth* dirDepth = nullptr;
PointDepth* pointDepth = nullptr;
ForwardRender* forwardRender = nullptr;

// Window
unsigned int win_id;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
void exit_app();
void keyboard(unsigned char key, int x, int y);
void resize_window(int w, int h);
void init_meshes();
void init_shaders();

void init_camera(vec3 eye, vec3 center, vec3 up);
void init_quad();
void init_gBuffer();
void init_lights();
void init_depth_map();
void init_point_depth_maps();
void init_shader_programs();

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
	glutInitContextVersion(4, 5);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// Create window
	win_id = glutCreateWindow("Renderer Window");
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(Globals::WIDTH, Globals::HEIGHT);
	glutReshapeWindow(Globals::WIDTH, Globals::HEIGHT);
	
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

	// Initialize shaders
	init_shaders();

	// Load sphere mesh
	init_meshes();

	// Set camera parameters
	init_camera(
		vec3(11.f, 6.f, 11.f), // Eye
		vec3(0.f, 1.f, 0.f), // Up
		vec3(0.f, 0.f, 0.f) // Center
	);

	// Initialize gBuffer textures and render buffer for depth
	init_gBuffer();

	// Initialize quad that will be rendered with texture (The scene texture)
	init_quad();

	// Initialize lights
	init_lights();

	// Initialize depth maps
	init_depth_map();

	// Init point depth maps
	init_point_depth_maps();

	// Initialize shader program classes
	init_shader_programs();

	// Get delta time
	old_time = (float)glutGet(GLUT_ELAPSED_TIME);
}

// TODO exit the app
// Exit from the application
void exit_app()
{
	//
	//// Deallocate all pointers
	//
}

// Keyboard inputs
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		glutDestroyWindow(win_id);
		_CrtDumpMemoryLeaks();
		exit(0);
		break;
	}
}

// Resize window
void resize_window(int w, int h)
{
	// Prevent from zero division
	if (h == 0) h = 1;

	// Recalculate camera projection matrix
	camera->set_camera_projection(Globals::PERSPECTIVE);
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
		spheres.push_back(mesh);
	}

	// Floor
	Mesh* plane = new Mesh("mesh/plane.obj", "mesh/wood.png");
	plane->translate_mesh(vec3(0.f, -1.f, 0.f));
	plane->scale_mesh(vec3(10.f, 1.f, 10.f));
	planes.push_back(plane);
}

// Initialize and compiling shaders
void init_shaders()
{
	// Deferred lighting shaders
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_shading_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_shading_fs.glsl");
	deferred_shading_program = initprogram(vertex_shader, fragment_shader);
}

// Initialize camera
void init_camera(vec3 eye, vec3 up, vec3 center)
{
	camera = new Camera(eye, up, center, Globals::PERSPECTIVE);
}

// Initialize a quad
void init_quad()
{
	float quadVertices[] = {

		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 4
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3

		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 1
	};

	glGenVertexArrays(1, &quad_VAO);

	// Setup quad VAO
	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quad_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

// Initialize gBuffers
void init_gBuffer()
{
	GBuffer = new gBuffer();
	// Set resolution
	GBuffer->set_gBuffer_resolution(Globals::WIDTH, Globals::HEIGHT);
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

		point_lights.push_back(light);
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
		direct_lights.push_back(light);
	}
}

// Initialize depth maps and the framebuffer
void init_depth_map()
{
	// Directional light depths
	for (int i = 0; i < direct_lights.size(); i++)
	{
		// Create depth map framebuffer for each light
		direct_lights[i]->create_depth_map_framebuffer();
	}

	// Initialize depth map program
	dirDepth = new DirectionalDepth();

}

// Initialize depth maps with framebuffer
void init_point_depth_maps()
{
	// Point light depths
	for (int i = 0; i < point_light_count; i++)
	{
		// Create depth map framebuffer for each light
		point_lights[i]->create_depth_map_framebuffer();
	}

	// Initialize point depth
	pointDepth = new PointDepth();
}

// Initialize shader program classes
void init_shader_programs()
{
	forwardRender = new ForwardRender();
	forwardRender->change_viewport_resolution(Globals::WIDTH, Globals::HEIGHT );
}

// Renders the scene
void render()
{
	// Get delta time
	cur_time = (float)glutGet(GLUT_ELAPSED_TIME);
	float delta = cur_time - old_time;
	old_time = cur_time;

	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//

	// Start gBuffer program
	GBuffer->start_program(); 
	
	camera->camera_rotate(vec3(0.f, 1.f, 0.f), delta / 10000); // Camera rotation smoothly
	// Set camera attributes
	GBuffer->set_projection_matrix(camera->get_projection_matrix());
	GBuffer->set_view_matrix(camera->get_view_matrix());

	// Draw scene
	for (int i = 0; i < sphere_count; i++) // Cubes
	{
		// Set model attributes
		GBuffer->set_model_matrix(spheres[i]->get_model_matrix());
		GBuffer->set_normal_matrix(spheres[i]->get_normal_matrix());
		GBuffer->set_diffuse_texture(spheres[i]->get_texture_id());
		// Draw
		GBuffer->render(spheres[i]->get_VAO(), spheres[i]->get_triangle_count()*3);
	}
	for (int i = 0; i < planes.size(); i++) // Planes
	{
		// Set model attributes
		GBuffer->set_model_matrix(planes[i]->get_model_matrix());
		GBuffer->set_normal_matrix(planes[i]->get_normal_matrix());
		GBuffer->set_diffuse_texture(planes[i]->get_texture_id());
		// Draw
		GBuffer->render(planes[i]->get_VAO(), planes[i]->get_triangle_count() * 3);
	}

	// Attach depth buffer to default framebuffer
	GBuffer->attach_depthbuffer_to_framebuffer(0);

	// Attach default framebuffer for no prevent further modifications
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	//// Shadow pass: Get the depth map of the scene
	//

	// Directional shadows
	dirDepth->start_program();
	for (int i = 0; i < direct_light_count; i++)
	{
		glViewport(0, 0, direct_lights[i]->depth_map_width, direct_lights[i]->depth_map_height);
		glBindFramebuffer(GL_FRAMEBUFFER, direct_lights[i]->depth_map_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Set space matrix
		dirDepth->set_space_matrix(direct_lights[i]->space_matrix);
		// Draw scene
		for (int ii = 0; ii < spheres.size(); ii++) // Cubes
		{
			// Set model matrix
			dirDepth->set_model_matrix(spheres[ii]->get_model_matrix());
			// Draw
			dirDepth->render(spheres[i]->get_VAO(), spheres[ii]->get_triangle_count() * 3);
		}
		for (int ii = 0; ii < planes.size(); ii++) // Planes
		{
			// Set model matrix
			dirDepth->set_model_matrix(planes[ii]->get_model_matrix());
			// Draw
			dirDepth->render(planes[ii]->get_VAO(), planes[ii]->get_triangle_count() * 3);
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Point light shadows
	pointDepth->start_program();
	for (int i = 0; i < point_lights.size(); i++)
	{
		glViewport(0, 0, point_lights[i]->depth_map_width, point_lights[i]->depth_map_height); // Use shadow resolutions
		glBindFramebuffer(GL_FRAMEBUFFER, point_lights[i]->depth_cubemap_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Draw scene
		for (int ii = 0; ii < spheres.size(); ii++) // Cubes
		{
			// Set model matrix
			pointDepth->set_model_matrix(spheres[ii]->get_model_matrix());
			// Set space matrices
			pointDepth->set_space_matrices(point_lights[i]->space_matrices);
			// Set far
			pointDepth->set_far(point_lights[i]->shadow_projection_far);
			// Set position
			pointDepth->set_position(point_lights[i]->position);
			// Draw
			pointDepth->render(spheres[ii]->get_VAO(), spheres[ii]->get_triangle_count() * 3);
		}
		for (int ii = 0; ii < planes.size(); ii++) // Planes
		{
			// Set model matrix
			pointDepth->set_model_matrix(planes[ii]->get_model_matrix());
			// Set space matrices
			pointDepth->set_space_matrices(point_lights[i]->space_matrices);
			// Set far
			pointDepth->set_far(point_lights[i]->shadow_projection_far);
			// Set position
			pointDepth->set_position(point_lights[i]->position);
			// Draw
			pointDepth->render(planes[ii]->get_VAO(), planes[ii]->get_triangle_count() * 3);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//
	//// 2. Lighting Pass: Calculate lighting pixel by pixel using gBuffer
	//

	glViewport(0, 0, Globals::WIDTH, Globals::HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Deferred shader program
	glUseProgram(deferred_shading_program);

	// Bind uniforms
	GLuint loc_viewer_pos = glGetUniformLocation(deferred_shading_program, "viewer_pos");
	glUniform3fv(loc_viewer_pos, 1, &(camera->get_eye())[0]);

	// Bind texture with gBuffer data
	GLuint loc_gPos = glGetUniformLocation(deferred_shading_program, "gPosition");
	GLuint loc_gNor = glGetUniformLocation(deferred_shading_program, "gNormal");
	GLuint loc_gAlb = glGetUniformLocation(deferred_shading_program, "gAlbedoSpec");
	glUniform1i(loc_gPos, 0);
	glUniform1i(loc_gNor, 1);
	glUniform1i(loc_gAlb, 2);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, GBuffer->get_gPosition());
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, GBuffer->get_gNormal());
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, GBuffer->get_gAlbedoSpec());
	// Bind point light uniforms
	int last = 0;
	for (int i = 0; i < point_light_count; i++)
	{
		std::string light_array_str = "point_lights[" + std::to_string(i) + "].position";
		GLuint loc_lights_pos = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, point_lights[i]->get_position_pointer());

		light_array_str = "point_lights[" + std::to_string(i) + "].color";
		GLuint loc_lights_col = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_col, 1, point_lights[i]->get_color_pointer());

		light_array_str = "point_lights[" + std::to_string(i) + "].radius";
		GLuint loc_light_r = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_r, (GLfloat)point_lights[i]->radius);

		light_array_str = "point_lights[" + std::to_string(i) + "].linear";
		GLuint loc_light_l = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_l, (GLfloat)point_lights[i]->linear);

		light_array_str = "point_lights[" + std::to_string(i) + "].quadratic";
		GLuint loc_light_q = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_q, point_lights[i]->quadratic);

		light_array_str = "point_lights[" + std::to_string(i) + "].far";
		GLuint loc_light_far = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_far, (GLfloat)point_lights[i]->shadow_projection_far);
		
		light_array_str = "point_lights[" + std::to_string(i) + "].point_shadow_map";
		GLuint loc_light_cubemap = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1i(loc_light_cubemap, 3 + i);
		glActiveTexture(GL_TEXTURE0 + 3 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, point_lights[i]->depth_cubemap);

		light_array_str = "point_lights[" + std::to_string(i) + "].intensity";
		GLuint loc_light_int = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_int, (GLfloat)point_lights[i]->intensity);

		last = 3 + i + 1; // TODO change the name of the variable
	}

	// Bind directional light uniforms
	for (int i = 0; i < direct_light_count; i++)
	{
		std::string light_array_str = "direct_lights[" + std::to_string(i) + "].color";
		GLuint loc_lights_pos = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, direct_lights[i]->get_color_pointer());

		light_array_str = "direct_lights[" + std::to_string(i) + "].direction";
		GLuint loc_lights_dir = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_dir, 1, direct_lights[i]->get_direction_pointer());

		light_array_str = "direct_lights[" + std::to_string(i) + "].intensity";
		GLuint loc_lights_in = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_lights_in, direct_lights[i]->intensity);

		light_array_str = "direct_lights[" + std::to_string(i) + "].directional_shadow_map";
		GLuint loc_lights_sm = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1i(loc_lights_sm, last + i);
		glActiveTexture(GL_TEXTURE0 + last + i);
		glBindTexture(GL_TEXTURE_2D, direct_lights[i]->depth_map);
		
		light_array_str = "direct_lights[" + std::to_string(i) + "].light_space_matrix";
		GLuint loc_lights_lsm = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniformMatrix4fv(loc_lights_lsm, 1, GL_FALSE, direct_lights[0]->get_space_matrix_pointer());
	}

	// Draw quad using gBuffer color data
	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//
	//// 3. Pass: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	//
	
	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer->get_fbo());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, GBuffer->get_width(), GBuffer->get_height(), 0, 0,
		GBuffer->get_width(), GBuffer->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Start forward rendering program
	forwardRender->start_program();

	// Set camera attributes
	forwardRender->set_projection_matrix(camera->get_projection_matrix());
	forwardRender->set_view_matrix(camera->get_view_matrix());

	// Draw scene
	for (int i = 0; i < point_light_count; i++)
	{
		forwardRender->set_model_matrix(point_lights[i]->mesh->get_model_matrix());
		forwardRender->set_color(point_lights[i]->color);
		forwardRender->render(point_lights[i]->mesh->get_VAO(), point_lights[i]->mesh->get_triangle_count() * 3);
	}
	
	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));
	
	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
