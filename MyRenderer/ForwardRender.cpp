#include "ForwardRender.h"

#include <init_shaders.h>

// Constructor
ForwardRender::ForwardRender()
{
	// Compiles shaders and generate program
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();

	init_depth_fbo();
}

// Destructor
ForwardRender::~ForwardRender() 
{
	glDeleteFramebuffers(1, &depth_fbo);
	glDeleteTextures(1, &depth_texture);
}

void ForwardRender::start_program(MainFramebuffer* fb)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb->get_FBO());
	glUseProgram(program);
}

// Change resolution
void ForwardRender::change_viewport_resolution(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
}

GLuint ForwardRender::get_shader_program()
{
	return program;
}

// Sets projection matrix
void ForwardRender::set_projection_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets view matrix
void ForwardRender::set_view_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets color
void ForwardRender::set_color(vec3 vec)
{
	glUniform3fv(loc_color, 1, &vec[0]);
}

// Render the model
void ForwardRender::render_model(Camera* camera, Model* model)
{
	glViewport(0, 0, width, height);

	// Set camera attributes
	set_projection_matrix(camera->get_projection_matrix());
	set_view_matrix(camera->get_view_matrix());
	
	// Draw call
	model->draw(program, camera->get_position());

	glBindVertexArray(0);
}

void ForwardRender::render(Scene* scene, MainFramebuffer* main_fb)
{
	start_program(main_fb);

	// Draw scene
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		if (scene->point_lights[i]->model != nullptr)
		{
			set_color(scene->point_lights[i]->color);
			render_model(scene->camera, scene->point_lights[i]->model);
		}

		if (scene->point_lights[i]->is_debug_active())
		{
			set_color(scene->point_lights[i]->color);
			// Light radius renders wih wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_model(scene->camera, scene->point_lights[i]->debug_model);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	// Copy depth buffer
	copy_depth_buffer(main_fb);
}

// Return depth buffer framebuffer
GLuint ForwardRender::get_depth_fbo()
{
	return depth_fbo;
}

GLuint ForwardRender::get_depth_texture()
{
	return depth_texture;
}

// Compiles the shaders and generates program
void ForwardRender::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_unlit_meshes_vs.glsl");
	GLint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_unlit_meshes_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Gets uniform locaitons from program
void ForwardRender::get_uniform_locations()
{
	loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
	loc_view_matrix = glGetUniformLocation(program, "view_matrix");
	loc_color = glGetUniformLocation(program, "color");
}

// Create a framebuffer only has depth attachment
void ForwardRender::init_depth_fbo()
{
	glGenFramebuffers(1, &depth_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

	// Depth as texture
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT,
		GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR: Framebuffer not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Copy depth buffer and store
void ForwardRender::copy_depth_buffer(MainFramebuffer* main_fb)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, main_fb->get_FBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_fbo);
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
