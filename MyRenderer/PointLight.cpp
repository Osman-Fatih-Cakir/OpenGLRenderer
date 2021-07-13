
#include <PointLight.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

// Constructor
PointLight::PointLight(vec3 pos, vec3 col)
{
	position = pos;
	color = col;

	// Calculate light radius
	float constant = 1.0f;
	linear = 0.02f;
	quadratic = 0.07f;
	float lightMax = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
	float _radius = (float)(-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * lightMax)))
			/ (2 * quadratic);
	radius = _radius;

	intensity = 1.0f;
	shadow_projection_far = 100.0f;

	depth_cubemap = -1;
	depth_cubemap_fbo = -1;
	for (int i = 0; i < 6; i++)
		space_matrices[i] = mat4(0.0f);

}

// Returns a pointer to the space matrices array
GLfloat* PointLight::get_space_matrices_pointer()
{
	return &space_matrices[0][0][0];
}

// Returns a pointer to the position vector
GLfloat* PointLight::get_position_pointer()
{
	return &position[0];
}

// Returns a pointer to the color vector
GLfloat* PointLight::get_color_pointer()
{
	return &color[0];
}
