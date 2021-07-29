#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <glm.hpp>
#include <string>

typedef glm::vec3 vec3;
typedef glm::vec2 vec2;

// Vertex struct
struct Vertex
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	vec3 tangent;
	vec3 bitangent;
};

// Texture struct
struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};

// This class represents a mesh in a model
class Mesh
{
public:
	// Constructor and destructor
	Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices, std::vector<Texture> _textures);
	~Mesh();

	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	GLuint VAO;

	void draw(GLuint shader_program, bool has_normal_map);

private:
	GLuint VBO, EBO;

	void setup_mesh();
};
