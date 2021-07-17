#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class PointDepth
{
public:

	PointDepth();
	~PointDepth();

	void start_program();
	void set_space_matrices(mat4 mats[6]);
	void set_model_matrix(mat4 mat);
	void set_far(float num);
	void set_position(vec3 pos);
	void render(GLuint VAO, unsigned int vertex_count);
	
private:

	GLuint program;
	GLuint loc_space_matrices;
	GLuint loc_model_matrix;
	GLuint loc_far;
	GLuint loc_position;

	void init_shaders();
	void get_uniform_locations();

};