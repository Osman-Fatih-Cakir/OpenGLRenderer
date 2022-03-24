#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Model.h>
#include <gBuffer.h>
#include <MainFramebuffer.h>
#include <Scene.h>

typedef glm::mat4 mat4;
typedef glm::vec3 vec3;

class ForwardRender
{
public:

	ForwardRender();
	~ForwardRender();

	void start_program(MainFramebuffer* fb);
	void change_viewport_resolution(unsigned int w, unsigned int h);
	GLuint get_shader_program();
	GLuint get_depth_fbo();
	GLuint get_depth_texture();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);
	void set_color(vec3 vec);
	void render(Scene* scene, MainFramebuffer* main_fb);

private:

	GLuint program;
	GLuint width = WINDOW_WIDTH;
	GLuint height = WINDOW_HEIGHT;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;
	GLuint loc_color;
	GLuint depth_fbo;
	GLuint depth_texture;

	void init_shaders();
	void get_uniform_locations();
	void init_depth_fbo();
	void copy_depth_buffer(MainFramebuffer* main_fb);
	void render_model(Camera* camera, Model* model);
};
