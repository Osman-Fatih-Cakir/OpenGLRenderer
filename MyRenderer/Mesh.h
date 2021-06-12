#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <vector>
#include <glm.hpp>

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

private:

	GLuint VAO;
	unsigned int triangle_count = 0;

	// Loads the mesh from specified path
	GLuint load_monkey(std::string path);
	// Generates the mesh buffers
	GLuint gen_monkey_buffers(std::vector<vec3> &positions, std::vector<vec3> &normals, std::vector<vec2> &tex_coords);
};
