#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class gBuffer
{
public:

	gBuffer();
	~gBuffer();

	void attach_depthbuffer_to_framebuffer(GLuint framebuffer);
	void set_gBuffer_resolution(int w, int h);

	GLuint get_gPosition();
	GLuint get_gNormal();
	GLuint get_gAlbedoSpec();
	GLuint get_fbo();

	void start_program();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_model_matrix(mat4 mat);
	void set_normal_matrix(mat4 mat);
	void set_diffuse_texture(GLuint id);
	unsigned int get_width();
	unsigned int get_height();
	void render(GLuint VAO, unsigned int vertex_count);

private:

	int gBuffer_width = 1024;
	int gBuffer_height = 1024;
	GLuint program;
	GLuint gBuffer_fbo;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_normal_matrix;
	GLuint loc_diffuse;
	GLuint gPosition;
	GLuint gNormal;
	GLuint gAlbedoSpec;

	void init_shaders();
	void create_framebuffer();
	void get_uniform_locations();
};