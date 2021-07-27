#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

#include <Model.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class PointLight
{
public:
	// Constructor
	PointLight(vec3 pos, vec3 col);
	// Destructor
	~PointLight();

	GLfloat* get_space_matrices_pointer();
	GLfloat* get_position_pointer();
	GLfloat* get_color_pointer();
	void create_depth_map_framebuffer();

	Model* model = nullptr;

	vec3 position;
	vec3 color;

	mat4 space_matrices[6];
	GLuint depth_cubemap;
	GLuint depth_cubemap_fbo;

	float shadow_projection_far;
	float radius;
	float linear;
	float quadratic;
	float intensity;

	int depth_map_width = 1024;
	int depth_map_height = 1024;

private:
	void init_space_matrices();
};
