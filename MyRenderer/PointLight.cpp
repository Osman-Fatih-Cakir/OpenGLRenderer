
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

// Destructor
PointLight::~PointLight()
{
	delete mesh;
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

void PointLight::create_depth_map_framebuffer()
{
	glGenFramebuffers(1, &depth_cubemap_fbo);
	glGenTextures(1, &depth_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);

	// Create 6 2D depth texture framebuffers for genertate a cubemap
	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depth_cubemap_fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
	// Set both to "none" because there is no need for color attachment
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
