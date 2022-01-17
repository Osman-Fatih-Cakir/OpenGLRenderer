#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <Window.h>
#include <gBuffer.h>
#include <MainFramebuffer.h>
#include <Scene.h>

class ForwardLitRender
{
public:
	ForwardLitRender();
	~ForwardLitRender();

	void render(gBuffer* _GBuffer, MainFramebuffer* main_fb, Scene* scene);

	void change_viewport_resolution(unsigned int x, unsigned int y);
	GLuint get_shader_program();
	void set_projection_matrix(mat4 mat);
	void set_view_matrix(mat4 mat);

private:

	void init_program();
	void init_uniforms();
	void blit_depth_buffer(gBuffer* GBuffer, MainFramebuffer* fb);

	GLuint program;
	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_model_matrix;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;
};