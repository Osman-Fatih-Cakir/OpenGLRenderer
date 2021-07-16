#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::vec2 vec2;

class DirectionalDepth
{
public:

	DirectionalDepth();
	~DirectionalDepth();

	void start_program();
	void change_depth_map_resolution(int w, int h);
	void set_space_matrix(mat4 mat);
	void set_model_matrix(mat4 mat);
	GLuint get_depth_map();
	void render(GLuint VAO, unsigned int vertex_count);

private:

	int depth_map_width = 1024;
	int depth_map_height = 1024;
	GLuint program;
	GLuint depth_map_fbo;
	GLuint depth_map;
	GLuint loc_space_matrix;
	GLuint loc_model_matrix;

	void init_shaders();
	void create_framebuffer();
	void get_uniform_locations();

};
