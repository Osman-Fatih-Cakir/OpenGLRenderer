
#include <PointDepth.h>

#include <gtc/matrix_transform.hpp>
#include <init_shaders.h>

PointDepth::PointDepth()
{
	// Compiles shaders and generate program
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();
}

PointDepth::~PointDepth() {}

void PointDepth::start_program()
{
	glUseProgram(program);
}

void PointDepth::set_space_matrices(mat4 mats[6])
{
	glUniformMatrix4fv(loc_space_matrices, 6, GL_FALSE, &mats[0][0][0]);
}

void PointDepth::set_model_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &mat[0][0]);
}

void PointDepth::set_far(float num)
{
	glUniform1f(loc_far, num);
}

void PointDepth::set_position(vec3 pos)
{
	glUniform3fv(loc_position, 1,&pos[0]);
}

void PointDepth::render(GLuint VAO, unsigned int vertex_count)
{
	glBindVertexArray(VAO);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
}

// Compiles shaders and generates program
void PointDepth::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/point_depth_vs.glsl");
	GLuint geometry_shader = initshaders(GL_GEOMETRY_SHADER, "shaders/point_depth_gs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/point_depth_fs.glsl");
	program = initprogram(vertex_shader, geometry_shader, fragment_shader);
}

// Get uniform locations from program
void PointDepth::get_uniform_locations()
{
	loc_space_matrices = glGetUniformLocation(program, "light_space_matrix");
	loc_position = glGetUniformLocation(program, "light_position");
	loc_far = glGetUniformLocation(program, "far");
	loc_model_matrix = glGetUniformLocation(program, "model");
}
