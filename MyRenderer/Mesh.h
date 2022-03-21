#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <glm.hpp>
#include <string>

typedef glm::mat4 mat4;
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
	Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices,
		std::vector<Texture> _textures, mat4 _transformation, bool alpha);
	~Mesh();
	bool has_alpha();

	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	mat4 transformation;
	mat4 prev_total_transformation;
	GLuint VAO;

	void draw(GLuint shader_program, bool has_normal_map, bool has_ao_map, bool has_emissive_map,
		bool has_opacity_map, mat4 model_matrix);

private:
	Mesh();

	GLuint VBO, EBO;
	bool _has_alpha = false;

	void setup_mesh();
};
