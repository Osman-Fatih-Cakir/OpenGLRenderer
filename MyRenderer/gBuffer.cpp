
#include <gBuffer.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <init_shaders.h>
#include <Window.h>

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

	// Initialize g-buffer framebuffer
	create_framebuffer();

	// Get uniform locations
	get_uniform_locations();
}

// Destructor
gBuffer::~gBuffer()
{
	// Deallocate texture buffers
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedoSpec);
	glDeleteTextures(1, &gPbr_materials);
	glDeleteTextures(1, &gEmissive);

	// Deallocate render buffer
	glDeleteRenderbuffers(1, &rbo_depth);

	// Deallocate framebuffer
	glDeleteFramebuffers(1, &gBuffer_fbo);
}

// Attachs the g-buffer depth buffer to given framebuffer
void gBuffer::attach_depthbuffer_to_framebuffer(GLuint framebuffer)
{
	// Get current fbo for set it back
	GLint current_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

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
// Returns g-buffer color attachment stores roughness and metallic
GLuint gBuffer::get_gPbr_materials()
{
	return gPbr_materials;
}
// Returns emissive color attachment
GLuint gBuffer::get_emissive()
{
	return gEmissive;
}

// Returns framebuffer object of g-buffer
GLuint gBuffer::get_fbo()
{
	return gBuffer_fbo;
}

GLuint gBuffer::get_shader_program()
{
	return program;
}

// Starts the shader program
void gBuffer::start_program()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
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

unsigned int gBuffer::get_width()
{
	return gBuffer_width;
}

unsigned int gBuffer::get_height()
{
	return gBuffer_height;
}

// Renders the scene
void gBuffer::render(Camera* camera, Model* model)
{
	// Cull backfaces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Camera matrix
	set_projection_matrix(camera->get_projection_matrix());
	set_view_matrix(camera->get_view_matrix());

	// Model matrix and normal matrix
	//set_model_matrix(model->get_model_matrix());
	//set_normal_matrix(model->get_normal_matrix());
	
	// Draw call
	model->draw(program);

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
	glBindTexture(GL_TEXTURE_2D, 0);

	// Normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Albedo color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Rougness, metallic, ao component
	glGenTextures(1, &gPbr_materials);
	glBindTexture(GL_TEXTURE_2D, gPbr_materials);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gPbr_materials, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Emissive color buffer
	glGenTextures(1, &gEmissive);
	glBindTexture(GL_TEXTURE_2D, gEmissive);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gBuffer_width, gBuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gEmissive, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);

	// Create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gBuffer_width, gBuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Prevent further modification
}

// Gets uniform locations from the program and store
void gBuffer::get_uniform_locations()
{
	loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
	loc_view_matrix = glGetUniformLocation(program, "view_matrix");
	loc_model_matrix = glGetUniformLocation(program, "model_matrix");
	loc_normal_matrix = glGetUniformLocation(program, "normal_matrix");
}
