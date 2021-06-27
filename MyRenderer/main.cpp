
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

// Timing
float old_time = 0.f;
float cur_time = 0.f;

const int light_count = 8;
struct Light
{
	vec3 position;
	vec3 color;
	float radius;
	float linear;
	float quadratic;
}lights[light_count];

// Camera
Camera* camera;

// Meshes
int sphere_count = 16;
std::vector<Mesh*> spheres;
std::vector<Mesh*> light_meshes;
std::vector<Mesh*> planes;

// VAOs
GLuint quad_VAO;

// Shader programs
GLuint gBuffer_program, deferred_shading_program, deferred_unlit_meshes_program;

// Framebuffers
GLuint gBuffer;

// Textures
GLuint gPosition, gNormal, gAlbedoSpec;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
void resize_window(int w, int h);
void init_meshes();
void init_shaders();
void init_camera(vec3 eye, vec3 center, vec3 up);
void init_quad();
void init_gBuffer();
void init_lights();

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
	glutCreateWindow("Renderer Window");
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
	// Cull backfaces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	

	// Function bindings
	glutReshapeFunc(resize_window);
	glutDisplayFunc(render);
}

// Initialize the parameters
void init()
{
	Globals::Log("*****************Start************************");
	// Load sphere mesh
	init_meshes();

	// Initialize shaders
	init_shaders();

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

	// Get delta time
	old_time = (float)glutGet(GLUT_ELAPSED_TIME);
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
	// gBuffer shaders
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/gbuffer_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/gbuffer_fs.glsl");
	gBuffer_program = initprogram(vertex_shader, fragment_shader);

	// Deferred lighting shaders
	vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_shading_vs.glsl");
	fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_shading_fs.glsl");
	deferred_shading_program = initprogram(vertex_shader, fragment_shader);

	// Deferred unlit meshes shaders
	vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_unlit_meshes_vs.glsl");
	fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_unlit_meshes_fs.glsl");
	deferred_unlit_meshes_program = initprogram(vertex_shader, fragment_shader);
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
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Globals::WIDTH, Globals::HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// Normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Globals::WIDTH, Globals::HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// Color + Speculer color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Globals::WIDTH, Globals::HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	
	// Create and attach depth buffer (renderbuffer)
	GLuint rbo_depth;
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Globals::WIDTH, Globals::HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Globals::Log("Framebuffer not complete!");
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Prevent further modification
}

// Initialize light paramters
void init_lights()
{
	srand((unsigned int)time(NULL));

	// Light positions
	for (int i = 0; i < light_count; i++)
	{
		// Set positions of the lights
		lights[i].position = vec3(
			((rand() % 100) / 100.f) * 20.f - 10.f,
			((rand() % 100) / 100.f) * 3.f + 1.f,
			((rand() % 100) / 100.f) * 20.f - 10.f
		);
		// Set colors of the lights
		lights[i].color = vec3(
			((rand() % 100) / 200.f) + 0.5f,
			((rand() % 100) / 200.f) + 0.5f,
			((rand() % 100) / 200.f) + 0.5f
		);
		// Draw a mesh for represent a light
		Mesh* light_mesh = new Mesh("mesh/cube.obj", "NONE");
		light_mesh->translate_mesh(lights[i].position);
		light_mesh->scale_mesh(vec3(0.1f, 0.1f, 0.1f));
		light_meshes.push_back(light_mesh);

		// Calculate light radius
		float constant = 1.0f;
		float linear = 0.7f;
		float quadratic = 1.8f;
		float lightMax = std::fmaxf(std::fmaxf(lights[i].color.r, lights[i].color.g), lights[i].color.b);
		float radius = (float)(-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
			/ (2 * quadratic);
		lights[i].radius = radius;

		lights[i].linear = 0.2f;
		lights[i].linear = 0.7f;
	}
}

// Renders the scene
void render()
{
	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//
	
	// Bind framebuffer to gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.f, 1.f);

	// gBuffer program
	glUseProgram(gBuffer_program);

	// Get delta time
	cur_time = (float)glutGet(GLUT_ELAPSED_TIME);
	float delta = cur_time - old_time;
	old_time = cur_time;

	camera->camera_rotate(vec3(0.f, 1.f, 0.f), delta / 25000);

	// Draw camera
	GLuint loc_proj = glGetUniformLocation(gBuffer_program, "projection_matrix");
	GLuint loc_view = glGetUniformLocation(gBuffer_program, "view_matrix");
	glUniformMatrix4fv(loc_proj, 1, GL_FALSE, &(camera->get_projection_matrix())[0][0]);
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, &(camera->get_view_matrix())[0][0]);
	
	// Draw scene meshes
	GLuint loc_model_matrix = glGetUniformLocation(gBuffer_program, "model_matrix");
	GLuint loc_normal_matrix = glGetUniformLocation(gBuffer_program, "normal_matrix");
	for (int i = 0; i < sphere_count; i++)
	{
		glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &(spheres[i]->get_model_matrix())[0][0]);
		glUniformMatrix4fv(loc_normal_matrix, 1, GL_FALSE, &(spheres[i]->get_normal_matrix())[0][0]);
		glBindVertexArray(spheres[i]->get_VAO());
		// Bind the texture
		GLuint loc_diff_texture = glGetUniformLocation(deferred_shading_program, "diffuse");
		glUniform1i(loc_diff_texture, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, spheres[i]->get_texture_id());
		glDrawArrays(GL_TRIANGLES, 0, spheres[i]->get_triangle_count() * 3);
		glBindVertexArray(0);
	}
	// Draw planes
	for (int i = 0; i < planes.size(); i++)
	{
		glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &(planes[i]->get_model_matrix())[0][0]);
		glUniformMatrix4fv(loc_normal_matrix, 1, GL_FALSE, &(planes[i]->get_normal_matrix())[0][0]);
		glBindVertexArray(planes[i]->get_VAO());
		// Bind the texture
		GLuint loc_diff_texture = glGetUniformLocation(deferred_shading_program, "diffuse");
		glUniform1i(loc_diff_texture, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, planes[i]->get_texture_id());
		glDrawArrays(GL_TRIANGLES, 0, planes[i]->get_triangle_count() * 3);
		glBindVertexArray(0);
	}

	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, Globals::WIDTH, Globals::HEIGHT, 0, 0, Globals::WIDTH, Globals::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//
	//// 2. Lighting Pass: Calculate lighting pixel by pixel using gBuffer
	//

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
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	// Bind light uniforms
	for (int i = 0; i < light_count; i++)
	{
		std::string light_array_str = "lights[" + std::to_string(i) + "].position";
		GLuint loc_lights_pos = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, &(lights[i].position)[0]);

		light_array_str = "lights[" + std::to_string(i) + "].color";
		loc_lights_pos = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, &(lights[i].color)[0]);

		light_array_str = "lights[" + std::to_string(i) + "].radius";
		GLuint loc_light_r = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_r, (GLfloat)lights[i].radius);

		light_array_str = "lights[" + std::to_string(i) + "].linear";
		GLuint loc_light_l = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_l, (GLfloat)lights[i].linear);

		light_array_str = "lights[" + std::to_string(i) + "].quadratic";
		GLuint loc_light_q = glGetUniformLocation(deferred_shading_program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_q, (GLfloat)lights[i].quadratic);
	}

	// Draw quad using gBuffer color data
	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	
	// 3. Pass: Draw light meshes (The meshes that are not lit but in the same scene with other meshes)
	
	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, Globals::WIDTH, Globals::HEIGHT, 0, 0, Globals::WIDTH, Globals::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Start forward rendering program
	glUseProgram(deferred_unlit_meshes_program);

	// Draw camera
	loc_proj = glGetUniformLocation(deferred_unlit_meshes_program, "projection_matrix");
	loc_view = glGetUniformLocation(deferred_unlit_meshes_program, "view_matrix");
	loc_model_matrix = glGetUniformLocation(deferred_unlit_meshes_program, "model_matrix");
	GLuint loc_color = glGetUniformLocation(deferred_unlit_meshes_program, "light_color");
	glUniformMatrix4fv(loc_proj, 1, GL_FALSE, &(camera->get_projection_matrix())[0][0]);
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, &(camera->get_view_matrix())[0][0]);

	// Draw light meshes
	for (int i = 0; i < light_count; i++)
	{
		glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &(light_meshes[i]->get_model_matrix())[0][0]);
		glUniform3fv(loc_color, 1, &(lights[i].color)[0]);
		glBindVertexArray(light_meshes[i]->get_VAO());
		glDrawArrays(GL_TRIANGLES, 0, light_meshes[i]->get_triangle_count() * 3);
		glBindVertexArray(0);
	}

	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));
	
	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
