#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <glm.hpp>

#include <Globals.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;

// This class will be changed when importing models is available
class Mesh
{
public:
	// Constructor and destructor
	Mesh(std::string path, const char* texture_path);
	~Mesh();

	GLuint get_VAO();
	unsigned int get_triangle_count();

	// Load the texture
	GLuint load_texture(const char* path);

	// Translate mesh
	void translate_mesh(vec3 tra);
	// Rotate mesh
	void rotate_mesh(vec3 rot, float angle);
	// Scale mesh
	void scale_mesh(vec3 scale);

	// Getters and setters
	mat4 get_model_matrix();
	GLfloat* get_model_matrix_pointer();
	void set_model_matrix(mat4 mat);
	mat4 get_normal_matrix();
	GLfloat* get_normal_matrix_pointer();
	void set_normal_matrix(mat4 mat);
	GLuint get_texture_id();
	void set_texture_id(GLuint _id);

private:

	GLuint VAO;
	GLuint texture_id;
	unsigned int triangle_count = 0;

	mat4 model_matrix;
	mat4 normal_matrix;

	// Loads the mesh from specified path
	GLuint load_obj_mesh(std::string path);
	// Generates the mesh buffers
	GLuint gen_obj_buffers(std::vector<vec3> &positions, std::vector<vec3> &normals, std::vector<vec2> &tex_coords);
};
