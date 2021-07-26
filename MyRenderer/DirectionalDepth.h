#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>

typedef glm::mat4 mat4;

class DirectionalDepth
{
public:

	DirectionalDepth();
	~DirectionalDepth();

	void start_program();
	GLuint get_shader_program();
	void set_space_matrix(mat4 mat);
	void set_model_matrix(mat4 mat);
	void render(Model* model, GLuint shader_program);

private:

	GLuint program;
	GLuint loc_space_matrix;
	GLuint loc_model_matrix;

	void init_shaders();
	void get_uniform_locations();

};
