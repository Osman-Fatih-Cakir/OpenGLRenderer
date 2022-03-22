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
	GLuint get_depth_texture();

private:
	
	GLuint FBO;
	GLuint color_texture;
	GLuint depth_texture;

	unsigned int width = WINDOW_WIDTH;
	unsigned int height = WINDOW_HEIGHT;
};
