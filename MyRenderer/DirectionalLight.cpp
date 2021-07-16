
#include <gtc/matrix_transform.hpp>
#include <DirectionalLight.h>


DirectionalLight::DirectionalLight(vec3 dir, vec3 col)
{
	direction = dir;
	color = col;

	projection_matrix = mat4(0.0f);
	view_matrix = mat4(0.0f);
	space_matrix = mat4(0.0f);

	intensity = 1.0f;
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
