
// TODO implement deferred shading
// TODO delete default vs/fs.glsl shaders
// TODO After that, check what I need


// TODO create uniform classes
// TODO create shader classes
// TODO light, renderer


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


const int light_count = 4;
struct Light
{
	vec3 position;
	vec3 color;
}lights[light_count];

// Camera
Camera* camera;

// Meshes
Mesh* monkey;
GLuint quad_VAO;

// Shader programs
GLuint deferred_shading_program;
GLuint gBuffer_program;

// Framebuffers
GLuint gBuffer;

// Textures
GLuint gPosition, gNormal, gAlbedoSpec;

void Init_Glut_and_Glew(int argc, char* argv[]);
void init();
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
	// TODO After updating glew version, delete this
	GLuint errr = glGetError(); if (errr) fprintf(stderr, "%s\n", gluErrorString(errr));
	if (GLEW_OK != err)
	{
		std::cout << "Unable to initalize Glew ! " << std::endl;
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	

	// Function bindings
	glutDisplayFunc(render);
}

// Initialize the parameters
void init()
{
	Globals::Log("*****************Start************************");
	// Load monkey mesh
	monkey = new Mesh("monkey/monkey.obj");

	// Initialize shaders
	init_shaders();

	// Set camera parameters
	init_camera(
		vec3(0.f, 0.f, -5.f), // Eye
		vec3(0.f, 1.f, 0.f), // Up
		vec3(0.f, 0.f, 0.f) // Center
	);

	// Initialize gBuffer textures and render buffer for depth
	init_gBuffer();

	// Initialize quad that will be rendered with texture (The scene texture)
	init_quad();

	// Initialize lights
	init_lights();
}

// Initialize and compiling shaders
void init_shaders()
{
	// gBuffer shaders
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/gbuffer_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/gbuffer_fs.glsl");
	gBuffer_program = initprogram(vertex_shader, fragment_shader);

	// deferred lightinh shaders
	vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_shading_vs.glsl");
	fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_shading_fs.glsl");
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
	/*
	// Create and attach depth buffer (renderbuffer)
	GLuint rbo_depth;
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Globals::WIDTH, Globals::HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Globals::Log("Framebuffer not complete!");
	*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Prevent further modification
}

// Initialize light paramters
void init_lights()
{
	// Light positions
	vec3 poses[light_count] = {
		vec3(2.f, 0.f, 0.f), vec3(-2.f, 0.f, 0.f), vec3(0.f, 2.f, 0.f), vec3(0.f, -2.f, 0.f)
	};
	vec3 colors[light_count] = {
		vec3(1.f, 1.f, 1.f), vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f)
	};
	for (int i = 0; i < light_count; i++)
	{
		lights[i].position = poses[i];
		lights[i].color = colors[i];
	}
}

// Renders the scene
void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.9f, 0.9f, 0.9f, 1.f);

	//
	//// 1. GBuffer Pass: Generate geometry/color data into gBuffers
	//
	
	// Bind framebuffer to gBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// gBuffer program
	glUseProgram(gBuffer_program);

	// Draw camera
	GLuint loc_proj = glGetUniformLocation(gBuffer_program, "projection");
	GLuint loc_view = glGetUniformLocation(gBuffer_program, "view");
	glUniformMatrix4fv(loc_proj, 1, GL_FALSE, &(camera->get_projection_matrix())[0][0]);
	glUniformMatrix4fv(loc_view, 1, GL_FALSE, &(camera->get_view_matrix())[0][0]);
	
	// Draw_model
	GLuint loc_model = glGetUniformLocation(gBuffer_program, "model");
	glUniformMatrix4fv(loc_model, 1, GL_FALSE, &(mat4(1.f))[0][0]);
	glBindVertexArray(monkey->get_VAO());
	glDrawArrays(GL_TRIANGLES, 0, monkey->get_triangle_count() * 3);
	glBindVertexArray(0);

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
	}

	// Draw quad with gBuffer data (as texture)
	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	
	// Error check
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));
	
	glutSwapBuffers();

	glutPostRedisplay(); // Render loop
}
