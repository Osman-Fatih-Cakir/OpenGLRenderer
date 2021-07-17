
#include <DirectionalDepth.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <Globals.h>
#include <init_shaders.h>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

// Constructor
DirectionalDepth::DirectionalDepth()
{
	// Initialize and compile shaders
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();
}

// Destructor
DirectionalDepth::~DirectionalDepth() {}

// Starts the shader program
void DirectionalDepth::start_program()
{
	glUseProgram(program);
}

// Sets space matrix
void DirectionalDepth::set_space_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_space_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Sets model matrix
void DirectionalDepth::set_model_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Renders the scene
void DirectionalDepth::render(GLuint VAO, unsigned int vertex_count)
{
	glBindVertexArray(VAO);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
}

// Compiles shaders and generate program
void DirectionalDepth::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/depth_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/depth_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}



// Gets uniforms locations from the program
void DirectionalDepth::get_uniform_locations()
{
	loc_space_matrix = glGetUniformLocation(program, "space_matrix");
	loc_model_matrix = glGetUniformLocation(program, "model_matrix");
}