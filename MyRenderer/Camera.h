#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Globals.h"


typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

class Camera
{
public:

	// Constructor
	Camera(vec3 _eye, vec3 _up, vec3 _center, Globals::Projection_Type proj_type);

	// Calculate camera attributes
	void set_camera_projection(Globals::Projection_Type proj_type);

	// Set camera projection matrix
	// The constructor calls them with default values
	// There function are public, the user can change the matrices
	void set_ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
	void set_perspective(GLfloat fovy, GLfloat aspect, GLfloat _near, GLfloat _far);

	// Look at function
	void camera_lookAt(vec3 eye, vec3 center, vec3 up);

	// Getters and setters
	Globals::Projection_Type get_projection_type;
	mat4 get_projection_matrix();
	void set_projection_matrix(mat4 mat);
	mat4 get_view_matrix();
	void set_view_matrix(mat4 mat);
	vec3 get_eye();
	void set_eye(vec3 vec);
	vec3 get_up();
	void set_up(vec3 vec);
	vec3 get_center();
	void set_center(vec3 vec);

private:
	// Projection type (Perspective or Ortographic)
	Globals::Projection_Type projection_type = Globals::PERSPECTIVE;
	// Projection matrix
	mat4 projection_matrix = mat4(1.f);
	// View matrix for camera
	mat4 view_matrix = mat4(1.f);
	// Eye coordinates
	vec3 eye = vec3(0.f);
	// Up vector
	vec3 up = vec3(0.f);
	// Center
	vec3 center = vec3(0.f);
};
