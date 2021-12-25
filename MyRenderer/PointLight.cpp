
#include <PointLight.h>
#include <gtc/matrix_transform.hpp>
#include <iostream>

// Constructor
PointLight::PointLight(vec3 pos, vec3 col, bool shadow)
{
	translate(pos.x, pos.y, pos.z, 1.0f);
	color = col;

	set_intensity(1.0f);
	shadow_projection_far = 100.0f;
	depth_cubemap = -1;
	depth_cubemap_fbo = -1;
	for (int i = 0; i < 6; i++)
		space_matrices[i] = mat4(0.0f);

	// Attenuation constants
	//linear = 1.5f;
	//quadratic = 3.8f;
	linear = 0.7f;
	quadratic = 1.8f;

	calculate_radius();

	if (shadow)
		create_shadow();
}

// Destructor
PointLight::~PointLight()
{
	if (model)
		delete model;
	if (debug_model)
		delete debug_model;

	// Deallocate cubemap texture
	glDeleteTextures(1, &depth_cubemap);

	// Deallocate framebuffer
	glDeleteFramebuffers(1, &depth_cubemap_fbo);
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

// Set intensity
void PointLight::set_intensity(float val)
{
	intensity = val;
}

// Return model matrix
mat4 PointLight::get_model_matrix()
{
	return model_matrix;
}

// Return the debug model matrix for light radius
mat4 PointLight::get_debug_model_matrix()
{
	return glm::scale(model_matrix, vec3(radius, radius, radius));
}

// Returns true if the debug mode is active
bool PointLight::is_debug_active()
{
	return debug_mode;
}

// Debug light
void PointLight::debug(Model* model)
{
	debug_model = model;
	debug_mode = true;
}

// Returns true if the lights casts shadow
bool PointLight::does_cast_shadow()
{
	return shadow_calculated;
}

// Translate
void PointLight::translate(float x, float y, float z, float delta)
{
	position.x += x * delta;
	position.y += y * delta;
	position.z += z * delta;

	// Calculate model matrix
	model_matrix = glm::translate(model_matrix, vec3(x * delta, y * delta, z * delta));

	if (shadow_calculated)
	{
		init_space_matrices();
	}
}

// Scale
void PointLight::scale(float x, float y, float z, float delta)
{
	model_matrix = glm::scale(model_matrix, vec3(x*delta, y*delta, z*delta));
}

// Create point light shadow maps
void PointLight::create_shadow()
{
	// Create depth map buffers
	create_depth_map_framebuffer();
	// Calculate space matrices
	init_space_matrices();

	shadow_calculated = true;
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

// Initialize space matrices for point light shadows
void PointLight::init_space_matrices()
{
	// Projection matrix
	shadow_projection_far = 500.f;
	mat4 pointlight_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, shadow_projection_far);
	// View matrices (for each cube plane)
	space_matrices[0] = pointlight_projection * glm::lookAt(position, position + vec3(1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
	space_matrices[1] = pointlight_projection * glm::lookAt(position, position + vec3(-1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f));
	space_matrices[2] = pointlight_projection * glm::lookAt(position, position + vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));
	space_matrices[3] = pointlight_projection * glm::lookAt(position, position + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f));
	space_matrices[4] = pointlight_projection * glm::lookAt(position, position + vec3(0.f, 0.f, 1.f), vec3(0.f, -1.f, 0.f));
	space_matrices[5] = pointlight_projection * glm::lookAt(position, position + vec3(0.f, 0.f, -1.f), vec3(0.f, -1.f, 0.f));
}

// Calculates radius
void PointLight::calculate_radius()
{	
	// Calculate light radius
	float constant = 1.0f;

	// Get light's luminance using Rec 709 luminance formula
	float light_luminance = glm::dot(color, vec3(0.2126, 0.7152, 0.0722));

	// Minimum luminance threshold - tweak to taste
	float min_luminance = 0.01;

	// Solve attenuation equation to get radius where it falls to min_luminance
	radius = solve_quadratic(constant - light_luminance/ min_luminance, linear, quadratic);
	radius *= intensity;

	float inner_radius = radius * 0.2f;
	half_radius = radius / 2.f;
	cutoff = 1.f / 2.f;
}

float PointLight::solve_quadratic(float kc, float kl, float kq)
{
	float lightMax = std::fmaxf(std::fmaxf(color.r, color.g), color.b);

	float _radius = (float)(-kc + std::sqrtf(kl * kl - 4 * kq
		* (kc - (256.0f / 5.0f) * lightMax))) / (2 * quadratic);

	return _radius;
}