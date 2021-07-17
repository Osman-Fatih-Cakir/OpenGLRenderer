#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class DirectionalLight
{

public:
	// Constructor
	DirectionalLight(vec3 dir, vec3 col);

	void set_view(vec3 eye, vec3 center, vec3 up);
	// The projection matrix must be orthogonal matrix
	void set_projection(float left, float right, float bottom, float top, float zNear, float zFar);
	// This function should be called after setting view and projection matrices
	void calculate_space_matrix();
	void calculate_space_matrix(mat4 proj_mat, mat4 view_mat);
	void create_depth_map_framebuffer();
	GLfloat* get_space_matrix_pointer();
	GLfloat* get_direction_pointer();
	GLfloat* get_color_pointer();

	vec3 direction;
	vec3 color;

	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 space_matrix;

	GLuint depth_map;
	GLuint depth_map_fbo;

	int depth_map_width = 1024;
	int depth_map_height = 1024;

	float intensity;
};
