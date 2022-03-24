#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Window.h>
#include <MainFramebuffer.h>
#include <gBuffer.h>
#include <ForwardRender.h>

class TAA
{
public:

	TAA();
	~TAA();

	void render(MainFramebuffer* main_fb, MainFramebuffer* prev_fb, gBuffer* GBuffer,
		ForwardRender* forwardRender);
	
private:

	void set_resolution();
	void set_cur_depth_map(GLuint map);
	void set_prev_depth_map(GLuint map);
	void set_cur_color_map(GLuint map);
	void set_prev_color_map(GLuint map);
	void set_velocity_map(GLuint map);

	void init_shaders();
	void init_quad();
	void draw_quad();
	void get_uniform_locations();
	void set_uniforms(MainFramebuffer* main_fb, MainFramebuffer* prev_fb, gBuffer* GBuffer,
		ForwardRender* forwardRender);

	GLuint taa_program;
	GLuint loc_resolution;
	GLuint loc_prev_depth_map;
	GLuint loc_cur_depth_map;
	GLuint loc_cur_color_map;
	GLuint loc_prev_color_map;
	GLuint loc_velocity_map;

	GLuint quad_VAO;
	GLuint quadVBO;
	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;
};
