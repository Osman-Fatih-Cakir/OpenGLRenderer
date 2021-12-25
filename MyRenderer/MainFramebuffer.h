#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Window.h>

class MainFramebuffer
{
public:

	MainFramebuffer();
	~MainFramebuffer();

	void create_framebuffer();

	GLuint get_FBO();
	GLuint get_color_texture();

private:
	
	GLuint FBO;
	GLuint color_texture;
	GLuint rbo_depth;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;
};
