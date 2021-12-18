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
	// TODO make model private (getter and setter)
	GLfloat* get_space_matrices_pointer();
	GLfloat* get_position_pointer();
	GLfloat* get_color_pointer();
	mat4 get_model_matrix();
	void create_shadow();
	void translate(float x, float y, float z, float delta);
	void scale(float x, float y, float z, float delta);

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

	int depth_map_width = 2048;
	int depth_map_height = 2048;

private:
	void create_depth_map_framebuffer();
	void init_space_matrices();

	bool shadow_calculated = false;
	mat4 model_matrix = mat4(1.f);
};
