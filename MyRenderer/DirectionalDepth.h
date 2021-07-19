#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

typedef glm::mat4 mat4;

class DirectionalDepth
{
public:

	DirectionalDepth();
	~DirectionalDepth();

	void start_program();
	void set_space_matrix(mat4 mat);
	void set_model_matrix(mat4 mat);
	void render(GLuint VAO, unsigned int vertex_count);

private:

	GLuint program;
	GLuint loc_space_matrix;
	GLuint loc_model_matrix;

	void init_shaders();
	void get_uniform_locations();

};