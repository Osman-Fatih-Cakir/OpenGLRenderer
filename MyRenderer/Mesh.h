#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <Globals.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

// This class will be changed when importing models is available
// For now it holds a model that is chosen and hardcoded
class Mesh
{
public:
	// Constructor
	Mesh(std::string path);

	GLuint get_VAO();
	unsigned int get_triangle_count();

	// Translate mesh
	void translate_mesh(vec3 tra);
	// Rotate mesh
	void rotate_mesh(vec3 rot, float angle);
	// Scale mesh
	void scale_mesh(vec3 scale);

	// Getters and setters
	mat4 get_model_matrix();
	void set_model_matrix(mat4 mat);
	mat4 get_normal_matrix();
	void set_normal_matrix(mat4 mat);

private:

	GLuint VAO;
	unsigned int triangle_count = 0;

	mat4 model_matrix;
	mat4 normal_matrix;

	// Loads the mesh from specified path
	GLuint load_obj_mesh(std::string path);
	// Generates the mesh buffers
	GLuint gen_obj_buffers(std::vector<vec3> &positions, std::vector<vec3> &normals, std::vector<vec2> &tex_coords);
};
