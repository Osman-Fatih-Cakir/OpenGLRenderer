#include "Camera.h"

#include <gtc/matrix_transform.hpp>
#include <Window.h>


Camera::Camera(vec3 _eye, vec3 _up, vec3 _center, Projection_Type proj_type)
{
	position = _eye;
	eye = _eye;
	up = _up;
	center = _center;
	projection_type = proj_type;
	set_camera_projection(proj_type); // Set projection matrix
	lookAt(eye, up, center); // Set view matrix
}

Camera::~Camera() {};

void Camera::set_camera_projection(Projection_Type proj_type)
{
	if (proj_type == PERSPECTIVE) // Set perspective projection
	{
		set_perspective(60.f, (GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 1000.f);
	}
	else if (proj_type == ORTOGRAPHIC) // Set ortographic projection
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
void Camera::lookAt(vec3 eye, vec3 up, vec3 center)
{
	view_matrix = glm::lookAt(eye, center, up);
}

// Camera translate
void Camera::translate(vec3 tra, float delta)
{
	view_matrix = glm::translate(view_matrix, tra * delta);
	mat4 inv = glm::inverse(view_matrix);
	position = vec3(inv[3][0], inv[3][1], inv[3][2]);
}

void Camera::translate(float x, float y, float z, float delta)
{
	view_matrix = glm::translate(view_matrix, vec3(x * delta, y * delta, z * delta));
	mat4 inv = glm::inverse(view_matrix);
	position = vec3(inv[3][0], inv[3][1], inv[3][2]);
}

// Camera rotate
void Camera::rotate(vec3 rot, float angle, float delta)
{
	view_matrix = glm::rotate(view_matrix, glm::radians(angle), rot * delta);
	mat4 inv = glm::inverse(view_matrix);
	position = vec3(inv[3][0], inv[3][1], inv[3][2]);
}

//
//// Getters and setters
//
Projection_Type Camera::get_projection_type()
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

mat4 Camera::get_prev_view_matrix()
{
	return prev_view_matrix;
}

void Camera::set_prev_view_matrix()
{
	prev_view_matrix = view_matrix;
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

vec3 Camera::get_position()
{
	return position;
}

void Camera::set_position(vec3 vec)
{
	position = vec;
	// TODO fix here
	//mat4 _view_matrix = view_matrix;
	//view_matrix = glm::translate(mat4(1.f), vec);
	//for (int i = 0; i < 3; i++)
	//	for (int j = 0; j < 3; j++)
	//		view_matrix[i][j] = _view_matrix[i][j];
}

void Camera::local_rotate(vec3 rot, float angle, float delta)
{
	view_matrix = glm::translate(view_matrix, vec3(position.x,position.y, position.z));
	view_matrix = glm::rotate(view_matrix, delta*angle, rot);
	view_matrix = glm::translate(view_matrix, vec3(-1.f * position.x,
		-1.f * position.y, -1.f * position.z));
}
