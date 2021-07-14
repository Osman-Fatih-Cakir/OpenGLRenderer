#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

#include <Mesh.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

class PointLight
{
public:
	// Constructor
	PointLight(vec3 pos, vec3 col);
	// Destructor
	~PointLight();

	// Returns a pointer to the space matrices array
	GLfloat* get_space_matrices_pointer();
	// Returns a pointer to the position vector
	GLfloat* get_position_pointer();
	// Returns a pointer to the color vector
	GLfloat* get_color_pointer();

	Mesh* mesh = nullptr;

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
};
