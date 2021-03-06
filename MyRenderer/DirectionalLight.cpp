
#include <gtc/matrix_transform.hpp>
#include <DirectionalLight.h>

// Constructor
DirectionalLight::DirectionalLight(vec3 dir, vec3 col, bool shadow)
{
	direction = dir;
	color = col;

	projection_matrix = mat4(0.0f);
	view_matrix = mat4(0.0f);
	space_matrix = mat4(0.0f);

	intensity = 0.2f;

	if (shadow)
	{
		//create_shadow(-100.f, 100.f, -100.f, 100.f, 0.01f, 10000.f,
		//	vec3(100.f, 100.f, 100.f), vec3(0.f, 0.f, 0.f), vec3(-1.f, 1.f, -1.f)
		//);
		//create_shadow(-20.f, 20.f, -20.f, 20.f, 0.01f, 1000.f,
		//	vec3(20, 20.f, 20.f), vec3(0.f, 0.f, 0.f), vec3(-1.f, 1.f, -1.f)
		create_shadow(-50.f, 50.f, -50.f, 50.f, 0.01f, 1000.f,
			vec3(2, 20.f, 2.f), vec3(0.f, 0.f, 0.f), vec3(-1.f, 1.f, -1.f)
		);
	}
}

// Destructor
DirectionalLight::~DirectionalLight()
{
	// Deallocate framebuffer
	glDeleteFramebuffers(1, &depth_map_fbo);

	// Deallocate depth map texture
	glDeleteTextures(1, &depth_map);
}

void DirectionalLight::set_view(vec3 eye, vec3 center, vec3 up)
{
	view_matrix = glm::lookAt(eye, center, up);
}

// The projection must be orthogonal
void DirectionalLight::set_projection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	projection_matrix = glm::ortho(left, right, bottom, top, zNear, zFar);
}

void DirectionalLight::create_shadow(float left, float right, float bottom, float top,
	float _near, float _far, vec3 eye, vec3 center, vec3 up)
{
	// Initialize shadow map buffers
	create_depth_map_framebuffer();

	// Calcualte shadow projection and space matrices
	set_projection(left, right, bottom, top, _near, _far);
	set_view(eye, center, up);
	calculate_space_matrix();

	cast_shadow = true;
}

// Returns true if light casts shadow
bool DirectionalLight::does_cast_shadow()
{
	return  cast_shadow;
}

// Calculating space matrices from view and projection matrices
// This function should be called after setting view and projection matrices
void DirectionalLight::calculate_space_matrix()
{
	space_matrix = projection_matrix * view_matrix;
}
// Assigns projection and view matrices and calculates space matrices from them
void DirectionalLight::calculate_space_matrix(mat4 proj_mat, mat4 view_mat)
{
	projection_matrix = proj_mat;
	view_matrix = view_mat;
	space_matrix = proj_mat * view_mat;
}

// Creates framebuffer for depth map
void DirectionalLight::create_depth_map_framebuffer()
{
	// Get current fbo for set it back
	GLint last_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &last_fbo);

	glGenFramebuffers(1, &depth_map_fbo);

	// Create 2D depth texture for store depths
	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	// Set both to "none" because there is no need for color attachment
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, last_fbo); // Set the last framebuffer back
}

// Returns space matrix pointer
GLfloat* DirectionalLight::get_space_matrix_pointer()
{
	return &space_matrix[0][0];
}

// Returns direction vector pointer
GLfloat* DirectionalLight::get_direction_pointer()
{
	return &direction[0];
}

// Returns color vector pointer
GLfloat* DirectionalLight::get_color_pointer()
{
	return &color[0];
}

vec3 DirectionalLight::get_color()
{
	return color;
}

vec3 DirectionalLight::get_direction()
{
	return direction;
}

void DirectionalLight::set_direction(vec3 mat)
{
	//TODOdirection = mat;

}

GLuint DirectionalLight::get_depth_map()
{
	return depth_map;
}

mat4 DirectionalLight::get_space_matrix()
{
	return space_matrix;
}
