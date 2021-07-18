
#include <gBuffer.h>

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
gBuffer::gBuffer()
{
	// Initialize and compile shaders, create shader program
	init_shaders();

	// Initialize g-buffer framebuffer and color attachments
	create_framebuffer();

	// Get uniform locations
	get_uniform_locations();
}

// Destructor
gBuffer::~gBuffer()
{

}

// Attachs the g-buffer depth buffer to given framebuffer
void gBuffer::attach_depthbuffer_to_framebuffer(GLuint framebuffer)
{
	// Get current fbo for set it back
	GLint current_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	glBlitFramebuffer(0, 0, Globals::WIDTH, Globals::HEIGHT, 0, 0, Globals::WIDTH, Globals::HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, current_fbo); // Set the last framebuffer back
}

// Sets the g-buffer resolution
void gBuffer::set_gBuffer_resolution(int w, int h)
{
	gBuffer_width = w;
	gBuffer_height = h;
	create_framebuffer(); // Create framebuffer with new resolution
}

// Returns g-buffer color attachment stores positionsü
GLuint gBuffer::get_gPosition()
{
	return gPosition;
}
// Returns g-buffer color attachment stores normals
GLuint gBuffer::get_gNormal()
{
	return gNormal;
}
// Returns g-buffer color attachment stores AlbedoSpecs
GLuint gBuffer::get_gAlbedoSpec()
{
	return gAlbedoSpec;
}

// Returns framebuffer object of g-buffer
GLuint gBuffer::get_fbo()
{
	return gBuffer_fbo;
}

// Starts the shader program
void gBuffer::start_program()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glUseProgram(program);
}

// Sets the projection matrix of camera
void gBuffer::set_projection_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets the view matrix of camera
void gBuffer::set_view_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets model matrix of model
void gBuffer::set_model_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets normal matrix of model
void gBuffer::set_normal_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_normal_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets diffuse texture
void gBuffer::set_diffuse_texture(GLuint id)
{
	GLuint loc_diff_texture = glGetUniformLocation(program, "diffuse");
	glUniform1i(loc_diff_texture, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, id);
}

unsigned int gBuffer::get_width()
{
	return gBuffer_width;
}

unsigned int gBuffer::get_height()
{
	return gBuffer_height;
}

// Renders the scene
void gBuffer::render(GLuint VAO, unsigned int vertex_count)
{
	glBindVertexArray(VAO);

	// Cull backfaces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glBindVertexArray(0);

	glDisable(GL_CULL_FACE); // Disable culling when rendering is done
}

// Compiles shaders and generates the shader program
void gBuffer::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/gbuffer_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/gbuffer_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Creates a framebuffer for gBuffer
void gBuffer::create_framebuffer()
{
	glGenFramebuffers(1, &gBuffer_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_fbo);

	// Position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// Normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// Color + Specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gBuffer_width, gBuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Globals::Log("Framebuffer not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Prevent further modification
}

// Gets uniform locations from the program and store
void gBuffer::get_uniform_locations()
{
	loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
	loc_view_matrix = glGetUniformLocation(program, "view_matrix");
	loc_model_matrix = glGetUniformLocation(program, "model_matrix");
	loc_normal_matrix = glGetUniformLocation(program, "normal_matrix");
	loc_diffuse = glGetUniformLocation(program, "diffuse");
}
