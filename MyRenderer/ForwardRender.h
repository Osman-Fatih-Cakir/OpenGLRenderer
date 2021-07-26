#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class ForwardRender
{
public:

	ForwardRender();
	~ForwardRender();

	void start_program();
	void change_viewport_resolution(unsigned int w, unsigned int h);
	GLuint get_shader_program();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_model_matrix(mat4 mat);
	void set_color(vec3 vec);
	void render(Model* model, GLuint shader_program);

private:

	GLuint program;
	GLuint width = 1024;
	GLuint height = 1024;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_color;

	void init_shaders();
	void get_uniform_locations();
};
