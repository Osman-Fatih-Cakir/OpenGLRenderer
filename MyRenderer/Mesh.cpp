
#include "Mesh.h"
#include <iostream>

// Constructor
Mesh::Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> _indices,
	std::vector<Texture> _textures, mat4 _transformation, bool alpha)
{
	vertices = _vertices;
	indices = _indices;
	textures = _textures;
	transformation = _transformation;
	_has_alpha = alpha;

	// Set the buffers
	setup_mesh();
}

// Destructor
Mesh::~Mesh() 
{
	// Deallocate buffers
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
bool Mesh::has_alpha()
{
	return _has_alpha;
}

// Draw the mesh
void Mesh::draw(GLuint shader_program, bool has_normal_map, bool has_ao_map, bool has_emissive_map,
	bool has_opacity_map, mat4 model_matrix)
{
	// Set the textures
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string name = textures[i].type;
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(shader_program, name.c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	// Check for these variables
	glUniform1i(glGetUniformLocation(shader_program, "has_normal_map"), has_normal_map);
	glUniform1i(glGetUniformLocation(shader_program, "has_ao_map"), has_ao_map);
	glUniform1i(glGetUniformLocation(shader_program, "has_emissive_map"), has_emissive_map);
	glUniform1i(glGetUniformLocation(shader_program, "has_opacity_map"), has_opacity_map);
	mat4 mt = model_matrix * transformation;
	mat4 nmt = glm::transpose(glm::inverse(mt));
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "normal_matrix"), 1, GL_FALSE,
		&nmt[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "model_matrix"), 1, GL_FALSE, 
		&mt[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader_program, "prev_model_matrix"), 1, GL_FALSE,
		&prev_total_transformation[0][0]);

	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Store previous transformation matrix
	prev_total_transformation = mt;
}

// Generate buffers for mesh data
void Mesh::setup_mesh()
{
	// Create buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// Load data into buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);
}