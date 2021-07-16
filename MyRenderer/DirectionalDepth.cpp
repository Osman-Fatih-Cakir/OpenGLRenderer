
#include <DirectionalDepth.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <Globals.h>
#include <init_shaders.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

// Constructor
DirectionalDepth::DirectionalDepth()
{
	// Initialize and compile shaders
	init_shaders();

	// Create framebuffer for depth map
	create_framebuffer();

	// Get uniform locations from program
	get_uniform_locations();
}

// Destructor
DirectionalDepth::~DirectionalDepth() {}

// Starts the shader program
void DirectionalDepth::start_program()
{
	glUseProgram(program);
	glViewport(0, 0, depth_map_width, depth_map_height); // Use shadow resolutions
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
}

// Change the resolution of the depth map
void DirectionalDepth::change_depth_map_resolution(int w, int h)
{
	depth_map_width = w;
	depth_map_height = h;
	create_framebuffer();
}

// Sets space matrix
void DirectionalDepth::set_space_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_space_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets model matrix
void DirectionalDepth::set_model_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Returns depth map 
GLuint DirectionalDepth::get_depth_map()
{
	return depth_map;
}

// Renders the scene
void DirectionalDepth::render(GLuint VAO, unsigned int vertex_count)
{
	glBindVertexArray(VAO);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
}

// Compiles shaders and generate program
void DirectionalDepth::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/depth_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/depth_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Creates framebuffer
void DirectionalDepth::create_framebuffer()
{
	// Get current fbo for set it back
	GLint last_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fbo);

	glGenFramebuffers(1, &depth_map_fbo);

	// Create 2D depth texture for store depths
	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	// Set both to "none" because there is no need for color attachment
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, last_fbo); // Set the last framebuffer back
}

// Gets uniforms locations from the program
void DirectionalDepth::get_uniform_locations()
{
	loc_space_matrix = glGetUniformLocation(program, "space_matrix");
	loc_model_matrix = glGetUniformLocation(program, "model_matrix");
}