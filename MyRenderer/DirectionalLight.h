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
	~DirectionalLight();

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

	vec3 direction = vec3(1.0f);
	vec3 color = vec3(1.0f);

	mat4 projection_matrix = mat4(1.0f);
	mat4 view_matrix = mat4(1.0f);
	mat4 space_matrix = mat4(1.0f);

	GLuint depth_map;
	GLuint depth_map_fbo;

	int depth_map_width = 2048;
	int depth_map_height = 2048;

	float intensity;
};
