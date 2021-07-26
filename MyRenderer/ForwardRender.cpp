#include "ForwardRender.h"

#include <init_shaders.h>

ForwardRender::ForwardRender()
{
	// Compiles shaders and generate program
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();
}

ForwardRender::~ForwardRender() {}

void ForwardRender::start_program()
{
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

// Sets model matrix
void ForwardRender::set_model_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets color
void ForwardRender::set_color(vec3 vec)
{
	glUniform3fv(loc_color, 1, &vec[0]);
}

// Render the scene
void ForwardRender::render(Model* model, GLuint shader_program)
{
	glViewport(0, 0, width, height);

	model->draw(shader_program);

	glBindVertexArray(0);
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
	loc_model_matrix = glGetUniformLocation(program, "model_matrix");
	loc_color = glGetUniformLocation(program, "color");
}
