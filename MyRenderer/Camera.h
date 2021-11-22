#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>


typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

enum Projection_Type
{
	PERSPECTIVE,
	ORTOGRAPHIC
};

class Camera
{
public:

	// Constructor and destructor
	Camera(vec3 _eye, vec3 _up, vec3 _center, Projection_Type proj_type);
	~Camera();

	// Calculate camera attributes
	void set_camera_projection(Projection_Type proj_type);

	// Set camera projection matrix
	// The constructor calls them with default values
	// There function are public, the user can change the matrices
	void set_ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top);
	void set_perspective(GLfloat fovy, GLfloat aspect, GLfloat _near, GLfloat _far);

	// Look at function
	void lookAt(vec3 eye, vec3 center, vec3 up);

	// Camera transformation
	void translate(vec3 tra, float delta);
	void translate(float x, float y, float z, float delta);
	void rotate(vec3 rot, float angle, float delta);

	// Getters and setters
	Projection_Type get_projection_type();
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
	vec3 get_position();
	void set_position(vec3 vec);

private:
	// Projection type (Perspective or Ortographic)
	Projection_Type projection_type = PERSPECTIVE;
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

	// Camera location in world
	vec3 position = vec3(0.f);

	// TODO camera forward vector
};
