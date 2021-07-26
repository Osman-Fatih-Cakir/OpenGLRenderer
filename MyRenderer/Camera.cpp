#include "Camera.h"

#include <gtc/matrix_transform.hpp>


Camera::Camera(vec3 _eye, vec3 _up, vec3 _center, Globals::Projection_Type proj_type)
{
	eye = _eye;
	up = _up;
	center = _center;
	projection_type = proj_type;
	set_camera_projection(proj_type); // Set projection matrix
	camera_lookAt(eye, up, center); // Set view matrix
}

Camera::~Camera() {};

void Camera::set_camera_projection(Globals::Projection_Type proj_type)
{
	if (proj_type == Globals::PERSPECTIVE) // Set perspective projection
	{
		set_perspective(75.f, (GLfloat)Globals::WIDTH / Globals::HEIGHT, 0.01f, 100.f);
	}
	else if (proj_type == Globals::ORTOGRAPHIC) // Set ortographic projection
	{
		set_ortho(-2.f, 2.f, -2.f, 2.f); // TODO check these numbers make sense
	}
}

void Camera::set_ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top)
{
	projection_matrix = glm::ortho(left, right, bottom, top);
}
// fovy takes the parameter as degree
void Camera::set_perspective(GLfloat fovy, GLfloat aspect, GLfloat _near, GLfloat _far)
{
	projection_matrix = glm::perspective(glm::radians(fovy), aspect, _near, _far);
}
// Look at function
void Camera::camera_lookAt(vec3 eye, vec3 up, vec3 center)
{
	view_matrix = glm::lookAt(eye, center, up);
}

// Camera translate
void Camera::camera_translate(vec3 tra)
{
	view_matrix = glm::translate(view_matrix, tra);
}

// Camera rotate
void Camera::camera_rotate(vec3 rot, float angle)
{
	view_matrix = glm::rotate(view_matrix, glm::radians(angle), rot);
}

//
//// Getters and setters
//
Globals::Projection_Type Camera::get_projection_type()
{
	return projection_type;
}
mat4 Camera::get_projection_matrix()
{
	return projection_matrix;
}

void Camera::set_projection_matrix(mat4 mat)
{
	projection_matrix = mat;
}

mat4 Camera::get_view_matrix()
{
	return view_matrix;
}

void Camera::set_view_matrix(mat4 mat)
{
	view_matrix = mat;
}

vec3 Camera::get_eye()
{
	return eye;
}

void Camera::set_eye(vec3 vec)
{
	eye = vec;
}

vec3 Camera::get_up()
{
	return up;
}

void Camera::set_up(vec3 vec)
{
	up = vec;
}

vec3 Camera::get_center()
{
	return center;
}

void Camera::set_center(vec3 vec)
{
	center = vec;
}
